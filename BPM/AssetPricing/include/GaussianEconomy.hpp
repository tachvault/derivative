/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_GAUSSIANECONOMY_H_
#define _DERIVATIVE_GAUSSIANECONOMY_H_

#include <stdexcept>
#include <vector>
#include <memory>

#include "MSWarnings.hpp"
#include <blitz/array.h>
#include "BlackScholesAsset.hpp"
#include "BlackScholesAssetAdapter.hpp"
#include "GaussianHJM.hpp"
#include "MultivariateNormal.hpp"

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
	using blitz::Array;
	using blitz::firstDim;
	using blitz::secondDim;
	using blitz::Range;
	using blitz::toEnd;
	using blitz::firstIndex;
	using blitz::secondIndex;

	/** Class representing assets quoted in a particular domestic currency. 
	These assets include a term structure of zero coupon bond prices and assets
	with deterministic continuous dividend yield (which may be zero).

	Each instance of the class GaussianEconomy is constructed from,
	1) equity assets in that economy (std::vector of shared pointers to 
	   instances of BlackScholesAsset)
	2) the Gauss/Markov HJM volatility for the interest rate dynamics in 
	   that economy (a shared pointer to an instance of DeterministicAssetVol 
	   (representing the Gauss/Markov HJM volatility. That is interest rate 
	   volatilities))
	3) its initial term structure of interest rates. shared pointer to an instance
	   of a class derived from TermStructure (representing the initial term 
	   structure of interest rates for this economy). */

	class PRICINGENGINE_DLL_API GaussianEconomy
	{
	public:

		enum {TYPEID = CLASS_GAUSSIANECONOMY_TYPE};

		GaussianEconomy(const std::vector<std::shared_ptr<BlackScholesAssetAdapter> >& xunderlying,
			std::shared_ptr<DeterministicAssetVol> xv,std::shared_ptr<TermStructure> xinitialTS);

		GaussianEconomy(const char* path);

	public:

		/// Vector of pointers to underlying assets.
		std::vector<std::shared_ptr<BlackScholesAssetAdapter> > underlying;

		/// Pointer to deterministic volatility function object (for Gaussian term structure dynamics).
		std::shared_ptr<DeterministicAssetVol> v;

		/// Vector of pointers to DeterministicAssetVol objects where all 
		/// components except one are zero - corresponds to volatilies of the state variables z.
		std::vector<std::shared_ptr<DeterministicAssetVol> > component_vol; 

		/// Pointer to term structure object containing the initial term structure of interest rates.
		std::shared_ptr<TermStructure> initialTS; 

		/// for closed-form solutions
		std::unique_ptr<GaussianHJM> hjm;
	};
}
/* namespace derivative */

#endif /* _DERIVATIVE_GAUSSIANECONOMY_H_ */

