/*
Copyright (c) 2013 - 2014 Nathan Muruganantha. All rights reserved.
*/

#include <memory>
#include "Name.hpp"
#include "ExchangeRateMySQLDAO.hpp"
#include "GroupRegister.hpp"
#include "MySqlConnection.hpp"
#include "EntityManager.hpp"
#include "EntityMgrUtil.hpp"
#include "IDataSource.hpp"
#include "Currency.hpp"
#include "CurrencyHolder.hpp"
#include "PrimaryAssetUtil.hpp"

namespace derivative
{
	GROUP_REGISTER(ExchangeRateMySQLDAO);
	DAO_REGISTER(IExchangeRate, MYSQL, ExchangeRateMySQLDAO);
	
	std::shared_ptr<IMake> ExchangeRateMySQLDAO::Make(const Name &nm)
	{
		/// Construct ExchangeRateMySQLDAO from given name and register with EntityManager
		std::shared_ptr<ExchangeRateMySQLDAO> dao = make_shared<ExchangeRateMySQLDAO>(nm);
		EntityMgrUtil::registerObject(nm, dao);
		LOG(INFO) << " ExchangeRateMySQLDAO  " << nm << " is constructed and registered with EntityManager" << endl;

		/// return constructed object if no exception is thrown
		return dao;
	}

	const std::shared_ptr<IObject> ExchangeRateMySQLDAO::find(const Name& nm)
	{
		LOG(INFO) << "ExchangeRateMySQLDAO::find(..) is called for " << nm << endl;
		/// If we are here means, ExchangeRate object with the name nm is
		/// not in the registry. That's we should fetch the 
		/// object from the data data source.

		/// constructEntity could throw exception.
		/// let the caller of this function handle
		m_exchangeRate = PrimaryUtil::ConstructEntity<IExchangeRate>(nm);

		/// Once we have the m_exchangeRate skeleton, it is time to populate the
		/// fields from the exchangeRate fetched from the database

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
		
		/// Populate the exchangeRate specific attributes
		findExchangeRate();
					
		/// now return m_exchangeRate
		return m_exchangeRate;
	}

	void ExchangeRateMySQLDAO::findExchangeRate()
	{
		try
		{
			std::unique_ptr<sql::PreparedStatement> pstmt;
			std::unique_ptr<sql::ResultSet> res;

			pstmt.reset(m_con->prepareStatement ("CALL get_forexById(?, @domestic, @foreign)"));
			pstmt->setUInt64(1, m_name.GetObjId());
			pstmt->execute();

			pstmt.reset(m_con->prepareStatement("SELECT @domestic AS _domestic, @foreign AS _foreign"));			
			res.reset(pstmt->executeQuery());
			while (res->next())
			{				
				/// Get the CurrencyHolder instance
                CurrencyHolder& currHolder = CurrencyHolder::getInstance();

				/// get the domestic currency
				std::string domestic = res->getString("_domestic").asStdString();
				const Currency& domCurr = currHolder.GetCurrency(domestic);
				m_exchangeRate->SetDomesticCurrency(domCurr);

				/// set foreign currency
				std::string foreign = res->getString("_foreign").asStdString();
				const Currency& foreignCurr = currHolder.GetCurrency(foreign);
				m_exchangeRate->SetForeignCurrency(foreignCurr);

				LOG(INFO) << " ExchangeRate object  " << m_exchangeRate->GetName() \
					      << " constructed with " << domestic \
					      << " " << foreign  << endl;
			}
		}
		catch (sql::SQLException &e) 
		{
			LOG(ERROR) << " No exchangeRate data found in source for " << m_name << endl;
			LOG(ERROR) << "# ERR: " << e.what();
			throw e;
		}		
	};

} /* namespace derivative */