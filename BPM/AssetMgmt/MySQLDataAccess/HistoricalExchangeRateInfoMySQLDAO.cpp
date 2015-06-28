/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include <memory>
#include "Name.hpp"
#include "HistoricalExchangeRateInfoMySQLDAO.hpp"
#include "Currency.hpp"
#include "GroupRegister.hpp"
#include "MySqlConnection.hpp"
#include "EntityManager.hpp"
#include "EntityMgrUtil.hpp"
#include "IDataSource.hpp"

namespace derivative
{
	GROUP_REGISTER(HistoricalExchangeRateInfoMySQLDAO);
	DAO_REGISTER(HistoricalExchangeRateInfo, MYSQL, HistoricalExchangeRateInfoMySQLDAO);
	
	const int HistoricalExchangeRateInfoMySQLDAO::MaxCount = 100;
	std::shared_ptr<IMake> HistoricalExchangeRateInfoMySQLDAO::Make(const Name &nm)
	{
		/// Construct HistoricalExchangeRateInfoMySQLDAO from given name and register with EntityManager
		std::shared_ptr<HistoricalExchangeRateInfoMySQLDAO> dao = make_shared<HistoricalExchangeRateInfoMySQLDAO>(nm);
		dao = dynamic_pointer_cast<HistoricalExchangeRateInfoMySQLDAO>(EntityMgrUtil::registerObject(nm, dao));
		LOG(INFO) << " HistoricalExchangeRateInfoMySQLDAO  " << nm << " is constructed and registered with EntityManager" << endl;

		/// return constructed object if no exception is thrown
		return dao;
	}

	std::shared_ptr<IMake> HistoricalExchangeRateInfoMySQLDAO::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("Not implemented");
	}

	std::shared_ptr<IMake> HistoricalExchangeRateInfoMySQLDAO::ConstructExemplar()
	{
		/// First get the ExchangeRateValue exemplar object from the registry
		/// Get the EntityManager instance
		EntityManager& entMgr= EntityManager::getInstance();	

		/// get the concrete types for the given alias name
		std::vector<Name> names;
		Name nm(IDailyExchangeRateValue::TYPEID, 0);
		grpType grpId = entMgr.findAlias(nm);

		/// get the examplar object for the ExchangeRateValue
		/// Exemplar objects should be initialized
		/// during global initialization time.
		std::shared_ptr<IObject> exemplar = entMgr.findObject(Name(grpId));

		return dynamic_pointer_cast<IMake>(exemplar);
	}

	std::shared_ptr<IDailyExchangeRateValue> HistoricalExchangeRateInfoMySQLDAO::ConstructDailyExchangeRateValue(const shared_ptr<IExchangeRate>& exchangeRate, \
		const std::shared_ptr<IMake> exemplar, size_t forex_id, std::string& tradeDate, \
		double open, double high, double low, double close)
	{
		auto domestic = exchangeRate->GetDomesticCurrency().GetCode();
		auto foreign = exchangeRate->GetForeignCurrency().GetCode();
		Name entityName(exemplar->GetName().GetGrpId(), std::hash<std::string>()(domestic + foreign + tradeDate));

		entityName.AppendKey(string("domestic"), boost::any_cast<string>(domestic));
		entityName.AppendKey(string("foreign"), boost::any_cast<string>(foreign));
		entityName.AppendKey(string("tradeDate"), boost::any_cast<string>(tradeDate));		

		/// Make the exemplar ExchangeRateValue to construct 
		/// ExchangeRateValue for the given exchangeRateVal	
		std::deque<boost::any> agrs;
		agrs.push_back(boost::any_cast<double>(open));
		agrs.push_back(boost::any_cast<double>(close));
		agrs.push_back(boost::any_cast<double>(high));
		agrs.push_back(boost::any_cast<double>(low));
		agrs.push_back(boost::any_cast<dd::date>(dd::from_simple_string(tradeDate)));
		std::shared_ptr<IDailyExchangeRateValue> dailyExchangeRateVal = dynamic_pointer_cast<IDailyExchangeRateValue>(dynamic_pointer_cast<IMake>(exemplar)->Make(entityName, agrs));
				
		dailyExchangeRateVal->SetExchangeRate(exchangeRate);
		return dailyExchangeRateVal;
	}

	const std::shared_ptr<IObject> HistoricalExchangeRateInfoMySQLDAO::find(const Name& nm)
	{
		LOG(INFO) << "HistoricalExchangeRateInfoMySQLDAO::find(..) is called for " << nm << endl;

		/// find the HistoricExchangeRateInfo from registry
		EntityManager& entMgr= EntityManager::getInstance();	
		std::shared_ptr<IObject> obj = entMgr.findObject(nm);
		if (obj != nullptr)
		{
			m_exchangeRateInfo = dynamic_pointer_cast<HistoricalExchangeRateInfo>(obj);
		}
		else
		{
			throw RegistryException("Unable to find HistoricalExchangeRateInfo");
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

		/// Populate the exchangeRate specific attributes
		findHistoricalExchangeRateInfo();

		/// now return m_exchangeRate
		return m_exchangeRateInfo;
	}

	void HistoricalExchangeRateInfoMySQLDAO::insert()
	{
		try
		{
			std::unique_ptr<sql::PreparedStatement> pstmt;			
			deque<shared_ptr<IDailyExchangeRateValue> > exchangeRateValues = m_exchangeRateInfo->GetDailyExchangeRateValues();
			for (auto &exchangeRate: exchangeRateValues)
			{
				/// id, tradedate, openprice, high, low, closeprice
				pstmt.reset(m_con->prepareStatement ("CALL insert_dailyForexValue(?, ?, ?, ?, ?, ?)"));
				auto exRate = exchangeRate->GetExchangeRate();
				auto forex_id = std::hash<std::string>()(exRate->GetDomesticCurrency().GetCode() + exRate->GetForeignCurrency().GetCode());
				pstmt->setUInt64(1, forex_id);
				pstmt->setString(2, dd::to_simple_string(exchangeRate->GetTradeDate()).c_str());
				pstmt->setDouble(3, exchangeRate->GetPriceOpen());
				pstmt->setDouble(4, exchangeRate->GetPriceHigh());
				pstmt->setDouble(5, exchangeRate->GetPriceLow());
				pstmt->setDouble(6, exchangeRate->GetPriceClose());			
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

	void HistoricalExchangeRateInfoMySQLDAO::findHistoricalExchangeRateInfo()
	{
		try
		{
			std::unique_ptr<sql::PreparedStatement> pstmt;
			std::unique_ptr<sql::ResultSet> res;
			deque<shared_ptr<IDailyExchangeRateValue> > data;

			/// before fetching the data get the exemplar for IDailyExchangeRateInfo
			std::shared_ptr<IMake>  exemplar = ConstructExemplar();
			
			std::string domestic;
			std::string foreign;
			dd::date startDate;
			dd::date endDate;
			HistoricalExchangeRateInfo::GetKeys(m_name, domestic, foreign, startDate, endDate);  
			
			pstmt.reset(m_con->prepareStatement ("CALL get_dailyForexValue(?, ?, ?, @tradeDate, @open, @high, @low, @close)"));
			auto forex_id = std::hash<std::string>()(m_exchangeRateInfo->GetDomesticCurrency().GetCode() + m_exchangeRateInfo->GetForeignCurrency().GetCode());
			pstmt->setUInt64(1, forex_id);
			
			/// get the exchangeRate data for the given symbol
			/// find the exchangeRate data and call setter
		    std::shared_ptr<IObject> obj = EntityMgrUtil::findObject(Name(IExchangeRate::TYPEID, forex_id));

			if (!obj)
			{
				throw DataSourceException("Unable to find exchangeRate data");
			}

			shared_ptr<IExchangeRate> exchangeRate = dynamic_pointer_cast<IExchangeRate>(obj);
			pstmt->setString(2, dd::to_iso_extended_string(startDate).c_str());
			pstmt->setString(3, dd::to_iso_extended_string(endDate).c_str());
			pstmt->execute();

			pstmt.reset(m_con->prepareStatement("SELECT @tradedate AS _date, @open AS _open, @high AS _high, @close AS _close"));			
			res.reset(pstmt->executeQuery());
			LOG(INFO) << " HistoricalExchangeRateInfo object for " << m_exchangeRateInfo->GetDomesticCurrency().GetCode() << ", " 
				      << m_exchangeRateInfo->GetForeignCurrency().GetCode() \
				      << ", " << dd::to_iso_extended_string(startDate) << ", " << dd::to_iso_extended_string(endDate) << endl;
			while (res->next())
			{
				std::string dateStr = res->getString("_date").asStdString();
				long double open = res->getDouble("_open");
				long double high = res->getDouble("_high");
				long double low = res->getDouble("_low");
				long double close = res->getDouble("_close");

				/// create HistoricalExchangeRateInfo Object
				LOG(INFO) << " constructed with " \
					<< " " << open  << " " << close  << " " << high \
					<< " " << open  << " " << open << std::endl;

				/// make DailyExchangeRateInfo
				std::shared_ptr<IDailyExchangeRateValue> dailyExchangeRateVal = ConstructDailyExchangeRateValue(exchangeRate, exemplar, \
					forex_id, dateStr, open, high, low, close);
				dailyExchangeRateVal = dynamic_pointer_cast<IDailyExchangeRateValue>(EntityMgrUtil::registerObject(dailyExchangeRateVal->GetName(), dailyExchangeRateVal));
				data.push_back(dailyExchangeRateVal);

			}

			/// got all the data
			m_exchangeRateInfo->SetDailyExchangeRateValues(data);
		}
		catch (sql::SQLException &e) 
		{
			LOG(ERROR) << " No exchangeRate data found in source for " << m_name << endl;
			LOG(ERROR) << "# ERR: " << e.what();
			throw e;
		}		
	};

} /* namespace derivative */