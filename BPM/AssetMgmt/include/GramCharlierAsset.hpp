/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_GRAMCHARLIERASSET_H_
#define _DERIVATIVE_GRAMCHARLIERASSET_H_

#include <boost/math/distributions/normal.hpp>
#include "GramCharlier.hpp"
#include "IAssetValue.hpp"

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
	/// Class for an asset whose price risk neutral distribution is given a Gram/Charlier expansion.
	class DERIVATIVEASSET_DLL_API GramCharlierAsset
	{
	public:

		enum {TYPEID = CLASS_GRAMCHARLIERASSET_TYPE};

		/// Constructor.
		inline GramCharlierAsset(GramCharlier& xgc,          ///< Gram/Charlier expanded density for the standardised risk-neutral distribution.
			double xsgm,                ///< Volatility level.
			double ini,                 ///< Initial ("time zero") value.
			double xT                   ///< Maturity for which the risk-neutral distribution is valid.
			) 
			: gc(xgc),sgm(xsgm),xzero(ini),T(xT),prices_(3)
		{ };

		/// Copy constructor.
		GramCharlierAsset(const GramCharlierAsset& xasset);

		/// define the assignment operator
		GramCharlierAsset& operator=(const GramCharlierAsset& xasset);
		
		inline double initial_value() const 
		{
			return xzero;
		};  ///< Query the initial ("time zero") value.
		
		inline void initial_value(double ini)
		{
			xzero = ini;
		}; ///< Set the initial ("time zero") value.
		inline double maturity() const 
		{
			return T;
		};
		/// Price a European call option.
		double call(double K,double domestic_discount,double foreign_discount = 1.0) const;
		
		/// Best fit calibration to a given set of Black/Scholes implied volatilities.
		double calibrate(std::shared_ptr<const Array<double,1> > xstrikes, std::shared_ptr<const Array<double,1> > xvols, \
			             double domestic_discount,double foreign_discount,int highest_moment);
		
		inline double standard_deviation() const
		{
			return sgm;
		};
		
		inline double skewness() const
		{
			return gc.skewness();
		};
		
		inline double excess_kurtosis() const 
		{
			return gc.excess_kurtosis();
		};

	private:
		GramCharlier&  gc;        ///< Gram/Charlier expanded density for the standardised risk-neutral distribution.
		double sgm;       ///< Volatility level.
		double xzero;     ///< Initial ("time zero") value.
		double T;         ///< Maturity for which the risk-neutral distribution is valid.
		boost::math::normal N;
		
		double genericBlackScholes(double S1,double S2,double vol,int sign) const;
		
		/// Objective function for best fit calibration to a given set of Black/Scholes implied volatilities.
		double calibration_objective_function(double xsgm);
		
		// Variables used during calibration.
		std::shared_ptr<const Array<double,1> > strikes_;
		Array<double,1>        prices_;
		double                 domestic_;
		double                 foreign_;
	};

} /* namespace derivative */

#endif /* _DERIVATIVE_GRAMCHARLIERASSET_H_ */
