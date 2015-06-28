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
#include "LIBORValueXigniteDAO.hpp"
#include "GroupRegister.hpp"
#include "EntityManager.hpp"
#include "EntityMgrUtil.hpp"
#include "IDataSource.hpp"
#include "CurrencyHolder.hpp"
#include "IIBOR.hpp"
#include "QFUtil.hpp"
#include "PrimaryAssetUtil.hpp"

using namespace utility;
using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;

namespace derivative
{
	GROUP_REGISTER(LIBORValueXigniteDAO);
	DAO_REGISTER(IIBORValue, XIGNITE, LIBORValueXigniteDAO);

	const int LIBORValueXigniteDAO::MaxCount = 100;
	std::shared_ptr<IMake> LIBORValueXigniteDAO::Make(const Name &nm)
	{
		/// Construct LIBORValueXigniteDAO from given name and register with EntityManager
		std::shared_ptr<LIBORValueXigniteDAO> dao = make_shared<LIBORValueXigniteDAO>(nm);
		dao = dynamic_pointer_cast<LIBORValueXigniteDAO>(EntityMgrUtil::registerObject(nm, dao));
		LOG(INFO) << " LIBORValueXigniteDAO  " << nm << " is constructed and registered with EntityManager" << endl;

		/// return constructed object if no exception is thrown
		return dao;
	}

	const std::shared_ptr<IObject> LIBORValueXigniteDAO::find(const Name& nm)
	{
		LOG(INFO) << "LIBORValueXigniteDAO::find(..) is called for " << nm << endl;
		/// If we are here means, LIBORValue object with the name nm is
		/// not in the registry. That's we should get the object from 
		/// Xignite.

		/// constructEntity could throw exception.
		/// let the caller of this function handle
		m_value = PrimaryUtil::ConstructEntity<IIBORValue>(nm);

		/// Now find and associate LIBOR with LIBORValue
		std::string curr;
		Maturity::MaturityType maturity;
		dd::date date;
		IIBORValue::GetKeys(nm, curr, maturity, date);
		Name LIBORName = IIBOR::ConstructName(curr, maturity);
		try
		{
			/// get the LIBOR from registry or from data source
			/// set the LIBOR object with LIBORValue
		//	std::shared_ptr<IIBOR> lrate = dynamic_pointer_cast<IIBOR>(EntityMgrUtil::findObject(LIBORName));
			m_value->SetRate(dynamic_pointer_cast<IIBOR>(EntityMgrUtil::findObject(LIBORName)));
		}
		catch (RegistryException& e)
		{
			LOG(WARNING) << " RegistryException thrown " << e.what() << endl;
			throw e;
		}
		catch (...)
		{
			LOG(WARNING) << " Unknown Exception thrown " << endl;
			throw;
		}

		/// Once we have the m_interestRate skeleton, it is time to populate the
		/// fields from the interestRate fetched from Xignite

		/// Populate the interestRate specific attributes
		findLIBORValue(date);
		m_value = dynamic_pointer_cast<IIBORValue>(EntityMgrUtil::registerObject(m_value->GetName(), m_value));
		/// now return m_interestRate
		return m_value;
	}

	Concurrency::task_status  LIBORValueXigniteDAO::findLIBORValue(const dd::date& issueDate)
	{
		/// Once we have the m_irValue skeleton, it is time to populate the
		/// fields from the rate value fetched from Xignite.
		/// build the URI to include all the query parameters
		/// http://www.xignite.com/xRates.json/GetRate?RateType=Prime&AsOfDate=12/27/2014 = > As of Date
		/// http://www.xignite.com/xRates.json/GetLatestRate?RateType=TreasuryConstant30Year = > latest rate

		web::http::uri_builder builder(U("http://www.xignite.com/"));
		/// Once have a builder instance, you can modify its components one by one:
		if (issueDate.is_not_a_date())
		{
			builder.set_path(U("xRates.json/GetLatestRate"));
		}
		else
		{
			builder.set_path(U("xRates.json/GetRate"));
		}

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
				m_value->SetLastRate(rate / 100);

				/// set date
				auto dateIter = v.as_object().find(U("Date"));
				auto dateStr = dateIter->second.as_string();

				/// get date
				auto repDate = getDateFromString(dateStr);
				m_value->SetReportedDate(repDate);
			}
			catch (const http_exception& e)
			{
				LOG(ERROR) << " Error in processing Rates data from Xignite: " << e.what() << endl;
				//throw e;				
			}
		}).wait();
	};

} /* namespace derivative */