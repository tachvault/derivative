/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schl�gl
*/

#ifndef _DERIVATIVE_WEBSERVICEUTIL_H_
#define _DERIVATIVE_WEBSERVICEUTIL_H_

#include <vector>
#include <string>

#include <cpprest/filestream.h>
#include <cpprest/json.h>

#include "Global.hpp"
#include "EquityVanillaOptMessage.hpp"
#include "EquityOptionSpreadMessage.hpp"
#include "FuturesVanillaOptMessage.hpp"
#include "ExchangeRateVanillaOptMessage.hpp"
#include "QFUtil.hpp"

using namespace utility;

namespace derivative
{
	class IMessage;
	namespace WebServiceUtil
	{
		void decodeLegs(const std::string& line, std::vector<OptionSpreadMessage::Leg>& legs);

		void decodeOption(const std::string& opt, OptionSpreadMessage::Leg& leg);
		
		web::json::value HandleEquityOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings);

		web::json::value HandleFuturesOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings);

		web::json::value HandleFXOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings);

		web::json::value HandleEquityOptionSpread(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings);

		web::json::value HandleFuturesOptionSpread(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings);

		web::json::value HandleFXOptionSpread(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings);

		web::json::value HandleEquityVanillaOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings);

		web::json::value HandleEquityBarrierOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings);

		web::json::value HandleEquityAverageOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings);

		web::json::value HandleEquityLookBackOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings);

		web::json::value HandleEquityChooserOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings);

		web::json::value HandleEquityMargrabeOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings);

		web::json::value HandleFuturesVanillaOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings);

		web::json::value HandleFuturesBarrierOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings);

		web::json::value HandleFuturesAverageOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings);

		web::json::value HandleFXVanillaOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings);
	
		web::json::value HandleFXBarrierOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings);

		web::json::value HandleFXAverageOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings);	
	}
}

/* namespace derivative */

#endif /* _DERIVATIVE_WEBSERVICEUTIL_H_ */