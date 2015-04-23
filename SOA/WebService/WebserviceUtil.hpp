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
#include "QFUtil.hpp"

using namespace utility;

namespace derivative
{
	class IMessage;
	namespace WebServiceUtil
	{
		void decodeLegs(const std::string& line, std::vector<EquityOptionSpreadMessage::Leg>& legs);

		void decodeOption(const std::string& opt, EquityOptionSpreadMessage::Leg& leg);
		
		web::json::value HandleEquityVanillaOption(EquityVanillaOptMessage::OptionTypeEnum opt, const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings);

		web::json::value HandleFuturesVanillaOption(EquityVanillaOptMessage::OptionTypeEnum opt, const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings);

		web::json::value HandleEquityOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings);

		web::json::value HandleFuturesOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings);

		web::json::value HandleEquityOptionSpread(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings);
	
		web::json::value HandleFuturesOptionSpread(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings);

		void HandleEquityOptionLegs(EquityVanillaOptMessage::Request &req, std::string& legs);
	}
}

/* namespace derivative */

#endif /* _DERIVATIVE_WEBSERVICEUTIL_H_ */