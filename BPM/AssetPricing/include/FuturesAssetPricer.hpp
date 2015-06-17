/*
Copyright (C) Nathan Muruganantha 2015
*/

#ifndef _DERIVATIVE_FUTURESASSETPRICER_H_
#define _DERIVATIVE_FUTURESASSETPRICER_H_

#include <memory>
#include "Global.hpp"

#if defined _WIN32 || defined __CYGWIN__
#ifdef PRICINGENGINE_EXPORTS
#ifdef __GNUC__
#define PRICINGENGINE_DLL_API __attribute__ ((dllexport))
#else
#define PRICINGENGINE_DLL_API __declspec(dllexport)
#endif
#else
#ifdef __GNUC__
#define PRICINGENGINE_DLL_API __attribute__ ((dllimport))
#else
#define PRICINGENGINE_DLL_API __declspec(dllimport)
#endif
#endif
#define PRICINGENGINE_LOCAL
#else
#if __GNUC__ >= 4
#define PRICINGENGINE_DLL_API __attribute__ ((visibility ("default")))
#define PRICINGENGINE_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define PRICINGENGINE_DLL_API
#define PRICINGENGINE_LOCAL
#endif
#endif

namespace derivative
{
	class BlackScholesAssetAdapter;
	class TermStructure;

	namespace FuturesVanillaOptionPricer
	{
		PRICINGENGINE_DLL_API  double ValueAmericanWithBinomial(const std::shared_ptr<BlackScholesAssetAdapter>& futures, double rate, \
			dd::date maturity, double strike, int optType, int N = 1000);

		PRICINGENGINE_DLL_API double ValueEuropeanWithBinomial(const std::shared_ptr<BlackScholesAssetAdapter>& futures, double rate, \
			dd::date maturity, double strike, int optType, int N);		
	}
}

/* namespace derivative */

#endif /* _DERIVATIVE_FUTURESASSETPRICER_H_ */