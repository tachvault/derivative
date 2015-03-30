/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_WEBSERVICEUTIL_H_
#define _DERIVATIVE_WEBSERVICEUTIL_H_

#include <cpprest/filestream.h>
#include <cpprest/json.h>

#include "Global.hpp"
#include "EquityVanillaOptMessage.hpp"

using namespace utility;

namespace derivative
{
	class IMessage;
	namespace WebServiceUtil
	{
		web::json::value HandleEquityVanillaOption(EquityVanillaOptMessage::OptionTypeEnum opt, const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings);

		web::json::value HandleFuturesVanillaOption(EquityVanillaOptMessage::OptionTypeEnum opt, const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings);

		web::json::value HandleEquityOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings);

		web::json::value HandleFuturesOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings);
	}
}

/* namespace derivative */

#endif /* _DERIVATIVE_WEBSERVICEUTIL_H_ */