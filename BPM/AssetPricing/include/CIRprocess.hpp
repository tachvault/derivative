/*
Copyright (C) Nathan Muruganantha 2015
Initial version: Copyright 2008 by Erik Schlögl
*/

#ifndef _DERIVATIVE_CIRPROCESS_H_
#define _DERIVATIVE_CIRPROCESS_H_

#include <stdexcept>
#include <vector>
#include "MSWarnings.hpp"
#include <blitz/array.h>

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

	/** Multidimensional process of the type considered by Cox/Ingersoll/Ross (1985):
		\f[ dV_t = \kappa(\theta-V_t)dt + \sigma\sqrt{V_t}dW_t \f]
		*/
	class CIRprocess
	{
	public:
		inline CIRprocess(double xkappa,  ///< Speed of mean reversion
			double xtheta,  ///< Level of mean reversion
			double xVzero,  ///< Initial value of the process
			const Array<double, 1>& xsigma  ///< Volatility vector
			) : kappa(xkappa), theta(xtheta), Vzero(xVzero), sigma(xsigma)
		{ };
		/// Query the number of factors driving the process.
		inline int factors() const
		{
			return sigma.extent(firstDim);
		};
		inline double get_kappa() const
		{
			return kappa;
		};
		inline double get_theta() const
		{
			return theta;
		};
		inline double get_sigma_level() const
		{
			return std::sqrt(blitz::sum(sigma*sigma));
		};
		inline double get_initial() const
		{
			return Vzero;
		};

	private:
		double                 kappa;  ///< Speed of mean reversion
		double                 theta;  ///< Level of mean reversion
		double                 Vzero;  ///< Initial value of the process
		const Array<double, 1>& sigma;  ///< Volatility vector
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_CIRPROCESS_H_ */