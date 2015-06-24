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
		enum VanillaOptionType { CALL = 1, PUT = -1 };

		PRICINGENGINE_DLL_API  double ValueAmericanWithBinomial(const std::shared_ptr<BlackScholesAssetAdapter>& futures, double rate, \
			dd::date maturity, double strike, VanillaOptionType optType, int N = 1000);

		PRICINGENGINE_DLL_API double ValueEuropeanWithBinomial(const std::shared_ptr<BlackScholesAssetAdapter>& futures, double rate, \
			dd::date maturity, double strike, VanillaOptionType optType, int N = 1000);

		PRICINGENGINE_DLL_API double ValueEuropeanWithMC(const std::shared_ptr<BlackScholesAssetAdapter>& futures, \
			std::shared_ptr<TermStructure> term, dd::date maturity, double strike, VanillaOptionType optType, int sim = 500000, \
			size_t N = 2, double ci = 0.95);

		PRICINGENGINE_DLL_API double ValueAmericanWithMC(const std::shared_ptr<BlackScholesAssetAdapter>& futures, \
			std::shared_ptr<TermStructure> term, dd::date maturity, double strike, VanillaOptionType optType, \
			size_t sim = 100000, size_t N = 100, size_t train = 100, int degree = 2, double ci = 0.95);
	}

	namespace FuturesBarrierOptionPricer
	{
		enum VanillaOptionType { CALL = 1, PUT = -1 };
		enum BarrierOptionTypeEnum { KDI = 0, KDO = 2, KUI = 3, KUO = 4 };

		PRICINGENGINE_DLL_API double ValueWithMC(const std::shared_ptr<BlackScholesAssetAdapter>& futures, std::shared_ptr<TermStructure> term, \
			int BarrierType, dd::date maturity, double strike, double barrier, int optType, size_t sim = 50000, size_t N = 100, double ci = 0.95);
	}

	namespace FuturesAverageOptionPricer
	{
		enum VanillaOptionType { CALL = 1, PUT = -1 };
		enum AverageOptionTypeEnum { FIXED_STRIKE = 0, FLOATING_STRIKE = 1 };

		PRICINGENGINE_DLL_API double ValueWithMC(const std::shared_ptr<BlackScholesAssetAdapter>& futures, \
			std::shared_ptr<TermStructure> term, int AverageOptType, dd::date maturity, double strike, int optType, \
			size_t sim = 100000, size_t N = 100, double ci = 0.95);
	}
}

/* namespace derivative */

#endif /* _DERIVATIVE_FUTURESASSETPRICER_H_ */