/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_YAHOOCONNECTIONUTIL_H_
#define _DERIVATIVE_YAHOOCONNECTIONUTIL_H_

#include "Global.hpp"

namespace derivative
{
	/// Provides Yahoo connection utilities
	class  YahooConnectionUtil
	{
	public:

		static pt::time_duration get_duration_from_string(std::string& str)
		{
			str.erase (std::remove(str.begin(), str.end(), '\"'), str.end());
			std::string durStr = str.substr(0, str.size() - 2);
			std::string ext = str.substr(str.size() - 2, str.size());
			boost::posix_time::time_duration dur = (ext.compare("pm") == 0) ? boost::posix_time::time_duration(12,0,0,0) : \
				boost::posix_time::time_duration(0,0,0,0);
			return dur + boost::posix_time::duration_from_string(durStr);
		}
	};
}

/// namespace derivative

#endif 
///_DERIVATIVE_YAHOOCONNECTIONUTIL_H_
