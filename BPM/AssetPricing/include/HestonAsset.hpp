/*
Copyright (C) Nathan Muruganantha 2015
Initial version: Copyright 2008, 2015 by Erik Schlögl
*/

#ifndef _DERIVATIVE_HESTONASSET_H_
#define _DERIVATIVE_HESTONASSET_H_

#include <memory>

#include "CIRprocess.hpp"
#include "BlackScholesAsset.hpp"
#include "GaussianQuadrature.hpp"
#include "ConstVol.hpp"

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
	using blitz::TinyVector;

	/// Class for an asset whose price process follows a stochastic volatility process of the Heston (1993) type.
	class HestonAsset
	{
	public:
		/// Constructor.
		HestonAsset(CIRprocess& xvol_process,     ///< Volatility process.
			double ini,                   ///< Initial ("time zero") value.
			double xrho,                  ///< Correlation between Brownian motion driving the volatility process and the Brownian motion driving the asset price process. 
			double xlambda,               ///< Market price of volatility risk parameter.
			int xn = 25                   ///< Gauss/Laguerre quadrature refinement
			);
		inline double initial_value() const 
		{
			return xzero; 
		};  ///< Query the initial ("time zero") value.
		inline void initial_value(double ini) 
		{ 
			xzero = ini; 
		}; ///< Set the initial ("time zero") value.
		inline double get_rho() const 
		{
			return rho; 
		};          ///< Query the correlation between Brownian motion driving the volatility process and the Brownian motion driving the asset price process. 
		inline double get_initial_volatility() const 
		{ 
			return std::sqrt(vol_process.get_initial()); 
		};
		/// Price a European call or put option (call is default).
		double option(double mat, double K, double r, int sign = 1) const;

	private:
		CIRprocess&      vol_process;     ///< Volatility process.
		double                 xzero;     ///< Initial ("time zero") value.
		double                   rho;     ///< Correlation between Brownian motion driving the volatility process and the Brownian motion driving the asset price process. 
		double                lambda;     ///< Market price of volatility risk parameter.
		double kappa;
		double theta;
		double sigma;
		GaussLaguerre gausslaguerre;
		int n;
		double P_Gatheral(double phi, int j, double mat, double x, double r) const;
	};

	/// Class to convert HestonAsset into BlackScholesAsset. Makes the conversion explicit, but avoids the need for BlackScholesAsset to "know" about HestonAsset.
	class HestonAsset_as_BlackScholesAsset : public BlackScholesAsset
	{
	private:
		Array<double, 1> vol_lvl;
		std::shared_ptr<DeterministicAssetVol> const_vol;
	public:
		HestonAsset_as_BlackScholesAsset(const HestonAsset& heston_asset);
		virtual ~HestonAsset_as_BlackScholesAsset();
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_HESTONASSET_H_ */