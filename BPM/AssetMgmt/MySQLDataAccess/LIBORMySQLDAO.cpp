/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include <memory>
#include "Name.hpp"
#include "LIBORMySQLDAO.hpp"
#include "GroupRegister.hpp"
#include "MySqlConnection.hpp"
#include "EntityManager.hpp"
#include "EntityMgrUtil.hpp"
#include "IDataSource.hpp"
#include "CurrencyHolder.hpp"
#include "PrimaryAssetUtil.hpp"

namespace derivative
{
	GROUP_REGISTER(LIBORMySQLDAO);
	DAO_REGISTER(IIBOR, MYSQL, LIBORMySQLDAO);

	const int LIBORMySQLDAO::MaxCount = 100;
	std::shared_ptr<IMake> LIBORMySQLDAO::Make(const Name &nm)
	{
		/// Construct LIBORMySQLDAO from given name and register with EntityManager
		std::shared_ptr<LIBORMySQLDAO> dao = make_shared<LIBORMySQLDAO>(nm);
		dao = dynamic_pointer_cast<LIBORMySQLDAO>(EntityMgrUtil::registerObject(nm, dao));
		LOG(INFO) << " LIBORMySQLDAO  " << nm << " is constructed and registered with EntityManager" << endl;

		/// return constructed object if no exception is thrown
		return dao;
	}

	const std::shared_ptr<IObject> LIBORMySQLDAO::find(const Name& nm)
	{
		LOG(INFO) << "LIBORMySQLDAO::find(..) is called for " << nm << endl;
		/// If we are here means, LIBORValue object with the name nm is
		/// not in the registry. That's we should fetch the 
		/// object from the data data source.

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
		catch (sql::SQLException &e)
		{
			LOG(ERROR) << " MySQL throws exception while connecting to the database " << nm << endl;
			LOG(ERROR) << "# ERR: " << e.what();
			throw e;
		}

		/// Populate the interestRate specific attributes
		findLIBOR(nm);

		if (!m_rate)
		{
			throw MySQLSrcException("Rate not found");
		}
		/// now return m_rate
		return m_rate;
	}

	void LIBORMySQLDAO::findLIBOR(const Name& name)
	{
		try
		{
			std::string currCode;
			Maturity::MaturityType maturity;
			dd::date tdate;
			IIBOR::GetKeys(name, currCode, maturity);

			std::unique_ptr<sql::Statement> stmt;
			std::unique_ptr<sql::ResultSet> res;

			stmt.reset(m_con->createStatement());
			std::string query = std::string("CALL get_LiborRate('") + currCode + std::string("')");
			res.reset(stmt->executeQuery(query));
			while (res->next())
			{

				size_t id = res->getInt("liborId");
				std::string code = res->getString("currencyCode").asStdString();
				std::string rateType = res->getString("type").asStdString();
				int maturity = res->getInt("maturityType");

				/// get country for the given code
				CurrencyHolder& currHolder = CurrencyHolder::getInstance();
				const Currency& curr = currHolder.GetCurrency(code);

				Name nm(IIBOR::TYPEID, id);
				std::shared_ptr<IIBOR> rate = PrimaryUtil::ConstructEntity<IIBOR>(nm);
				rate->SetCurrency(curr);
				rate->SetRateType(rateType);
				rate->SetMaturityType(static_cast<Maturity::MaturityType>(maturity));
				rate = dynamic_pointer_cast<IIBOR>(EntityMgrUtil::registerObject(rate->GetName(), rate));

				LOG(INFO) << " New InterestRate bject created with Country(" << code \
					<< ", Maturity " << static_cast<Maturity::MaturityType>(maturity) << endl;

				if (id == name.GetObjId())
				{
					m_rate = rate;
				}
			}
		}
		catch (sql::SQLException &e)
		{
			LOG(ERROR) << " Error loading country data " << endl;
			LOG(ERROR) << "# ERR: " << e.what();
			throw e;
		};
	};

	void LIBORMySQLDAO::find(const Name& nm, std::vector<std::shared_ptr<IObject> > & entities)
	{
		try
		{
			if (!m_con)
			{
				MySQLConnection::InitConnection(m_driver, m_con);
			}
		}
		catch (sql::SQLException &e)
		{
			LOG(ERROR) << " MySQL throws exception while connecting to the database " << nm << endl;
			LOG(ERROR) << "# ERR: " << e.what();
			throw e;
		}

		try
		{
			std::string currCode;
			Maturity::MaturityType maturity;
			dd::date tdate;
			IIBOR::GetKeys(nm, currCode, maturity);

			std::unique_ptr<sql::Statement> stmt;
			std::unique_ptr<sql::ResultSet> res;

			stmt.reset(m_con->createStatement());
			std::string query = std::string("CALL get_LiborRate('") + currCode + std::string("')");
			res.reset(stmt->executeQuery(query));
			while (res->next())
			{
				size_t id = res->getInt("liborId");
				std::string code = res->getString("currencyCode").asStdString();
				std::string rateType = res->getString("type").asStdString();
				int maturity = res->getInt("maturityType");

				/// get currency for the given code
				CurrencyHolder& currHolder = CurrencyHolder::getInstance();
				const Currency& curr = currHolder.GetCurrency(code);

				Name nm(IIBOR::TYPEID, id);
				std::shared_ptr<IIBOR> rate = PrimaryUtil::ConstructEntity<IIBOR>(nm);
				rate->SetCurrency(curr);
				rate->SetRateType(rateType);
				rate->SetMaturityType(static_cast<Maturity::MaturityType>(maturity));
				rate = dynamic_pointer_cast<IIBOR>(EntityMgrUtil::registerObject(rate->GetName(), rate));

				LOG(INFO) << " New LIBORRate bject created with Currency(" << code \
					<< ", Rate Type " << rateType \
					<< ", Maturity " << static_cast<Maturity::MaturityType>(maturity) << endl;

				entities.push_back(rate);
			}
		}
		catch (sql::SQLException &e)
		{
			LOG(ERROR) << " Error loading country data " << endl;
			LOG(ERROR) << "# ERR: " << e.what();
			throw e;
		};
	}
} /* namespace derivative */