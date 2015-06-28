/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_QFUTIL_H_
#define _DERIVATIVE_QFUTIL_H_

#include <string>
#include <iostream>
#include <vector>
#include <regex>

#include "Global.hpp"

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
	bool SERVICE_UTIL_DLL_API verify_compare(double x,double y,double eps);

	void SERVICE_UTIL_DLL_API load_file(std::string& s, std::istream& is);
	
	template <class T>
	bool between(const T& x,const T& a,const T& b)
	{
		T lower = (a<b) ? a : b;
		T upper = (a<b) ? b : a;
		return ((x>=lower)&&(x<=upper));    
	}

	template <class T>
	T sign(const T& x)
	{
		return (x<0) ? -1 : ((x==0) ? 0 : 1);            
	}

	template <class T>
	int get_first_index(std::vector<T> vec,T what)
	{
		int i;
		for (i=0;i<vec.size();i++) if (what==vec[i]) break;
		if (i==vec.size()) i = -1; // not found
		return i;
	}
	
	SERVICE_UTIL_DLL_API void splitLine(const std::string& line, std::vector<std::string>& vec, char delim = ',');

	SERVICE_UTIL_DLL_API pt::time_duration get_duration_from_string(std::string& str);

	SERVICE_UTIL_DLL_API dd::date getDateFromString(const std::wstring& input);
}

/* namespace derivative */

#endif /* _DERIVATIVE_QFUTIL_H_ */
