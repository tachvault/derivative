/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#include "QFUtil.hpp"

namespace derivative
{

	bool verify_compare(double x,double y,double eps)
	{
		double ax = (x>0.0) ? x : -x;
		if (ax>eps) eps *= ax;
		x -= y;
		x = (x>0.0) ? x : -x;
		return (x<eps);       
	}

	void load_file(std::string& s, std::istream& is)
	{
		s.erase();
		if(is.bad()) return;
		char c;
		while(is.get(c)) 
		{
			if(s.capacity() == s.size())  s.reserve(s.capacity() * 3);
			s.append(1, c); 
		}
	}

	void splitLine(const std::string& line, std::vector<std::string>& vec, char delim)
	{
		auto it = line.cbegin();
		auto beg = it;
		for (; it != line.cend(); ++it)
		{
			if (*it == delim)
			{
				vec.push_back(std::string(beg, it));
				beg = it;
				++beg;
			}
		}
		if (beg != line.cend()) vec.push_back(std::string(beg, line.cend()));
	}

	pt::time_duration get_duration_from_string(std::string& str)
	{
		str.erase(std::remove(str.begin(), str.end(), '\"'), str.end());
		std::string durStr = str.substr(0, str.size() - 2);
		std::string ext = str.substr(str.size() - 2, str.size());
		boost::posix_time::time_duration dur = (ext.compare("pm") == 0) ? boost::posix_time::time_duration(12, 0, 0, 0) : \
			boost::posix_time::time_duration(0, 0, 0, 0);
		return dur + boost::posix_time::duration_from_string(durStr);
	}

	dd::date getDateFromString(const std::wstring& input)
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
			throw std::domain_error("Invalid date");
		}
	}
}
