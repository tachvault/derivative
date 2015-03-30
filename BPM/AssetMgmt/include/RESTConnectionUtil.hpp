/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_RESTCONNECTIONUTIL_H_
#define _DERIVATIVE_RESTCONNECTIONUTIL_H_

#include <iostream>
#include <string>
#include <regex>

#include "Global.hpp"
#include "DException.hpp"
#include "Exchange.hpp"
#include "ExchangeExt.hpp"
#include "IStock.hpp"

namespace derivative
{
	/// Provides REST connection utilities
	class  RESTConnectionUtil
	{
	public:

		static void splitLine(const std::string& line, std::vector<std::string>& vec)
		{
			auto it = line.cbegin();
			auto beg = it;
			for (; it != line.cend(); ++it)
			{
				if (*it == ',')
				{
					vec.push_back(std::string(beg, it));
					beg = it;
					++beg;
				}
			}
			if (beg != line.cend()) vec.push_back(std::string(beg, line.cend()));
		}

		static pt::time_duration get_duration_from_string(std::string& str)
		{
			str.erase(std::remove(str.begin(), str.end(), '\"'), str.end());
			std::string durStr = str.substr(0, str.size() - 2);
			std::string ext = str.substr(str.size() - 2, str.size());
			boost::posix_time::time_duration dur = (ext.compare("pm") == 0) ? boost::posix_time::time_duration(12, 0, 0, 0) : \
				boost::posix_time::time_duration(0, 0, 0, 0);
			return dur + boost::posix_time::duration_from_string(durStr);
		}

		static dd::date getDateFromString(const std::wstring& input)
		{
			//std::wregex rgx(L"(\\d+)/(\\d+)/(\\d+)\\s+(\\d+):(\\d+):(\\d+)\\s+(AM|PM)");
			std::wregex rgx(L"^(\\d+)/(\\d+)/(\\d+)");
			std::wsmatch match;
			if (std::regex_search(input.begin(), input.end(), match, rgx))
			{
				wchar_t* pEnd;
				auto m = std::wcstol(match[1].str().c_str(), &pEnd, 10);
				auto d = std::wcstol(match[2].str().c_str(), &pEnd, 10);
				auto y = std::wcstol(match[3].str().c_str(), &pEnd, 10);
				dd::date out(y, m, d);

				return out;
			}
			else
			{
				LOG(ERROR) << " unable to convert " << input.c_str() << endl;
				throw XigniteSrcException("Invalid date from Xignite");
			}
		}

		static std::string GetTickerSymbol(unsigned short src, std::shared_ptr<IStock> stock)
		{
			/// get the exchange corresponding to the ticker symbol
			ExchangeExt& symMap = ExchangeExt::getInstance();
			auto exchange = stock->GetExchange();

			/// using the exchange, get the extension
			auto ext = symMap.GetExchangeExt(src, exchange.GetExchangeName());

			/// now construct the complete symbol used by the given Data source
			if (ext.empty())
			{
				return stock->GetSymbol();
			}

			std::string symbol = stock->GetSymbol() + std::string(".") + ext;
			return symbol;
		}
	};
}

/// namespace derivative

#endif 
///_DERIVATIVE_RESTCONNECTIONUTIL_H_
