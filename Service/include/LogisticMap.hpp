/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_LOGISTICMAP_H_
#define _DERIVATIVE_LOGISTICMAP_H_

#include <algorithm>
#include "ClassType.hpp"

#undef min
#undef max

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
	class LogisticMap 
	{
	public:

		enum {TYPEID = CLASS_LOGISTICMAP_TYPE};

		inline LogisticMap(double xa = 0.0,double xb = 1.0) : a(xa),b(xb)
		{ };
		inline double min() const 
		{
			return a; 
		}; 
		inline double max() const
		{ 
			return b; 
		}; 
		inline void set_min(double xa) 
		{ 
			a = xa; 
		}; 
		inline void set_max(double xb)
		{ 
			b = xb; 
		}; 
		inline void operator()(double& x) const;
		inline void inverse(double& x) const;
		inline void operator*=(double x);

	private:
		double a,b;
	};

	inline void LogisticMap::operator()(double& x) const
	{
		x = a + (b-a)/(1.0+std::exp(-x));
	}

	inline void LogisticMap::inverse(double& x) const
	{
		x = -std::log((b-a)/(x-a)-1.0);
	}

	inline void LogisticMap::operator*=(double x)
	{
		a *= x;
		b *= x;
		if (x<0.0) std::swap(a,b);      
	}
}

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

/* namespace derivative */

#endif /* _DERIVATIVE_LOGISTICMAP_H_ */

