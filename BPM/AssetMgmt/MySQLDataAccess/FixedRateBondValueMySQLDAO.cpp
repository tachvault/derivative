/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include <memory>
#include "Name.hpp"
#include "FixedRateBondValueMySQLDAO.hpp"
#include "GroupRegister.hpp"
#include "MySqlConnection.hpp"
#include "EntityManager.hpp"
#include "EntityMgrUtil.hpp"
#include "IDataSource.hpp"
#include "CountryHolder.hpp"
#include "IFixedRateBond.hpp"

namespace derivative
{
	GROUP_REGISTER(FixedRateBondValueMySQLDAO);
	DAO_REGISTER(IFixedRateBondValue, MYSQL, FixedRateBondValueMySQLDAO);

	const int FixedRateBondValueMySQLDAO::MaxCount = 100;
	std::shared_ptr<IMake> FixedRateBondValueMySQLDAO::Make(const Name &nm)
	{
		/// Construct FixedRateBondValueMySQLDAO from given name and register with EntityManager
		std::shared_ptr<FixedRateBondValueMySQLDAO> dao = make_shared<FixedRateBondValueMySQLDAO>(nm);
		dao = dynamic_pointer_cast<FixedRateBondValueMySQLDAO>(EntityMgrUtil::registerObject(nm, dao));
		LOG(INFO) << " FixedRateBondValueMySQLDAO  " << nm << " is constructed and registered with EntityManager" << endl;

		/// return constructed object if no exception is thrown
		return dao;
	}

	std::shared_ptr<IFixedRateBondValue> FixedRateBondValueMySQLDAO::constructEntity(const Name& nm)
	{
		/// First get the FixedRateBondValue exemplar object from the registry
		/// Get the EntityManager instance
		EntityManager& entMgr= EntityManager::getInstance();	

		/// get the concrete types for the given alias name
		std::vector<Name> names;
		entMgr.findAlias(nm, names);
		if (names.empty())
		{
			LOG(ERROR) << " No concrete name found for " << nm << endl;
			throw DataSourceException("FixedRateBondValue not bound with correct alias");
		}

		Name entityName((*names.begin()).GetGrpId(), nm.GetObjId());

		/// copy the keys from Alias Name to concrete name
		entityName.SetKeys(nm.GetKeyMap());

		/// get the examplar object for the FixedRateBondValue
		/// Exemplar objects should be initialized
		/// during global initialization time.
		std::shared_ptr<IObject> exemplar = entMgr.findObject(Name(entityName.GetGrpId()));

		/// Now we have the exampler object. 
		/// Make the exemplar FixedRateBondValue to construct FixedRateBondValue for the given name
		std::shared_ptr<IFixedRateBondValue> value;
		try
		{
			value = dynamic_pointer_cast<IFixedRateBondValue>(dynamic_pointer_cast<IMake>(exemplar)->Make(entityName));
		}
		catch(RegistryException& e)
		{
			LOG(WARNING) << " Object already found in registry " << e.what() << endl;
			return value;
		}
		/// we don't want to catch other exceptions here.. Let the client code handle that.

		/// if the bond is not in the registry and constructed sucessfully
		/// then we need to find and associate FixedRateBond with FixedRateBondValue
		std::string symbol;
		dd::date tradeDate;
		IFixedRateBondValue::GetKeys(nm, symbol, tradeDate);
		Name FixedRateBondName = IFixedRateBond::ConstructName(symbol);
		try
		{
			/// get the FixedRateBond from registry or from data source
			/// set the FixedRateBond object with FixedRateBondValue
			value->SetFixedRateBond(dynamic_pointer_cast<IFixedRateBond>(EntityMgrUtil::findObject(FixedRateBondName)));
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
		return value;
	}

	const std::shared_ptr<IObject> FixedRateBondValueMySQLDAO::find(const Name& nm)
	{
		LOG(INFO) << "FixedRateBondValueMySQLDAO::find(..) is called for " << nm << endl;
		/// If we are here means, FixedRateBondValue object with the name nm is
		/// not in the registry. That's we should fetch the 
		/// object from the data data source.

		/// constructEntity could throw exception.
		/// let the caller of this function handle
		std::shared_ptr<IFixedRateBondValue> value = constructEntity(nm);

		/// Once we have the m_stock skeleton, it is time to populate the
		/// fields from the stock fetched from the database

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

		/// Populate the stock specific attributes
		return findBond(value);
	}

	void FixedRateBondValueMySQLDAO::find(const Name& nm, std::vector<std::shared_ptr<IObject> > & entities)
	{
		LOG(INFO) << "FixedRateBondValueMySQLDAO::find(..) is called for " << nm << endl;

		/// If we are here means, FixedRateBondValue object with trade date in nm are
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
		findFixedRateBondValue(nm, entities);
	}

	std::shared_ptr<IObject> FixedRateBondValueMySQLDAO::findBond(std::shared_ptr<IFixedRateBondValue>& bond)
	{
		try
		{
			std::unique_ptr<sql::PreparedStatement> pstmt;
			std::unique_ptr<sql::ResultSet> res;

			/// get the processed date.
			std::string symbol;
			dd::date tradeDate;
			IFixedRateBondValue::GetKeys(bond->GetName(), symbol, tradeDate);

			/// check if we need to proceed to get data
			std::unique_ptr<sql::Statement> stmt;
			std::unique_ptr<sql::ResultSet> countRes;

			std::string date = dd::to_iso_extended_string(tradeDate);
			sql::SQLString tradedate(date.c_str());

			pstmt.reset(m_con->prepareStatement ("CALL get_FixedRateBondValue(?, ?, @out_tradedate,\
												 @out_tradedPrice, @out_quotedPrice, @out_maturitydate, @out_yield)"));

			pstmt->setString(1, symbol.c_str());
			pstmt->setDateTime(2, tradedate);
			pstmt->execute();

			pstmt.reset(m_con->prepareStatement("SELECT @out_tradedate AS tdate, @out_tradedPrice AS tPrice, \
												@out_quotedPrice AS qPrice, @out_maturitydate AS mdate, @out_yield AS yield"));			
			res.reset(pstmt->executeQuery());
			while (res->next())
			{
				/// trade date
				auto stdate = res->getString("tdate").asStdString();
				if (stdate.empty())
				{
					/// unbind Libor rate value object and set it null.
					EntityManager& entMgr= EntityManager::getInstance();
					entMgr.unbind(dynamic_pointer_cast<IObject>(bond));
				//	bond = nullptr;					
					throw std::domain_error("No liborratevalue data found");
				}
				auto tDate = boost::any_cast<dd::date>(dd::from_simple_string(stdate));

				/// trade price
				auto tPrice = res->getDouble("tPrice");

				/// quoted price
				auto qPrice = res->getDouble("qPrice");

				/// maturity date
				auto smdate = res->getString("mdate");
				auto mDate = boost::any_cast<dd::date>(dd::from_simple_string(smdate));

				/// make sure the trade date is less than the maturity date
				if (tDate > mDate) throw std::logic_error("Invalid trade and or maturity dates"); 

				// yield
				auto yield = res->getDouble("yield");

				bond->SetMaturityDate(mDate);
				bond->SetTradeDate(tDate);
				bond->SetYield(yield);
				bond->SetTradePrice(tPrice);
				bond->SetQuotedPrice(qPrice);
				bond = dynamic_pointer_cast<IFixedRateBondValue>(EntityMgrUtil::registerObject(bond->GetName(), bond));
				LOG(INFO) << " FixedRateBondValue " << symbol << " with trade date " << tDate << " constructed with " \
					<< "maturity date" << mDate \
					<< "trade date" << tDate \
					<< "trade price date" << tPrice \
					<< "quoted price date" << qPrice \
					<< "yield " << yield <<	endl;

				return bond;
			}
		}
		catch (sql::SQLException &e) 
		{
			LOG(ERROR) << " No fixed rate bond data found in source for " << m_name << endl;
			LOG(ERROR) << "# ERR: " << e.what();
			throw e;
		}
	}

	void FixedRateBondValueMySQLDAO::findFixedRateBondValue(const Name& nm, \
		std::vector<std::shared_ptr<IObject> > & entities)
	{
		dd::date tdate;
		std::string cntryCode;
		IFixedRateBondValue::GetKeys(nm, cntryCode, tdate);
		try
		{
			EntityManager& entMgr= EntityManager::getInstance();

			std::unique_ptr<sql::Statement> stmt;
			std::unique_ptr<sql::ResultSet> res;

			stmt.reset(m_con->createStatement());
			std::string query = std::string("select symbol from fixedratebond where countrycode like \"") + cntryCode + "%\"";
			res.reset(stmt->executeQuery(query));
			while(res->next())
			{
				auto symbol = res->getString(1);
				Name bondName = IFixedRateBondValue::ConstructName(symbol, tdate);
				try
				{
					/// try getting if from EntityManager. If not in entitity manager
					/// then fetch from database
					std::shared_ptr<IObject> obj = entMgr.findObject(bondName);	
					entities.push_back(obj);

					/// if found then skip the steps of getting from the database.
					continue;
				}
				catch(RegistryException& e)
				{
					LOG(INFO) << bondName << " not found in registry. fetch from data store " << endl;
				}

				/// no object with same type or child type found
				/// in registry. We have to fetch from database		
				try
				{						
					LOG(INFO) << " Attempt to fetch from data store " << endl;
					std::shared_ptr<IObject>obj = find(bondName);
					entities.push_back(obj);
				}
				catch (sql::SQLException &e) 
				{
					LOG(ERROR) << " No data found in source for " << bondName << endl;
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