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
#include "QFUtil.hpp"

using namespace utility;
using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace concurrency::streams;

namespace derivative
{
	Concurrency::task_status  OptionXigniteDAO::RetriveOptFileName()
	{
		/// Once we have the m_Option skeleton, it is time to populate URL of 
		/// download file into named key
		///http://cloudfiles.xignite.com/xGlobalOptionsFile.json/GetFile?Date=12/30/2014&Exchange=OPRA&Type=Calls&_token=D4F26162807840BF881D2110243FA080
		web::http::uri_builder builder(U("http://cloudfiles.xignite.com/"));

		/// Once have a builder instance, you can modify its components one by one:
		builder.set_path(U("xGlobalOptionsFile.json/GetFile"));

		/// include file date
		dd::date date = m_optFile.GetFileDate();
		std::string rateDate = std::to_string(date.month()) + "/" + std::to_string(date.day()) + \
			"/" + std::to_string(date.year());
		utility::string_t rateDate_str_t = utility::conversions::to_string_t(rateDate);
		builder.append_query(L"Date=" + rateDate_str_t);

		/// Include Exchange name
		string_t exchange = m_optFile.GetExchange();
		builder.append_query(L"Exchange=" + exchange);

		/// Include optionType name
		string_t str = (m_optFile.GetOptionType() == OptionFile::OptionType::VANILLA_CALL) ? U("Calls") : U("Puts");
		builder.append_query(L"Type=" + str);

		/// add token
		std::string token("D4F26162807840BF881D2110243FA080");
		utility::string_t token_t = utility::conversions::to_string_t(token);
		builder.append_query(L"_token=" + token_t);

		http_client client(L"http://cloudfiles.xignite.com");
		return client.request(methods::GET, builder.to_string()).then([&](http_response response) -> pplx::task < json::value >
		{

			if (response.status_code() == status_codes::OK)
			{
				return response.extract_json();
			}

			// Handle error cases.
			LOG(ERROR) << "Xignite does return error condition: " << response.status_code() << endl;
			throw  XigniteSrcException("Xignite return error condition");
		})
			.then([&](pplx::task<json::value> previousTask)
		{
			try
			{
				const json::value& v = previousTask.get();
				auto result = v.as_object().find(U("Outcome"))->second.as_string();
				if (result.compare(U("Success")))
				{
					LOG(ERROR) << " Unable to retrieve Option file URL. Xignite returns " << result.c_str() << endl;
					throw XigniteSrcException("Xignite returns failure");
				}

				/// Get url of cloud download file
				auto url = v.as_object().find(U("Url"))->second.as_string();
				m_optFile.SetFileURL(url);
			}
			catch (const http_exception& e)
			{
				LOG(ERROR) << " Error in processing Rates data from Xignite: " << e.what() << endl;
				//throw e;				
			}
		}).wait();
	};

	void OptionXigniteDAO::RetriveOptFile()
	{
		/// build the uri_builder with the url retrieved earlier
		const string_t url = m_optFile.GetFileURL();
		web::http::uri_builder builder(url);
		auto clientURL = builder.scheme() + U("://") + builder.host();

		/// set the output file to write option data
		const string_t fileName = utility::conversions::to_string_t(m_optFile.GetFileName());

		// Open a stream to the file to write the HTTP response body into.
		auto fileBuffer = std::make_shared<streambuf<uint8_t>>();
		file_buffer<uint8_t>::open(fileName, std::ios::out).then([&](streambuf<uint8_t> outFile) -> pplx::task < http_response >
		{
			*fileBuffer = outFile;

			// Create an HTTP request.
			// Encode the URI query since it could contain special characters like spaces.
			http_client client(clientURL);
			return client.request(methods::GET, builder.to_string());
		})
			// Write the response body into the file buffer.
			.then([=](http_response response) -> pplx::task < size_t >
		{
			LOG(INFO) << "Response status code %u returned " << response.status_code();
			return response.body().read_to_end(*fileBuffer);
		})
			// Close the file buffer.
			.then([=](size_t)
		{
			return fileBuffer->close();
		})
			// Wait for the entire response body to be written into the file.
			.wait();
	}

} /* namespace derivative */