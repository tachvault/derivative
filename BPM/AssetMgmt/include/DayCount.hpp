/// Copyright (C) Nathan Muruganantha 2013 - 2014

#ifndef _DERIVATIVE_DAYCOUNT_H_
#define _DERIVATIVE_DAYCOUNT_H_

#if defined _WIN32
#pragma once
#pragma warning (push)
#pragma warning (disable : 4251)
#endif

#include <memory>
#include <string>
#include <iostream>
#include "ClassType.hpp"
#include "IObject.hpp"
#include "Name.hpp"

#if defined _WIN32 || defined __CYGWIN__
  #ifdef FINUTILITY_EXPORTS
    #ifdef __GNUC__
      #define FIN_UTIL_DLL_API __attribute__ ((dllexport))
    #else
      #define FIN_UTIL_DLL_API __declspec(dllexport)
    #endif
  #else
    #ifdef __GNUC__
      #define FIN_UTIL_DLL_API __attribute__ ((dllimport))
    #else
      #define FIN_UTIL_DLL_API __declspec(dllimport)
    #endif
  #endif
  #define FIN_UTIL_DLL_LOCAL
#else
  #if __GNUC__ >= 4
    #define FIN_UTIL_DLL_API __attribute__ ((visibility ("default")))
    #define FIN_UTIL_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define FIN_UTIL_DLL_API
    #define FIN_UTIL_DLL_LOCAL
  #endif
#endif
 

namespace derivative
{
	/// Model DayCount convention used in the 
	/// bond pricing
	class FIN_UTIL_DLL_API DayCount
	{
	public:

		enum {TYPEID = CLASS_DAYCOUNT_TYPE};

		enum DayCountType
		{
			_30_360	= 0,
			actual_actual = 1,
			actual_360 = 2,
			actual_365 = 3,
			_30_360_European = 4
		};

		static double getPeriod(const dd::date& start, const dd::date& end, const DayCountType& countType);
	};		
}

/* namespace derivative */
#pragma warning(pop)
#endif /* _DERIVATIVE_DAYCOUNT_H_ */