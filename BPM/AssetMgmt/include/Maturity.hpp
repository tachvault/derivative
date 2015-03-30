/// Copyright (C) Nathan Muruganantha 2013 - 2014

#ifndef _DERIVATIVE_MATURITY_H_
#define _DERIVATIVE_MATURITY_H_

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
	/// Model Maturity convention used in the 
	/// bond pricing
	class FIN_UTIL_DLL_API Maturity
	{
	public:

		enum {TYPEID = CLASS_MATURITY_TYPE};

		enum MaturityType 
		{
			O = 1, W1 = 2, W2 = 3, W3 = 4, W4 = 5, M1 = 6, M2 = 7, M3 = 8, M4 = 9, M5 = 10, M6 = 11, M7 = 12, \
			M8 = 13, M9 = 14, M10 = 15, M11 = 16, Y1 = 17, Y2 = 18, Y3 = 19, Y4 = 20, Y5 = 21, Y6 = 22, Y7 = 23, \
			Y8 = 24, Y9 = 25, Y10 = 26, Y20 = 27, Y30 = 28
		};

		static dd::date getNextDate(const dd::date& start, const MaturityType& maturity);

		static Maturity::MaturityType getMaturity(int days);
	};		
}

/* namespace derivative */
#pragma warning(pop)
#endif /* _DERIVATIVE_MATURITY_H_ */