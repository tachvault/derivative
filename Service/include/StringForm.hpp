/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_STRINGFORM_H_
#define _DERIVATIVE_STRINGFORM_H_

#include <iostream>
#include <sstream>
#include <cmath>

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
	class Bound_StringForm;     // StringForm plus value

	class SERVICE_UTIL_DLL_API StringForm 
	{
	public:
		enum str_scheme { LEFT, RIGHT, CENTER };

	public:
		explicit StringForm(int p = 6) : prc(p) 
		{ 
			fmt = std::ios_base::fmtflags(0); 
			wdt = 0; pl = false; 
			tz = false; 
			fl = ' '; 
		};
		/// Make a Bound_StringForm for (*this) and d
		Bound_StringForm operator()(double d) const;
		Bound_StringForm operator()(const char* c) const;
		StringForm& scientific() 
		{ 
			fmt = std::ios_base::scientific; 
			return *this;
		};
		StringForm& fixed() 
		{ 
			fmt = std::ios_base::fixed; 
			return *this;
		};
		StringForm& general() 
		{ 
			fmt = std::ios_base::fmtflags(0); 
			return *this; 
		};
		StringForm& stringscheme(str_scheme s)
		{ 
			str_sch = s; 
			return *this; 
		};
		StringForm& precision(int p) 
		{ 
			prc = p; 
			return *this;
		};
		StringForm& width(int w) 
		{ 
			wdt = w; 
			return *this; 
		};
		StringForm& fill(char c)
		{ 
			fl = c;
			return *this;
		};

	private:
		friend class Bound_StringForm;
		int prc;      ///< precision
		int wdt;      ///< width, 0 means as wide as necessary
		std::ios_base::fmtflags fmt;      ///< general, scientific, or fixed
		bool pl;      ///< explicit plus
		bool tz;      ///< trailing zeros
		char fl;      ///< fill character
		str_scheme str_sch;
	};

	class SERVICE_UTIL_DLL_API Bound_StringForm 
	{
	public:

		Bound_StringForm(const StringForm& ff,double v);
		Bound_StringForm(const StringForm& ff,const char* v);

	public:

		std::ostringstream s;
	};

	inline Bound_StringForm StringForm::operator()(double d) const
	{ 
		return Bound_StringForm(*this,d); 
	}

	inline Bound_StringForm StringForm::operator()(const char* c) const
	{ 
		return Bound_StringForm(*this,c); 
	}

	SERVICE_UTIL_DLL_API std::ostream& operator<<(std::ostream& os,const Bound_StringForm& bf);
}

/* namespace derivative */

#endif /* _DERIVATIVE_STRINGFORM_H_ */