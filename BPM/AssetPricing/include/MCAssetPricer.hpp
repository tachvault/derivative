/*
Copyright (C) Nathan Muruganantha 2015
*/

#ifndef _DERIVATIVE_MCASSETPRICER_H_
#define _DERIVATIVE_MCASSETPRICER_H_

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
	class MCPayoff;

	PRICINGENGINE_DLL_API double GeneralMC(std::vector<std::shared_ptr<BlackScholesAssetAdapter> >& assets, MCPayoff& payoff, \
		std::shared_ptr<TermStructure> term, double tenor, int numeraire_index, size_t sim, size_t N, double ci, size_t max_sim);

	PRICINGENGINE_DLL_API double AntitheticMC(std::vector<std::shared_ptr<BlackScholesAssetAdapter> >& assets, MCPayoff& payoff, \
		std::shared_ptr<TermStructure> term, double tenor, int numeraire_index, size_t sim, size_t N, double ci, size_t max_sim);

	PRICINGENGINE_DLL_API double QRMC(std::vector<std::shared_ptr<BlackScholesAssetAdapter> >& assets, MCPayoff& payoff, \
		std::shared_ptr<TermStructure> term, double tenor, int numeraire_index, size_t sim, size_t N, double ci, size_t max_sim);
}

/* namespace derivative */

#endif /* _DERIVATIVE_MCASSETPRICER_H_ */