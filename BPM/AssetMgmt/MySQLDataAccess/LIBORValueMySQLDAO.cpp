/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include <memory>
#include "Name.hpp"
#include "LIBORValueMySQLDAO.hpp"
#include "GroupRegister.hpp"
#include "MySqlConnection.hpp"
#include "EntityManager.hpp"
#include "EntityMgrUtil.hpp"
#include "IDataSource.hpp"
#include "CountryHolder.hpp"
#include "IIBOR.hpp"
#include "PrimaryAssetUtil.hpp"

namespace derivative
{
	GROUP_REGISTER(LIBORValueMySQLDAO);
	DAO_REGISTER(IIBORValue, MYSQL, LIBORValueMySQLDAO);

	const int LIBORValueMySQLDAO::MaxCount = 100;
	std::shared_ptr<IMake> LIBORValueMySQLDAO::Make(const Name &nm)
	{
		/// Construct LIBORValueMySQLDAO from given name and register with EntityManager
		std::shared_ptr<LIBORValueMySQLDAO> dao = make_shared<LIBORValueMySQLDAO>(nm);
		dao = dynamic_pointer_cast<LIBORValueMySQLDAO>(EntityMgrUtil::registerObject(nm, dao));
		LOG(INFO) << " LIBORValueMySQLDAO  " << nm << " is constructed and registered with EntityManager" << endl;

		/// return constructed object if no exception is thrown
		return dao;
	}

	const std::shared_ptr<IObject> LIBORValueMySQLDAO::find(const Name& nm)
	{
		LOG(INFO) << "LIBORValueMySQLDAO::find(..) is called for " << nm << endl;
		/// If we are here means, LIBORValue object with the name nm is
		/// not in the registry. That's we should fetch the 
		/// object from the data data source.

		/// constructEntity could throw exception.
		/// let the caller of this function handle
		m_value = PrimaryUtil::ConstructEntity<IIBORValue>(nm);

		/// Now find and associate LIBOR with LIBORValue
		std::string curr;
		Maturity::MaturityType maturity;
		dd::date date;
		IIBORValue::GetKeys(nm, curr, maturity, date);
		Name LIBORName = IIBOR::ConstructName(curr, maturity);		
		try
		{
			/// get the LIBOR from registry or from data source
			/// set the LIBOR object with LIBORValue
			m_value->SetRate(dynamic_pointer_cast<IIBOR>(EntityMgrUtil::findObject(LIBORName)));
		}
		catch(RegistryException& e)
		{
			LOG(WARNING) << " RegistryException thrown " << e.what() << endl;
			throw e;
		}
		catch(...)
		{
			LOG(WARNING) << " Unknown Exception thrown " << endl;
			throw;
		}

		/// Once we have the m_interestRate skeleton, it is time to populate the
		/// fields from the interestRate fetched from the database

		/// Make a connection to the MySQL if not connected
		/// log the SQLException error if thrown
		/// and retrow the exception so that the client
		/// can catch and take required action
		try
		{
			if (!m_con)
			{
				MySQLConnection::InitConnection(m_driver, m_con);
			}
		}
		catch(sql::SQLException &e) 
		{
			LOG(ERROR) << " MySQL throws exception while connecting to the database " << nm << endl;
			LOG(ERROR) << "# ERR: " << e.what();
			throw e;
		}	

		/// Populate the interestRate specific attributes
		findLIBORValue(date);

		/// now return m_interestRate
		return m_value;
	}

	void LIBORValueMySQLDAO::find(const Name& nm, std::vector<std::shared_ptr<IObject> > & entities)
	{
		LOG(INFO) << "LIBORValueMySQLDAO::find(..) is called for " << nm << endl;

		/// If we are here means, LIBORValue object with trade date in nm are
		/// not in the registry. That's we should fetch the 
		/// object from the data data source.
		try
		{
			if (!m_con)
			{
				MySQLConnection::InitConnection(m_driver, m_con);
			}
		}
		catch(sql::SQLException &e) 
		{
			LOG(ERROR) << " MySQL throws exception while connecting to the database " << nm << endl;
			LOG(ERROR) << "# ERR: " << e.what();
			throw e;
		}	

		/// Populate the bond value specific attributes
		findLIBORValue(nm, entities);
	}

	void LIBORValueMySQLDAO::findLIBORValue(const dd::date& issuedDate)
	{
		std::string date;
		try
		{
			std::unique_ptr<sql::PreparedStatement> pstmt;
			std::unique_ptr<sql::ResultSet> res;

			date = dd::to_iso_extended_string(issuedDate);
			sql::SQLString issuedate(date.c_str());

			pstmt.reset(m_con->prepareStatement ("CALL get_liborRateValue(?, ?, @out_rate, @out_date)"));
			auto liborId = m_value->GetRate()->GetName().GetObjId();
			pstmt->setUInt64(1, liborId);
			pstmt->setDateTime(2, issuedate);
			pstmt->execute();

			pstmt.reset(m_con->prepareStatement("SELECT @out_rate AS _rate, @out_date AS _issueDate"));			
			res.reset(pstmt->executeQuery());
			while (res->next())
			{
				auto lastDate = res->getString("_issueDate").asStdString();
				if (lastDate.empty())
				{
					/// unbind Libor rate value object and set it null.
					EntityManager& entMgr= EntityManager::getInstance();
					entMgr.unbind(dynamic_pointer_cast<IObject>(m_value));
					m_value = nullptr;					
					throw std::domain_error("No liborratevalue data found");
				}

				/// else construct the object and return.
				auto rate = res->getDouble("_rate");
				m_value->SetLastRate(rate);
				m_value->SetReportedDate(boost::any_cast<dd::date>(dd::from_simple_string(lastDate)));
				m_value = dynamic_pointer_cast<IIBORValue>(EntityMgrUtil::registerObject(m_value->GetName(), m_value));
				LOG(INFO) << " Interest rate value " << m_value->GetName() \
					<< " constructed with " << rate << endl;
			}
		}
		catch (sql::SQLException &e) 
		{
			LOG(ERROR) << " No interest data found in source for " << m_value->GetRate()->GetName().GetObjId() 
				<< " and "  << date << endl;
			LOG(ERROR) << "# ERR: " << e.what();
			throw e;
		}		
	};

	void LIBORValueMySQLDAO::findLIBORValue(const Name& nm, \
		std::vector<std::shared_ptr<IObject> > & entities)
	{
		std::string currCode;
		Maturity::MaturityType maturity;
		dd::date tdate;
		IIBORValue::GetKeys(nm, currCode, maturity, tdate);
		try
		{
			EntityManager& entMgr= EntityManager::getInstance();

			std::unique_ptr<sql::Statement> stmt;
			std::unique_ptr<sql::ResultSet> res;

			stmt.reset(m_con->createStatement());
			std::string query = std::string("select maturityType from liborrate where currencyCode = \"") + currCode + "\"";
			res.reset(stmt->executeQuery(query.c_str()));
			while(res->next())
			{
				auto maturity = static_cast<Maturity::MaturityType>(res->getInt(1));
				Name rateName = IIBORValue::ConstructName(currCode, maturity, tdate);

				/// try to get from the registry
				try
				{
					/// try getting if from EntityManager. If not in entitity manager
					/// then fetch from database
					std::shared_ptr<IObject> obj = entMgr.findObject(rateName);	
					entities.push_back(obj);

					/// if found then skip the steps of getting from database.
					continue;
				}
				catch(RegistryException& e)
				{
					LOG(INFO) << rateName << " not in registry " << endl;
				}

				/// if not found then get from the database
				try
				{
					LOG(INFO) << " Attempt to fetch from data store " << endl;
					std::shared_ptr<IObject> obj = find(rateName);
					entities.push_back(obj);
				}
				catch (sql::SQLException &e) 
				{
					LOG(ERROR) << " No data found in source for " << rateName << endl;
					LOG(ERROR) << "# ERR: " << e.what();
					throw e;
				}	
				catch(std::domain_error& e)
				{
				}			
			}
		}
		catch (sql::SQLException &e) 
		{
			LOG(ERROR) << " No data found in source for " << nm << endl;
			LOG(ERROR) << "# ERR: " << e.what();
			throw e;
		}		
	};
} /* namespace derivative */