/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#include <memory>
#include "Name.hpp"
#include "FuturesMySQLDAO.hpp"
#include "Currency.hpp"
#include "GroupRegister.hpp"
#include "MySqlConnection.hpp"
#include "EntityManager.hpp"
#include "EntityMgrUtil.hpp"
#include "IDataSource.hpp"
#include "Exchange.hpp"
#include "ExchangeHolder.hpp"
#include "Country.hpp"
#include "CountryHolder.hpp"
#include "PrimaryAssetUtil.hpp"

namespace derivative
{
	GROUP_REGISTER(FuturesMySQLDAO);
	DAO_REGISTER(IFutures, MYSQL, FuturesMySQLDAO);

	const int FuturesMySQLDAO::MaxCount = 100;
	std::shared_ptr<IMake> FuturesMySQLDAO::Make(const Name &nm)
	{
		/// Construct FuturesMySQLDAO from given name and register with EntityManager
		std::shared_ptr<FuturesMySQLDAO> dao = make_shared<FuturesMySQLDAO>(nm);
		dao = dynamic_pointer_cast<FuturesMySQLDAO>(EntityMgrUtil::registerObject(nm, dao));
		LOG(INFO) << " FuturesMySQLDAO  " << nm << " is constructed and registered with EntityManager" << endl;

		/// return constructed object if no exception is thrown
		return dao;
	}

	const std::shared_ptr<IObject> FuturesMySQLDAO::find(const Name& nm)
	{
		LOG(INFO) << "FuturesMySQLDAO::find(..) is called for " << nm << endl;
		/// If we are here means, Futures object with the name nm is
		/// not in the registry. That's we should fetch the 
		/// object from the data data source.

		/// constructEntity could throw exception.
		/// let the caller of this function handle
		m_futures = PrimaryUtil::ConstructEntity<IFutures>(nm);

		/// Once we have the m_futures skeleton, it is time to populate the
		/// fields from the futures fetched from the database

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
		
		/// Populate the futures specific attributes
		findFutures(nm);
					
		/// now return m_futures
		return m_futures;
	}

	void FuturesMySQLDAO::findFutures(const Name& nm)
	{
		try
		{
			std::unique_ptr<sql::PreparedStatement> pstmt;
			std::unique_ptr<sql::ResultSet> res;

			/// symName, exName, descript, imVol, hVol
			pstmt.reset(m_con->prepareStatement ("CALL get_futuresByFuturesId(?, @symName, @exName, @descript, @impliedVol, @histVol, @count)"));
			pstmt->setUInt64(1, nm.GetObjId());
			pstmt->execute();

			pstmt.reset(m_con->prepareStatement("SELECT @symName AS _symbol, @exName AS _exName, @descript AS _description, @impliedVol AS _impliedVol, @histVol AS _histVol, @count AS _count"));			
			res.reset(pstmt->executeQuery());
			while (res->next())
			{
				auto count = res->getDouble("_count");
				if (count == 0) throw std::domain_error("Futures not found");

				std::string symbol = res->getString("_symbol").asStdString();
				m_futures->SetSymbol(symbol);

				/// get the exchange name 
				std::string exchangeName = res->getString("_exName").asStdString();
				/// Get the ExchangeHolder instance
                ExchangeHolder& exHolder = ExchangeHolder::getInstance();
				const Exchange& ex = exHolder.GetExchange(exchangeName);
				m_futures->SetExchange(ex);

				/// set the implied and historical vol
				double impliedVol = res->getDouble("_impliedVol");
				double histVol = res->getDouble("_histVol");
				m_futures->SetImpliedVol(impliedVol);
				m_futures->SetHistVol(histVol);

				std::string description = res->getString("_description").asStdString();
				m_futures->SetDescription(description);	
				m_futures = dynamic_pointer_cast<IFutures>(EntityMgrUtil::registerObject(m_futures->GetName(), m_futures));
				LOG(INFO) << " Futures object  " << m_futures->GetName() \
					      << " constructed with " << symbol \
					      << " " << exchangeName \
						  << " " << description << endl;
			}
		}
		catch (sql::SQLException &e) 
		{
			LOG(ERROR) << " No futures data found in source for " << m_name << endl;
			LOG(ERROR) << "# ERR: " << e.what();
			throw e;
		}		
	};

} /* namespace derivative */