/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_BLACKSCHOLESASSET_H_
#define _DERIVATIVE_BLACKSCHOLESASSET_H_

#include <boost/math/distributions/normal.hpp>

#include "GenericBlackScholes.hpp"
#include "DeterministicVol.hpp"
#include "FlatTermStructure.hpp"

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
	/// Class for an asset whose price process follows a geometric Brownian motion.
	/// Objects of type BlackScholesAsset are copyable, assignable and movable.
	class DERIVATIVEASSET_DLL_API BlackScholesAsset 
	{
	public:
		 enum {TYPEID = CLASS_BLACKSCHOLESASSET_TYPE};

		/// Constructor.
		BlackScholesAsset(std::shared_ptr<DeterministicAssetVol> xv,  ///< Volatility function.
			double ini,                 ///< Initial ("time zero") value.
			double div = 0.0            ///< Dividend yield.
			);		

		/// Constructor.
		BlackScholesAsset(std::shared_ptr<DeterministicAssetVol> xv,  ///< Volatility function.
			double ini,                 ///< Initial ("time zero") value.
			std::shared_ptr<TermStructure> div            ///< Dividend yield.
			);		

		/// Copy constructor.
		BlackScholesAsset(const BlackScholesAsset& xasset);

		/// define the assignment operator
		BlackScholesAsset& operator=(const BlackScholesAsset& xasset);

		BlackScholesAsset(const char* path);

		inline double initial_value() const 
		{
			return xzero; 
		};  ///< Query the initial ("time zero") value.

		inline void initial_value(double ini) 
		{ 
			xzero = ini;
		}; ///< Set the initial ("time zero") value.

		/// (Forward) dividend yield between times u and v.
		inline double dividend_yield(double u,double v) const 
		{ 
			return dividend->forward_yield(u,v);
		};  

		/// (Forward) dividend discount factor between times u and v.
		inline double dividend_discount(double u,double v) const 
		{ 
			return (*dividend)(v)/(*dividend)(u); 
		};  

		inline void dividend_yield(double ini) 
		{ 
			dividend.reset(new FlatTermStructure(ini,0.0,100.0)); 
		}; 

		/// Price a European call or put option (call is default).
		/// In the case of Black's formula, the value r corresesponds to the period
		/// upto delivery date
		double option(double mat,double K,double r,int sign = 1,double today = 0.0) const;
		
		/// Delta of a European call or put option (call is default).
		double delta(double mat,double K,double r,int sign = 1) const;
		
		/// Gamma of a European call or put option (call is default).
		double gamma(double mat,double K,double r,int sign = 1) const;
		
		/// Vega of a European call or put option (call is default).
		double vega(double mat, double K, double r, int sign = 1) const;

		/// Theta of a European call or put option (call is default).
		double theta(double mat, double K, double r, int sign = 1) const;

		/// Option to exchange one asset for another.
		double Margrabe(const BlackScholesAsset& S,double mat,double K, int sign = 1) const;
		
		double DoleansExp(double t,double T,const Array<double,1>& dW) const;
		
		inline double volatility(double t,double ttm) const 
		{ 
			return std::sqrt(v->volproduct(t,ttm,*v)/ttm); 
		};
		
		/// Change the volatility function.
		inline void set_volatility(std::shared_ptr<DeterministicAssetVol> xv) 
		{ 
			v = xv; 
		};
		/// Covariance of logarithmic asset prices.
		inline double covariance(double t,double ttm,const BlackScholesAsset& S) const 
		{ 
			return v->volproduct(t,ttm,*(S.v)); 
		};
		/// Access the volatility function.
		inline const DeterministicAssetVol& volatility_function() const 
		{ 
			return *v; 
		};
		
		/// Calculate the implied volatility for a given price.
		double implied_volatility(double price,double mat,double K,double r,int sign = 1) const;

	private:

		/// Volatility function of the 
		std::shared_ptr<DeterministicAssetVol> v;                    
		double                 xzero;                ///< Initial ("time zero") value.
		std::shared_ptr<TermStructure>  dividend;  ///< Term structure of dividend yields.
		boost::math::normal N;
		GenericBlackScholes genericBlackScholes;
	};

} /* namespace derivative */

#endif /* _DERIVATIVE_BLACKSCHOLESASSET_H_ */
