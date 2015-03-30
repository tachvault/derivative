/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_MBINARY_H_
#define _DERIVATIVE_MBINARY_H_

#include <vector>
#include "MultivariateNormal.hpp"
#include "BlackScholesAsset.hpp"
#include "TermStructure.hpp"
#include "MCPayoff.hpp"
#include "MCMapping.hpp"
#include "GaussianEconomy.hpp"
#include "GaussMarkovWorld.hpp"
#include "ClassType.hpp"

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
	using blitz::firstIndex;
	using blitz::secondIndex;
	using blitz::thirdIndex;

	class PRICINGENGINE_DLL_API MBinaryPayoff : public MCPayoff 
	{
	public:

		enum {TYPEID = CLASS_MBINARYPAYOFF_TYPE};

		std::vector<std::shared_ptr<BlackScholesAssetAdapter> >  underlying;  ///< Vector of pointers to underlying assets.
		const TermStructure&                           ts;  ///< (Deterministic) term structure of interest rates.
		Array<double,1>                             alpha;  ///< Payoff powers.
		Array<int,2>                                    S;  ///< Exercise indicators.
		Array<double,2>                                 A;  ///< Exercise condition matrix.  
		Array<double,1>                                 a;  ///< Strike vector.
		double                                   notional;
		inline MBinaryPayoff(const TermStructure&     xts,  ///< (Deterministic) term structure of interest rates.
			int                ntimeline,  ///< Number of points on the time line collecting all event dates.
			int         payoff_dimension,  ///< Number of (asset,time) combinations.
			int       exercise_dimension,  ///< Number of indicator functions.
			double xnotional = 1.0
			) : MCPayoff(ntimeline-1,payoff_dimension),alpha(payoff_dimension),
			ts(xts),S(exercise_dimension,exercise_dimension),A(exercise_dimension,payoff_dimension),
			a(exercise_dimension),notional(xnotional) { };
		/// Calculate discounted payoff. 
		virtual double operator()(const Array<double,1>& underlying_values,const Array<double,1>& numeraire_values);
	};

	class PRICINGENGINE_DLL_API MBinary 
	{
	public:   

		enum {TYPEID = CLASS_MBINARY_TYPE};

		MBinary(std::vector<std::shared_ptr<BlackScholesAssetAdapter> >& xunderlying,  ///< Vector of pointers to underlying assets.
			const TermStructure&                           xts,  ///< (Deterministic) term structure of interest rates.
			const Array<double,1>&                   xtimeline,  ///< Time line collecting all event dates.
			const Array<int,2>&                         xindex,  ///< A 2 x N matrix of indices, where each column represents the 
			///< indices of an (asset,time) combination affecting the payoff.
			const Array<double,1>&                      xalpha,  ///< Payoff powers.
			const Array<int,2>&                             xS,  ///< Exercise indicators.
			const Array<double,2>&                          xA,  ///< Exercise condition matrix.  
			const Array<double,1>&                          xa,  ///< Strike vector.
			double xnotional = 1.0
			);
		MBinary(MBinaryPayoff& xpayoff);
		MBinary(const GaussMarkovWorld& world,MBinaryPayoff& xpayoff);
		MBinary(const MBinary& original,const Array<double,1>& T,const Array<double,2>& history);
		~MBinary();
		std::shared_ptr<MBinaryPayoff> get_payoff() const;
		double price(unsigned long n = 1000000);
		double price(double t,double single_underlying,unsigned long n = 1000000);
		double price(const Array<double,1>& T,const Array<double,2>& history,unsigned long n = 1000000);
		// "Greeks"
		/// Delta with respect to the i-th asset.
		double delta(int i,unsigned long n = 1000000);
		// Accessors
		inline const Array<double,2>& covariance_matrix() const { return SCS; };
		inline const Array<double,1>& eigenvalues() const { return mvrnd->eigenvalues(); };

	private:
		// Inputs
		int                              payoff_dimension;  ///< Number of (asset,time) combinations.
		int                            exercise_dimension;
		int                              number_of_assets;
		bool                                    worthless;
		std::vector<std::shared_ptr<BlackScholesAssetAdapter> >& underlying;  ///< Vector of pointers to underlying assets.
		const TermStructure&                           ts;  ///< (Deterministic) term structure of interest rates.
		Array<double,1>                          timeline;  ///< Time line collecting all event dates.
		Array<int,2>                                index;  ///< A 2 x N matrix of indices, where each column represents the 
		///< indices of an (asset,time) combination affecting the payoff.
		Array<double,1>                             alpha;  ///< Payoff powers.
		Array<int,2>                                    S;  ///< Exercise indicators.
		Array<double,2>                                 A;  ///< Exercise condition matrix.  
		Array<double,1>                                 a;  ///< Strike vector.
		double                                   notional;
		// Values pre-calculated in the constructor
		Array<double,1>  mu;  ///< Vector of drifts.
		Array<double,2> sgm;  ///< N x N covariance matrix of logarithmic asset prices at event dates.
		double         beta;
		Array<double,1>  Sd;
		Array<double,2> SCS;
		Array<double,1>   x;  ///< Initial prices of the underlying assets to match payoff vector - thus repeat entries are possible.
		std::unique_ptr<MultivariateNormal> mvrnd;
		// Saved values to reduce calculations
		double P_t,I_t;
		unsigned long current_n;
		void initialise();
		void initialise(const Array<double,1>& T,const Array<double,2>& history);
		double dPdX(int i);
		double dIdX(int i);
	};

} /* namespace derivative */

#endif /* _DERIVATIVE_MBINARY_H_ */

