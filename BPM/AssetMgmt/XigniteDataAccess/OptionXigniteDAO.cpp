/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include <memory>
#include <iostream>
#include <regex>

#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <string.h>
#include <cpprest/json.h>

#include "Name.hpp"
#include "OptionXigniteDAO.hpp"
#include "GroupRegister.hpp"
#include "EntityManager.hpp"
#include "EntityMgrUtil.hpp"
#include "IDataSource.hpp"
#include "CountryHolder.hpp"
#include "IIR.hpp"
#include "RESTConnectionUtil.hpp"

using namespace utility;
using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;

namespace derivative
{
	GROUP_REGISTER(OptionXigniteDAO);

	std::shared_ptr<IMake> OptionXigniteDAO::Make(const Name &nm)
	{
		/// Construct OptionXigniteDAO from given name and register with EntityManager
		std::shared_ptr<OptionXigniteDAO> dao = make_shared<OptionXigniteDAO>(nm);
		EntityMgrUtil::registerObject(nm, dao);
		LOG(INFO) << " OptionXigniteDAO  " << nm << " is constructed and registered with EntityManager" << endl;

		/// return constructed object if no exception is thrown
		return dao;
	}

	const std::shared_ptr<IObject> OptionXigniteDAO::find(const Name& nm)
	{
		LOG(INFO) << "OptionXigniteDAO::find(..) is called for " << nm << endl;
		
		/// Populate the interestRate specific attributes
		findOption(nm);

		/// now return null.
		return nullptr;
	}

	Concurrency::task_status  OptionXigniteDAO::findOption(const Name& nm)
	{
		/// Once we have the m_Option skeleton, it is time to populate URL of 
		/// download file into named key
		///http://cloudfiles.xignite.com/xGlobalOptionsFile.json/GetFile?Date=12/30/2014&Exchange=OPRA&Type=Calls&_token=D4F26162807840BF881D2110243FA080
		
		/// get the primary keys
		std::string exchange;
		dd::date fileDate;
		HistoricalStockInfo::GetKeys(nm, symbolStr, startDate, endDate);

		/// append start date
		builder.append_query(L"a=" + std::to_wstring((startDate.month() - 1)));
		builder.append_query(L"b=" + std::to_wstring(startDate.day()));
		builder.append_query(L"c=" + std::to_wstring(startDate.year()));
		web::http::uri_builder builder(U("http://cloudfiles.xignite.com/"));
		/// Once have a builder instance, you can modify its components one by one:
		builder.set_path(U("xRates.json/GetLatestRate"));

		/// Include rate type
		utility::string_t type_str_t = utility::conversions::to_string_t(m_value->GetRate()->GetRateType());
		builder.append_query(L"RateType=" + type_str_t);

		/// include date only if date is valid
		if (!issueDate.is_not_a_date())
		{
			std::string rateDate = std::to_string(issueDate.month()) + "/" + std::to_string(issueDate.day()) + \
				"/" + std::to_string(issueDate.year());
			utility::string_t rateDate_str_t = utility::conversions::to_string_t(rateDate);
			builder.append_query(L"AsOfDate=" + rateDate_str_t);
		}

		/// add token
		std::string token("D4F26162807840BF881D2110243FA080");
		utility::string_t token_t = utility::conversions::to_string_t(token);
		builder.append_query(L"_token=" + token_t);

		//http_client client(L"http://www.xignite.com/xRates.json/");
		http_client client(L"http://www.xignite.com");
		return client.request(methods::GET, builder.to_string()).then([&](http_response response) -> pplx::task < json::value >
		{

			if (response.status_code() == status_codes::OK)
			{
				return response.extract_json();
			}

			// Handle error cases.
			LOG(ERROR) << "Xignite does return error condition: " << response.status_code() << endl;
			throw  XigniteSrcException("Xignite return error condition");
			/// return pplx::task_from_result(json::value());
		})
			.then([&](pplx::task<json::value> previousTask)
		{
			try
			{
				const json::value& v = previousTask.get();

				/// set rate
				auto rateStr = v.as_object().find(U("Text"))->second.as_string();
				rateStr.erase(std::remove(rateStr.begin(), rateStr.end(), '%'), rateStr.end());
				double rate = boost::lexical_cast<double>(rateStr);
				m_value->SetLastRate(rate/100);

				/// set date
				auto dateIter = v.as_object().find(U("Date"));
				auto dateStr = dateIter->second.as_string();

				/// get date from DESTConnectionUtil
				auto repDate = RESTConnectionUtil::getDateFromString(dateStr);
				m_value->SetReportedDate(repDate);
				//m_value->SetLastRate(v.as_double());

			}
			catch (const http_exception& e)
			{
				LOG(ERROR) << " Error in processing Rates data from Xignite: " << e.what() << endl;
				//throw e;				
			}
		}).wait();
	};

} /* namespace derivative */