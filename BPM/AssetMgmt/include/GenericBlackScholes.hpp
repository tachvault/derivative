/* 
   Copyright (C) Nathan Muruganantha 2013 - 2014
   Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_GENERICBLACKSCHOLES_H_
#define _DERIVATIVE_GENERICBLACKSCHOLES_H_

#include <boost/math/distributions/normal.hpp>
#include "ClassType.hpp"

#if defined _WIN32 || defined __CYGWIN__
#ifdef DERIVATIVEASSET_EXPORTS
#ifdef __GNUC__
#define DERIVATIVEASSET_DLL_API __attribute__ ((dllexport))
#else
#define DERIVATIVEASSET_DLL_API __declspec(dllexport)
#endif
#else
#ifdef __GNUC__
#define DERIVATIVEASSET_DLL_API __attribute__ ((dllimport))
#else
#define DERIVATIVEASSET_DLL_API __declspec(dllimport)
#endif
#endif
#define DERIVATIVEASSET_DLL_LOCAL
#else
#if __GNUC__ >= 4
#define DERIVATIVEASSET_DLL_API __attribute__ ((visibility ("default")))
#define DERIVATIVEASSET_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define DERIVATIVEASSET_DLL_API
#define DERIVATIVEASSET_DLL_LOCAL
#endif
#endif

namespace derivative 
{
	class DERIVATIVEASSET_DLL_API GenericBlackScholes 
	{
	public:

		enum {TYPEID = CLASS_GENERICBLACKSCHOLES_TYPE};

		double operator()(double S1,double S2,double vol,int sign) const;

	private:
		boost::math::normal N;
	};

} /* namespace derivative */

#endif /* _DERIVATIVE_GENERICBLACKSCHOLES_H_ */
