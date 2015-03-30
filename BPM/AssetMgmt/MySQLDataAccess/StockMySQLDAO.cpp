/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#include <memory>
#include "Name.hpp"
#include "StockMySQLDAO.hpp"
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
	GROUP_REGISTER(StockMySQLDAO);
	DAO_REGISTER(IStock, MYSQL, StockMySQLDAO);

	std::shared_ptr<IMake> StockMySQLDAO::Make(const Name &nm)
	{
		/// Construct StockMySQLDAO from given name and register with EntityManager
		std::shared_ptr<StockMySQLDAO> dao = make_shared<StockMySQLDAO>(nm);
		EntityMgrUtil::registerObject(nm, dao);
		LOG(INFO) << " StockMySQLDAO  " << nm << " is constructed and registered with EntityManager" << endl;

		/// return constructed object if no exception is thrown
		return dao;
	}

	const std::shared_ptr<IObject> StockMySQLDAO::find(const Name& nm)
	{
		LOG(INFO) << "StockMySQLDAO::find(..) is called for " << nm << endl;
		/// If we are here means, Stock object with the name nm is
		/// not in the registry. That's we should fetch the 
		/// object from the data data source.

		/// constructEntity could throw exception.
		/// let the caller of this function handle
		m_stock = PrimaryUtil::ConstructEntity<IStock>(nm);

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
		findStock();
					
		/// now return m_stock
		return m_stock;
	}

	void StockMySQLDAO::findStock()
	{
		try
		{
			std::unique_ptr<sql::PreparedStatement> pstmt;
			std::unique_ptr<sql::ResultSet> res;

			pstmt.reset(m_con->prepareStatement ("CALL get_stockByStockId(?, @symName, @exName, @descript, @cntry, @impliedVol, @histVol)"));
			pstmt->setUInt64(1, m_name.GetObjId());
			pstmt->execute();

			pstmt.reset(m_con->prepareStatement("SELECT @symName AS _symbol, @exName AS _exName, @descript AS _description, @cntry AS _cntry, @impliedVol AS _impliedVol, @histVol AS _histVol"));			
			res.reset(pstmt->executeQuery());
			while (res->next())
			{
				std::string symbol = res->getString("_symbol").asStdString();
				m_stock->SetSymbol(symbol);

				/// get the exchange name 
				std::string exchangeName = res->getString("_exName").asStdString();
				/// Get the ExchangeHolder instance
                ExchangeHolder& exHolder = ExchangeHolder::getInstance();
				const Exchange& ex = exHolder.GetExchange(exchangeName);
				m_stock->SetExchange(ex);

				/// get the country code 
				std::string cntryCode = res->getString("_cntry").asStdString();
				/// Get the CountryHolder instance
                CountryHolder& cntryHolder = CountryHolder::getInstance();
				const Country& cntry= cntryHolder.GetCountry(cntryCode);
				m_stock->SetCountry(cntry);

				/// set the implied and historical vol
				double impliedVol = res->getDouble("_impliedVol");
				double histVol = res->getDouble("_histVol");
				m_stock->SetImpliedVol(impliedVol);
				m_stock->SetHistVol(histVol);

				std::string description = res->getString("_description").asStdString();
				m_stock->SetDescription(description);		
				LOG(INFO) << " Stock object  " << m_stock->GetName() \
					      << " constructed with " << symbol \
					      << " " << exchangeName \
						  << " " << description  \
						  << " " << cntryCode << endl;
			}
		}
		catch (sql::SQLException &e) 
		{
			LOG(ERROR) << " No stock data found in source for " << m_name << endl;
			LOG(ERROR) << "# ERR: " << e.what();
			throw e;
		}		
	};

} /* namespace derivative */