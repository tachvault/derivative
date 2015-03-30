/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_GEOMETRICBROWNIANMOTION_H_
#define _DERIVATIVE_GEOMETRICBROWNIANMOTION_H_

#include <stdexcept>
#include <vector>
#include "MSWarnings.hpp"
#include <blitz/array.h>
#include "BlackScholesAsset.hpp"
#include "BlackScholesAssetAdapter.hpp"

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

	class PRICINGENGINE_DLL_API GeometricBrownianMotion
	{
	public:

		enum {TYPEID = CLASS_GEOMETRICBROWNIANMOTION_TYPE};

		GeometricBrownianMotion(std::vector<std::shared_ptr<BlackScholesAssetAdapter> >& xunderlying);
		
		~GeometricBrownianMotion();
		
		/// Query the dimension of the process.
		int dimension() const;
		
		/// Query the number of factors driving the process.
		inline int factors() const 
		{
			return dW.extent(firstDim);
		};
		
		/// Set process timeline.
		bool set_timeline(const Array<double,1>& timeline);
		
		inline bool set_numeraire(int num) 
		{
			return (num<dimension());
		};
		
		/// Get process timeline.
		inline const Array<double,1>& get_timeline() const 
		{
			return *timeline_;
		};
		
		inline int number_of_steps() const 
		{
			return T->extent(firstDim)-1;
		};
		
		/// Generate a realisation of the process under the martingale measure associated with deterministic bond prices. underlying_values is an asset x (time points) Array.
		void operator()(Array<double,2>& underlying_values,const Array<double,2>& x,const TermStructure& ts);
		
		/// Generate a realisation of the process under the martingale measure associated with a given numeraire asset. underlying_values is an asset x (time points) Array.
		void operator()(Array<double,2>& underlying_values,Array<double,1>& numeraire_values,const Array<double,2>& x,const TermStructure& ts,int numeraire_index);

	private:
		Array<double,1>*                                T; ///< Process timeline.
		Array<double,1>*                        timeline_; ///< Timeline of asset price realisations reported by operator().
		std::vector<std::shared_ptr<BlackScholesAssetAdapter> >& underlying; ///< Vector of pointers to underlying assets.
		Array<double,1>                                dW; ///< Work space to hold Brownian motion increments.
		Array<double,1>                           vol_lvl; ///< Work space to hold volatility levels.
		Array<double,2>                      asset_values; ///< Work space to hold asset values.
		Array<int,1>*                        time_mapping; ///< Mapping of indices from internal to external timeline.
		void add_drift(double tstart,double tend,int numeraire_index);
	};
}

/* namespace derivative */

#endif  /* _DERIVATIVE_GEOMETRICBROWNIANMOTION_H_ */