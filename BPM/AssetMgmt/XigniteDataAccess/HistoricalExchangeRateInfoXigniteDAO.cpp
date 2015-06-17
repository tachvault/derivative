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
#include "QFUtil.hpp"

#include "IExchangeRate.hpp"
#include "IDataSource.hpp"

#include "HistoricalExchangeRateInfoXigniteDAO.hpp"
#include "HistoricalExchangeRateInfo.hpp"
#include "IDailyExchangeRateValue.hpp"

using namespace utility;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;

namespace derivative
{
	GROUP_REGISTER(HistoricalExchangeRateInfoXigniteDAO);
	DAO_REGISTER(HistoricalExchangeRateInfo, XIGNITE, HistoricalExchangeRateInfoXigniteDAO);

	const int HistoricalExchangeRateInfoXigniteDAO::MaxCount = 100;
	std::shared_ptr<IMake> HistoricalExchangeRateInfoXigniteDAO::ConstructExemplar()
	{
		/// First get the ExchangeRateValue exemplar object from the registry
		/// Get the EntityManager instance
		EntityManager& entMgr = EntityManager::getInstance();

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

	std::shared_ptr<IDailyExchangeRateValue> HistoricalExchangeRateInfoXigniteDAO::ConstructDailyExchangeRateValue(\
		const shared_ptr<IExchangeRate>& rate, const std::shared_ptr<IMake> exemplar, \
		const std::string& domestic, const std::string& foreign, const std::string& tradeDate, \
		double open, double high, double low, double close, double adjClose)
	{

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
		agrs.push_back(boost::any_cast<double>(adjClose));
		agrs.push_back(boost::any_cast<dd::date>(dd::from_us_string(tradeDate)));
		std::shared_ptr<IDailyExchangeRateValue> exchangeRateVal = dynamic_pointer_cast<IDailyExchangeRateValue>(dynamic_pointer_cast<IMake>(exemplar)->Make(entityName, agrs));

		return exchangeRateVal;
	}

	std::shared_ptr<IMake> HistoricalExchangeRateInfoXigniteDAO::Make(const Name &nm)
	{
		/// Construct HistoricalExchangeRateInfoXigniteDAO from given name and register with EntityManager
		std::shared_ptr<HistoricalExchangeRateInfoXigniteDAO> dao = make_shared<HistoricalExchangeRateInfoXigniteDAO>(nm);
		dao = dynamic_pointer_cast<HistoricalExchangeRateInfoXigniteDAO>(EntityMgrUtil::registerObject(nm, dao));
		LOG(INFO) << " HistoricalExchangeRateInfoXigniteDAO  " << nm << " is constructed and registered with EntityManager" << endl;

		/// return constructed object if no exception is thrown
		return dao;
	}

	const std::shared_ptr<IObject> HistoricalExchangeRateInfoXigniteDAO::find(const Name& nm)
	{
		LOG(INFO) << "HistoricalExchangeRateInfoXigniteDAO::find(..) is called for " << nm << endl;

		/// find the HistoricExchangeRateInfo from registry
		EntityManager& entMgr = EntityManager::getInstance();
		std::shared_ptr<IObject> obj = entMgr.findObject(nm);
		if (obj != nullptr)
		{
			m_rateInfo = dynamic_pointer_cast<HistoricalExchangeRateInfo>(obj);
		}
		else
		{
			throw RegistryException("Unable to find HistoricalExchangeRateInfo");
		}

		/// Once we have the m_exchangeRateInfo skeleton, it is time to populate the
		/// fields for DailyExchangeRateValue fetched from Yahoo.

		/// build the URI to include all the query parameters

		/// To specify daily quote stop and start dates try this:
		/// http://globalcurrencies.xignite.com/xGlobalCurrencies.csv/GetHistoricalRatesRanges?_token=D4F26162807840BF881D2110243FA080&Symbols=EURUSD&PriceType=Mid&StartDate=4/19/2015&EndDate=4/26/2015&PeriodType=Daily&FixingTime=22:00 
		/// Domestic (Base ) currrency - EUR
		/// Foreign (quote) currency - USD
		web::http::uri_builder builder(U("http://globalcurrencies.xignite.com/"));
		/// Once have a builder instance, you can modify its components one by one:
		builder.set_path(U("xGlobalCurrencies.csv/GetHistoricalRatesRanges"));

		/// get the primary keys
		std::string domesticStr;
		std::string foreignStr;
		dd::date startDate;
		dd::date endDate;
		HistoricalExchangeRateInfo::GetKeys(nm, domesticStr, foreignStr, startDate, endDate);

		/// append base and quoted currencies
		utility::string_t baseCurr = utility::conversions::to_string_t(domesticStr);
		utility::string_t quotedCurr = utility::conversions::to_string_t(foreignStr);

		builder.append_query(L"_token=D4F26162807840BF881D2110243FA080");
		builder.append_query(L"Symbols=" + baseCurr + quotedCurr);
		builder.append_query(L"PriceType=Mid");
		std::string sdate = to_string(startDate.month()) + "/" + to_string(startDate.day()) + "/" + to_string(startDate.year());
		builder.append_query(L"StartDate=" + utility::conversions::to_string_t(sdate));

		std::string edate = to_string(endDate.month()) + "/" + to_string(endDate.day()) + "/" + to_string(endDate.year());
		builder.append_query(L"EndDate=" + utility::conversions::to_string_t(edate));

		builder.append_query(L"PeriodType=Daily");
		builder.append_query(L"FixingTime=22:00");

		http_client client(U("http://globalcurrencies.xignite.com/"));

		/// before fetching the data get the exemplar for IDailyExchangeRateInfo
		std::shared_ptr<IMake>  exemplar = ConstructExemplar();

		/// get the exchangeRate data for the given symbol
		/// find the exchangeRate data and call setter
		std::shared_ptr<IObject> exchangeRateObj = EntityMgrUtil::findObject(\
			Name(IExchangeRate::TYPEID, std::hash<std::string>()(domesticStr + foreignStr)));

		if (!exchangeRateObj)
		{
			throw DataSourceException("Unable to find exchangeRate data");
		}
		std::shared_ptr<IExchangeRate> exchangeRate = dynamic_pointer_cast<IExchangeRate>(exchangeRateObj);
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

			/// declare list of dailyExchangeRateValue
			deque<shared_ptr<IDailyExchangeRateValue> > exchangeRateValues;

			/// initialize input stream from allData
			std::istringstream input;
			input.str(allData);

			/// read and skip header.
			std::string header;
			std::getline(input, header);

			/// now read daily exchangeRate values one at a time
			/// FIXME: Is there a better way? Can we read directly from stream rather than 
			/// getting all into string and read from string?
			for (std::string line; std::getline(input, line);)
			{
				std::vector<string> vec;
				splitLine(line, vec);

				/// skip if no data
				if (vec.empty()) break;
				if (!vec[9].empty()) dateStr = vec[9]; else break;
				if (!vec[11].empty()) open = atof(vec[11].c_str()); else break;
				if (!vec[11].empty()) high = atof(vec[12].c_str()); else break;
				if (!vec[11].empty()) low = atof(vec[13].c_str()); else break;
				if (!vec[11].empty()) close = atof(vec[14].c_str()); else break;
				if (!vec[11].empty()) adjClose = atof(vec[15].c_str()); else break;

				/// create HistoricalExchangeRateInfo Object
				LOG(INFO) << " constructed with " << dateStr \
					<< " " << open << " " << close << " " << high \
					<< " " << open << " " << open << std::endl;

				/// make DailyExchangeRateInfo
				std::shared_ptr<IDailyExchangeRateValue> dailyExchangeRateVal = ConstructDailyExchangeRateValue(\
					exchangeRate, exemplar, domesticStr, foreignStr, dateStr, open, high, low, close, adjClose);

				exchangeRateValues.push_back(dailyExchangeRateVal);
			}
			m_rateInfo->SetDailyExchangeRateValues(exchangeRateValues);
		})
			// Wait for the entire response body to be de-serialized.
			.wait();

		m_rateInfo = dynamic_pointer_cast<HistoricalExchangeRateInfo>(EntityMgrUtil::registerObject(m_rateInfo->GetName(), m_rateInfo));
		return m_rateInfo;
	}

} /* namespace derivative */