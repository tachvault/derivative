/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/


#include <memory>
#include <string>
#include <iostream>
#include <sstream>
#include <boost/tokenizer.hpp> 
#include <boost/algorithm/string.hpp>

#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <string.h>

#include "Name.hpp"
#include "Currency.hpp"
#include "GroupRegister.hpp"
#include "EntityManager.hpp"
#include "EntityMgrUtil.hpp"

#include "IStock.hpp"
#include "IDataSource.hpp"

#include "HistoricalStockInfoYahooDAO.hpp"
#include "HistoricalStockInfo.hpp"
#include "IDailyStockValue.hpp"
#include "HistoricalStockInfo.hpp"

using namespace utility;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;

namespace derivative
{
	GROUP_REGISTER(HistoricalStockInfoYahooDAO);
	DAO_REGISTER(HistoricalStockInfo, YAHOO, HistoricalStockInfoYahooDAO);

	std::shared_ptr<IMake> HistoricalStockInfoYahooDAO::ConstructExemplar()
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

	std::shared_ptr<IDailyStockValue> HistoricalStockInfoYahooDAO::ConstructDailyStockValue(const shared_ptr<IStock>& stock, const std::shared_ptr<IMake> exemplar, const std::string& symbol, const std::string& tradeDate, \
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
		std::shared_ptr<IDailyStockValue> stockVal = dynamic_pointer_cast<IDailyStockValue>(dynamic_pointer_cast<IMake>(exemplar)->Make(entityName, agrs));

		return stockVal;
	}

	std::shared_ptr<IMake> HistoricalStockInfoYahooDAO::Make(const Name &nm)
	{
		/// Construct HistoricalStockInfoYahooDAO from given name and register with EntityManager
		std::shared_ptr<HistoricalStockInfoYahooDAO> dao = make_shared<HistoricalStockInfoYahooDAO>(nm);
		EntityMgrUtil::registerObject(nm, dao);
		LOG(INFO) << " HistoricalStockInfoYahooDAO  " << nm << " is constructed and registered with EntityManager" << endl;

		/// return constructed object if no exception is thrown
		return dao;
	}

	const std::shared_ptr<IObject> HistoricalStockInfoYahooDAO::find(const Name& nm)
	{
		LOG(INFO) << "HistoricalStockInfoYahooDAO::find(..) is called for " << nm << endl;

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

		/// Once we have the m_stockInfo skeleton, it is time to populate the
		/// fields for DailyStockValue fetched from Yahoo.

		/// build the URI to include all the query parameters

		/// To specify daily quote stop and start dates try this:
		/// “http://ichart.finance.yahoo.com/table.csv?s=” & ticker & 
		///         “&d=” & (endDate.Month – 1) & “&e=” & endDate.Day & “&f=” & endDate.Year & “&g=
		///          d&a=” & (startDate.Month – 1) & “&b=” & startDate.Day & “&c=” & startDate.Year & “&ignore=.csv”
		/// So to get AAPL from Jan 1, 2008 til May 2, 2011 you could use:
		/// http://ichart.finance.yahoo.com/table.csv?s=AAPL&d=4&e=2&f=2011&g=d&a=0&b=1&c=2008&ignore.csv

		web::http::uri_builder builder(U("http://ichart.finance.yahoo.com/"));
		/// Once have a builder instance, you can modify its components one by one:
		builder.set_path(U("table.csv"));

		/// get the primary keys
		std::string symbolStr;
		dd::date startDate;
		dd::date endDate;
		HistoricalStockInfo::GetKeys(nm, symbolStr, startDate, endDate);

		/// append start date
		builder.append_query(L"a=" + std::to_wstring((startDate.month() - 1)));
		builder.append_query(L"b=" + std::to_wstring(startDate.day()));
		builder.append_query(L"c=" + std::to_wstring(startDate.year()));

		/// append end date
		builder.append_query(L"d=" + std::to_wstring((endDate.month() - 1)));
		builder.append_query(L"e=" + std::to_wstring(endDate.day()));
		builder.append_query(L"f=" + std::to_wstring(endDate.year()));

		/// append query string for daily data
		builder.append_query(L"g=d");
		builder.append_query(L"ignore=.csv");
		http_client client(U("http://ichart.finance.yahoo.com/"));

		/// before fetching the data get the exemplar for IDailyStockInfo
		std::shared_ptr<IMake>  exemplar = ConstructExemplar();	

		/// get the stock data for the given symbol
		/// find the stock data and call setter
		std::shared_ptr<IObject> stockObj = EntityMgrUtil::findObject(Name(IStock::TYPEID, std::hash<std::string>()(symbolStr)));

		if (!stockObj)
		{
			throw DataSourceException("Unable to find stock data");
		}
		std::shared_ptr<IStock> stock = dynamic_pointer_cast<IStock>(stockObj);
		client.request(methods::GET, builder.to_string()).then([=](http_response response)
		{
			/// get all data into allData string variable
			Concurrency::streams::container_buffer<std::string> instringbuffer;
			response.body().read_to_end(instringbuffer).wait();
			std::string& allData = instringbuffer.collection();

			/// declare and initialize local variables.
			boost::char_separator<char> sep(","); 
			std::string dateStr;
			double open(0.0), close(0.0), high(0.0), low(0.0), adjClose(0.0);

			/// declare list of dailyStockValue
			deque<shared_ptr<IDailyStockValue> > stockValues;
			
			/// initialize input stream from allData
			std::istringstream input;
			input.str(allData);
			
			/// read and skip header.
			std::string header;
			std::getline(input, header);

			/// now read daily stock values one at a time
			/// FIXME: Is there a better way? Can we read directly from stream rather than 
			/// getting all into string and read from string?
			for (std::string line; std::getline(input, line); )
			{
				LOG(INFO) << "Processing: " << line << endl;			
				boost::tokenizer<boost::char_separator<char> >  tokens(line, sep); 
				auto it = tokens.begin();
				if (it != tokens.end()) dateStr = line;  else break;
				++it;
				if (it != tokens.end()) open = atof((*it).c_str()); else break;
				++it;
				if (it != tokens.end()) high = atof((*it).c_str()); else break;
				++it;
				if (it != tokens.end()) low = atof((*it).c_str()); else break;
				++it;
				if (it != tokens.end()) close = atof((*it).c_str()); else break;
				++it;
				if (it != tokens.end()) adjClose = atof((*it).c_str()); else break;

				/// create HistoricalStockInfo Object
				LOG(INFO) << " constructed with " << dateStr \
					<< " " << open  << " " << close  << " " << high \
					<< " " << open  << " " << open << std::endl;

				/// make DailyStockInfo
				std::shared_ptr<IDailyStockValue> dailyStockVal = ConstructDailyStockValue(stock, exemplar, symbolStr, dateStr, \
					open, high, low, close, adjClose);
				stockValues.push_back(dailyStockVal);			
			}
			m_stockInfo->SetDailyStockValues(stockValues);
		})
			// Wait for the entire response body to be de-serialized.
			.wait();

		return m_stockInfo;
	}	

} /* namespace derivative */