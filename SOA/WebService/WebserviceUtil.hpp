/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
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
		
		bool HandleEquityOption(const std::vector<string_t>& paths, \
			const std::map<string_t, string_t>& query_strings, web::json::value& msg);

		bool HandleFuturesOption(const std::vector<string_t>& paths, \
			const std::map<string_t, string_t>& query_strings, web::json::value& msg);

		bool HandleFXOption(const std::vector<string_t>& paths, \
			const std::map<string_t, string_t>& query_strings, web::json::value& msg);

		bool HandleEquityOptionSpread(const std::vector<string_t>& paths, \
			const std::map<string_t, string_t>& query_strings, web::json::value& msg);

		bool HandleFuturesOptionSpread(const std::vector<string_t>& paths, \
			const std::map<string_t, string_t>& query_strings, web::json::value& msg);

		bool HandleFXOptionSpread(const std::vector<string_t>& paths, \
			const std::map<string_t, string_t>& query_strings, web::json::value& msg);

		bool HandleEquityVanillaOption(const std::vector<string_t>& paths, \
			const std::map<string_t, string_t>& query_strings, web::json::value& msg);

		bool HandleEquityBarrierOption(const std::vector<string_t>& paths, \
			const std::map<string_t, string_t>& query_strings, web::json::value& msg);

		bool HandleEquityAverageOption(const std::vector<string_t>& paths, \
			const std::map<string_t, string_t>& query_strings, web::json::value& msg);

		bool HandleEquityLookBackOption(const std::vector<string_t>& paths, \
			const std::map<string_t, string_t>& query_strings, web::json::value& msg);

		bool HandleEquityChooserOption(const std::vector<string_t>& paths, \
			const std::map<string_t, string_t>& query_strings, web::json::value& msg);

		bool HandleEquityMargrabeOption(const std::vector<string_t>& paths, \
			const std::map<string_t, string_t>& query_strings, web::json::value& msg);

		bool HandleFuturesVanillaOption(const std::vector<string_t>& paths, \
			const std::map<string_t, string_t>& query_strings, web::json::value& msg);

		bool HandleFuturesBarrierOption(const std::vector<string_t>& paths, \
			const std::map<string_t, string_t>& query_strings, web::json::value& msg);

		bool HandleFuturesAverageOption(const std::vector<string_t>& paths, \
			const std::map<string_t, string_t>& query_strings, web::json::value& msg);

		bool HandleFXVanillaOption(const std::vector<string_t>& paths, \
			const std::map<string_t, string_t>& query_strings, web::json::value& msg);
	
		bool HandleFXBarrierOption(const std::vector<string_t>& paths, \
			const std::map<string_t, string_t>& query_strings, web::json::value& msg);

		bool HandleFXAverageOption(const std::vector<string_t>& paths, \
			const std::map<string_t, string_t>& query_strings, web::json::value& msg);
	}
}

/* namespace derivative */

#endif /* _DERIVATIVE_WEBSERVICEUTIL_H_ */