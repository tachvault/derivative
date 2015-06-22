/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include <memory>
#include "Name.hpp"
#include "IRMySQLDAO.hpp"
#include "GroupRegister.hpp"
#include "MySqlConnection.hpp"
#include "EntityManager.hpp"
#include "EntityMgrUtil.hpp"
#include "IDataSource.hpp"
#include "CountryHolder.hpp"
#include "PrimaryAssetUtil.hpp"

namespace derivative
{
	GROUP_REGISTER(IRMySQLDAO);
	DAO_REGISTER(IIR, MYSQL, IRMySQLDAO);

	const int IRMySQLDAO::MaxCount = 100;
	std::shared_ptr<IMake> IRMySQLDAO::Make(const Name &nm)
	{
		/// Construct IRMySQLDAO from given name and register with EntityManager
		std::shared_ptr<IRMySQLDAO> dao = make_shared<IRMySQLDAO>(nm);
		dao = dynamic_pointer_cast<IRMySQLDAO>(EntityMgrUtil::registerObject(nm, dao));
		LOG(INFO) << " IRMySQLDAO  " << nm << " is constructed and registered with EntityManager" << endl;

		/// return constructed object if no exception is thrown
		return dao;
	}

	const std::shared_ptr<IObject> IRMySQLDAO::find(const Name& nm)
	{
		LOG(INFO) << "IRMySQLDAO::find(..) is called for " << nm << endl;
		/// If we are here means, IRValue object with the name nm is
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
		findIR(nm);

		if (!m_rate)
		{
			throw MySQLSrcException("Rate not found");
		}
		/// now return m_rate
		return m_rate;
	}

	void IRMySQLDAO::findIR(const Name& name)
	{
		try
		{
			std::string cntryCode;
			Maturity::MaturityType maturity;
			dd::date tdate;
			IIR::GetKeys(name, cntryCode, maturity);

			std::unique_ptr<sql::Statement> stmt;
			std::unique_ptr<sql::ResultSet> res;

			stmt.reset(m_con->createStatement());
			std::string query = std::string("CALL get_InterestRate('") + cntryCode + std::string("')");
			res.reset(stmt->executeQuery(query));
			while (res->next())
			{

				size_t id = res->getInt("rateId");
				std::string code = res->getString("countryCode").asStdString();
				std::string rateType = res->getString("type").asStdString();
				int maturity = res->getInt("maturityType");

				/// get country for the given code
				CountryHolder& cntryHolder = CountryHolder::getInstance();
				const Country& cntry = cntryHolder.GetCountry(code);

				Name nm(IIR::TYPEID, id);
				std::shared_ptr<IIR> rate = PrimaryUtil::ConstructEntity<IIR>(nm);
				rate->SetCountry(cntry);
				rate->SetRateType(rateType);
				rate->SetMaturityType(static_cast<Maturity::MaturityType>(maturity));
				rate = std::dynamic_pointer_cast<IIR>(EntityMgrUtil::registerObject(rate->GetName(), rate));

				LOG(INFO) << " New InterestRate bject created with Country(" << code \
					<< ", Rate Type " << rateType \
					<< ", Maturity " << static_cast<Maturity::MaturityType>(maturity) << endl;

				if (id == name.GetObjId())
				{
					m_rate = rate;
				}
			}
			res->close();
		}
		catch (sql::SQLException &e)
		{
			LOG(ERROR) << " Error loading country data " << endl;
			LOG(ERROR) << "# ERR: " << e.what();
			throw e;
		};
		m_con = nullptr;
	};

	void IRMySQLDAO::find(const Name& nm, std::vector<std::shared_ptr<IObject> > & entities)
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
			std::string cntryCode;
			Maturity::MaturityType maturity;
			dd::date tdate;
			IIR::GetKeys(nm, cntryCode, maturity);

			std::unique_ptr<sql::Statement> stmt;
			std::unique_ptr<sql::ResultSet> res;

			stmt.reset(m_con->createStatement());
			std::string query = std::string("CALL get_InterestRate('") + cntryCode + std::string("')");
			res.reset(stmt->executeQuery(query));
			while (res->next())
			{
				size_t id = res->getInt("rateId");
				std::string code = res->getString("countryCode").asStdString();
				std::string rateType = res->getString("type").asStdString();
				int maturity = res->getInt("maturityType");

				/// get country for the given code
				CountryHolder& cntryHolder = CountryHolder::getInstance();
				const Country& cntry = cntryHolder.GetCountry(code);

				Name nm(IIR::TYPEID, id);
				std::shared_ptr<IIR> rate = PrimaryUtil::ConstructEntity<IIR>(nm);
				rate->SetCountry(cntry);
				rate->SetRateType(rateType);
				rate->SetMaturityType(static_cast<Maturity::MaturityType>(maturity));
				rate = dynamic_pointer_cast<IIR>(EntityMgrUtil::registerObject(rate->GetName(), rate));

				LOG(INFO) << " New InterestRate bject created with Country(" << code \
					<< ", Rate Type " << rateType \
					<< ", Maturity " << static_cast<Maturity::MaturityType>(maturity) << endl;

				entities.push_back(rate);
			}
			res->close();
		}
		catch (sql::SQLException &e)
		{
			LOG(ERROR) << " Error loading country data " << endl;
			LOG(ERROR) << "# ERR: " << e.what();
			throw e;
		};
		m_con = nullptr;
	}

} /* namespace derivative */