/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include <memory>
#include "Name.hpp"
#include "HistoricalStockInfoMySQLDAO.hpp"
#include "Currency.hpp"
#include "GroupRegister.hpp"
#include "MySqlConnection.hpp"
#include "EntityManager.hpp"
#include "EntityMgrUtil.hpp"
#include "IDataSource.hpp"

namespace derivative
{
	GROUP_REGISTER(HistoricalStockInfoMySQLDAO);
	DAO_REGISTER(HistoricalStockInfo, MYSQL, HistoricalStockInfoMySQLDAO);

	std::shared_ptr<IMake> HistoricalStockInfoMySQLDAO::Make(const Name &nm)
	{
		/// Construct HistoricalStockInfoMySQLDAO from given name and register with EntityManager
		std::shared_ptr<HistoricalStockInfoMySQLDAO> dao = make_shared<HistoricalStockInfoMySQLDAO>(nm);
		EntityMgrUtil::registerObject(nm, dao);
		LOG(INFO) << " HistoricalStockInfoMySQLDAO  " << nm << " is constructed and registered with EntityManager" << endl;

		/// return constructed object if no exception is thrown
		return dao;
	}

	std::shared_ptr<IMake> HistoricalStockInfoMySQLDAO::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("Not implemented");
	}

	std::shared_ptr<IMake> HistoricalStockInfoMySQLDAO::ConstructExemplar()
	{
		/// First get the StockValue exemplar object from the registry
		/// Get the EntityManager instance
		EntityManager& entMgr= EntityManager::getInstance();	

		/// get the concrete types for the given alias name
		std::vector<Name> names;
		Name nm(IDailyStockValue::TYPEID, 0);
		grpType grpId = entMgr.findAlias(nm);

		/// get the examplar object for the StockValue
		/// Exemplar objects should be initialized
		/// during global initialization time.
		std::shared_ptr<IObject> exemplar = entMgr.findObject(Name(grpId));

		return dynamic_pointer_cast<IMake>(exemplar);
	}

	std::shared_ptr<IDailyStockValue> HistoricalStockInfoMySQLDAO::ConstructDailyStockValue(const shared_ptr<IStock>& stock, const std::shared_ptr<IMake> exemplar, \
		std::string& symbol, std::string& tradeDate, \
		double open, double high, double low, double close, double adjClose)
	{

		Name entityName(exemplar->GetName().GetGrpId(), std::hash<std::string>()(symbol + tradeDate));

		entityName.AppendKey(string("symbol"), boost::any_cast<string>(symbol));
		entityName.AppendKey(string("tradeDate"), boost::any_cast<string>(tradeDate));		

		/// Make the exemplar StockValue to construct 
		/// StockValue for the given stockVal	
		std::deque<boost::any> agrs;
		agrs.push_back(boost::any_cast<double>(open));
		agrs.push_back(boost::any_cast<double>(close));
		agrs.push_back(boost::any_cast<double>(high));
		agrs.push_back(boost::any_cast<double>(low));
		agrs.push_back(boost::any_cast<double>(adjClose));
		agrs.push_back(boost::any_cast<dd::date>(dd::from_simple_string(tradeDate)));
		std::shared_ptr<IDailyStockValue> dailyStockVal = dynamic_pointer_cast<IDailyStockValue>(dynamic_pointer_cast<IMake>(exemplar)->Make(entityName, agrs));


		dailyStockVal->SetStock(stock);
		return dailyStockVal;
	}

	const std::shared_ptr<IObject> HistoricalStockInfoMySQLDAO::find(const Name& nm)
	{
		LOG(INFO) << "HistoricalStockInfoMySQLDAO::find(..) is called for " << nm << endl;

		/// find the HistoricStockInfo from registry
		EntityManager& entMgr= EntityManager::getInstance();	
		std::shared_ptr<IObject> obj = entMgr.findObject(nm);
		if (obj != nullptr)
		{
			m_stockInfo = dynamic_pointer_cast<HistoricalStockInfo>(obj);
		}
		else
		{
			throw RegistryException("Unable to find HistoricalStockInfo");
		}

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
		findHistoricalStockInfo();

		/// now return m_stock
		return m_stockInfo;
	}

	void HistoricalStockInfoMySQLDAO::insert()
	{
		try
		{
			std::unique_ptr<sql::PreparedStatement> pstmt;			
			deque<shared_ptr<IDailyStockValue> > stockValues = m_stockInfo->GetDailyStockValues();
			for (auto &stock: stockValues)
			{
				/// id, tradedate, openprice, high, low, closeprice, adjClose
				pstmt.reset(m_con->prepareStatement ("CALL get_stockByHistoricalStockInfoId(?, ?, ?, ?, ?, ?, ?)"));
				pstmt->setUInt64(1, stock->GetName().GetObjId());
				pstmt->setString(2, dd::to_simple_string(stock->GetTradeDate()).c_str());
				pstmt->setDouble(3, stock->GetPriceOpen());
				pstmt->setDouble(4, stock->GetPriceHigh());
				pstmt->setDouble(5, stock->GetPriceLow());
				pstmt->setDouble(6, stock->GetPriceClose());
				pstmt->setDouble(7, stock->GetPriceAdjClose());				
				pstmt->execute();
			}
		}
		catch (sql::SQLException &e) 
		{
			LOG(ERROR) << " Insert throws exception " << m_name << endl;
			LOG(ERROR) << "# ERR: " << e.what();
			throw e;
		}		
	}

	void HistoricalStockInfoMySQLDAO::findHistoricalStockInfo()
	{
		try
		{
			std::unique_ptr<sql::PreparedStatement> pstmt;
			std::unique_ptr<sql::ResultSet> res;
			deque<shared_ptr<IDailyStockValue> > data;

			/// before fetching the data get the exemplar for IDailyStockInfo
			std::shared_ptr<IMake>  exemplar = ConstructExemplar();

			/// get the primary keys
			std::string symbolStr;
			dd::date startDate;
			dd::date endDate;
			HistoricalStockInfo::GetKeys(m_name, symbolStr, startDate, endDate);

			pstmt.reset(m_con->prepareStatement ("CALL get_stockByHistoricalStockInfoId(?, ?, ?, @tradeDate, @open, @high, @low, @close, @adjClose)"));
			pstmt->setUInt64(1, m_name.GetObjId());
			std::string startStringStr = dd::to_simple_string(startDate);
			std::string endStringStr = dd::to_simple_string(endDate);

			/// get the stock data for the given symbol
			/// find the stock data and call setter
			std::shared_ptr<IObject> obj = EntityMgrUtil::findObject(Name(IStock::TYPEID, std::hash<std::string>()(symbolStr)));

			if (!obj)
			{
				throw DataSourceException("Unable to find stock data");
			}

			shared_ptr<IStock> stock = dynamic_pointer_cast<IStock>(obj);
			pstmt->setString(2, startStringStr.c_str());
			pstmt->setString(3, endStringStr.c_str());
			pstmt->execute();

			pstmt.reset(m_con->prepareStatement("SELECT @tradedate AS _date, @open AS _open, @high AS _high, @close AS _close, @adjClose AS _adjClose"));			
			res.reset(pstmt->executeQuery());
			LOG(INFO) << " HistoricalStockInfo object for " << symbolStr << ", " << startStringStr << ", " << endStringStr << endl;
			while (res->next())
			{
				std::string dateStr = res->getString("_date").asStdString();
				double open = res->getDouble("_open");
				double high = res->getDouble("_high");
				double low = res->getDouble("_low");
				double close = res->getDouble("_close");
				double adjClose = res->getDouble("_adjClose");

				/// create HistoricalStockInfo Object
				LOG(INFO) << " constructed with " << dateStr \
					<< " " << open  << " " << close  << " " << high \
					<< " " << open  << " " << open << std::endl;

				/// make DailyStockInfo
				std::shared_ptr<IDailyStockValue> dailyStockVal = ConstructDailyStockValue(stock, exemplar, symbolStr, dateStr, \
					open, high, low, close, adjClose);
				data.push_back(dailyStockVal);

			}

			/// got all the data
			m_stockInfo->SetDailyStockValues(data);
		}
		catch (sql::SQLException &e) 
		{
			LOG(ERROR) << " No historical stock data found in source for " << m_name << endl;
			LOG(ERROR) << "# ERR: " << e.what();
			throw e;
		}		
	};

} /* namespace derivative */