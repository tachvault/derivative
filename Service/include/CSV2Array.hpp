/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_CSV2ARRAY_H_
#define _DERIVATIVE_CSV2ARRAY_H_

#include <fstream>
#include <iostream>
#include <functional>

#include "MSWarnings.hpp"
#include <blitz/array.h>
#include <list>
#include <iterator>
#include <boost/regex.hpp>

#if defined _WIN32 || defined __CYGWIN__
  #ifdef SERVICEUTIL_EXPORTS
    #ifdef __GNUC__
      #define SERVICE_UTIL_DLL_API __attribute__ ((dllexport))
    #else
      #define SERVICE_UTIL_DLL_API __declspec(dllexport)
    #endif
  #else
    #ifdef __GNUC__
      #define SERVICE_UTIL_DLL_API __attribute__ ((dllimport))
    #else
      #define SERVICE_UTIL_DLL_API __declspec(dllimport)
    #endif
  #endif
  #define ESERVICE_UTIL_DLL_LOCAL
#else
  #if __GNUC__ >= 4
    #define SERVICE_UTIL_DLL_API __attribute__ ((visibility ("default")))
    #define SERVICE_UTIL_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define SERVICE_UTIL_DLL_API
    #define SERVICE_UTIL_DLL_LOCAL
  #endif
#endif

namespace derivative
{

	SERVICE_UTIL_DLL_API void load_file(std::string& s, std::istream& is);
	SERVICE_UTIL_DLL_API std::string make_string(const char* str);
	SERVICE_UTIL_DLL_API std::string make_string_strip_spaces(const char* str);

	/// Second argument is a function to convert a null-terminated C string into T
	template <class T> blitz::Array<T,2> CSV2Array(std::istream& is,std::function<T (const char *)> convert)
	{
		static const boost::regex e("\n",boost::regex::normal | boost::regbase::icase);
		static const boost::regex ei(",",boost::regex::normal | boost::regbase::icase);
		static const char* null = "";
		int j,k,n;
		std::string s;
		std::list<std::string> l;
		s.erase();
		load_file(s, is);
		std::vector<T> data(0);
		size_t cols = 0;
		size_t rows = 0;
		boost::regex_split(std::back_inserter(l), s, e);
		while(l.size())
		{
			std::list<std::string> li;
			s = *(l.begin());
			l.pop_front();
			boost::regex_split(std::back_inserter(li), s, ei);
			j = 0;
			while(li.size()) 
			{
				s = *(li.begin());
				li.pop_front();
				s.append(null,1);
				T d = convert(s.data());
				data.push_back(d);
				j++;          
			}
			cols = (j>cols) ? j : cols;
			rows++;  
		}
		blitz::Array<T,2> mat(rows,cols);
		n = 0;
		for (j=0;j<rows;j++)
		{
			for (k=0;k<cols;k++) 
			{
				if (n<data.size()) 
				{
					mat(j,k) = data[n];
					n++; 
				}
			}
		}
		return mat;
	}
}

/* namespace derivative */

#endif /* _DERIVATIVE_CSV2ARRAY_H_ */
