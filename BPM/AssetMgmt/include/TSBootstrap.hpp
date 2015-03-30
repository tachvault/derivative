/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_TSBOOTSTRAP_H_
#define _DERIVATIVE_TSBOOTSTRAP_H_

#include "TermStructure.hpp"

#if defined _WIN32 || defined __CYGWIN__
#ifdef PRIMARYASSET_EXT_EXPORTS
#ifdef __GNUC__
#define PRIMARYASSET_EXT_API __attribute__ ((dllexport))
#else
#define PRIMARYASSET_EXT_API __declspec(dllexport)
#endif
#else
#ifdef __GNUC__
#define PRIMARYASSET_EXT_API __attribute__ ((dllimport))
#else
#define PRIMARYASSET_EXT_API __declspec(dllimport)
#endif
#endif
#define PRIMARYASSET_EXT_LOCAL
#else
#if __GNUC__ >= 4
#define PRIMARYASSET_EXT_API __attribute__ ((visibility ("default")))
#define PRIMARYASSET_EXT_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define PRIMARYASSET_EXT_API
#define PRIMARYASSET_EXT_LOCAL
#endif
#endif

namespace derivative
{
	/// Abstract base class for term structures which can be bootstrapped from coupon bond prices or, equivalently, swap rates.
	class PRIMARYASSET_EXT_API TSBootstrap : public TermStructure
	{
	private:
		class bootstrap_class 
		{
		private:
			TSBootstrap* ts_;
			const Array<double,1>& t_;
			const Array<double,1>& p_;
			const blitz::Range& slice;
			const blitz::Range& completed;
			size_t idx;
		public:
			inline bootstrap_class(const Array<double,1>& t,const Array<double,1>& p,const blitz::Range& s,const blitz::Range& c,size_t i,TSBootstrap* ts) 
				: ts_(ts),t_(t),p_(p),slice(s),completed(c),idx(i)
			{ };

			double operator()(double t);
		};

	public:

		enum {TYPEID = CLASS_TSBOOTSTRAP_TYPE};

		/// General constructor.
		inline TSBootstrap(const Array<double,1>& xT,             ///< Time line of maturities .
			const Array<double,1>& xB              ///< Time T(0) forward zero coupon bond prices for these maturities - thus the first bond price is always 1
			) : TermStructure(xT.copy(),xB.copy()) 
		{ };
		void bootstrap(std::vector<std::shared_ptr<DeterministicCashflow> > cashflows,double eps = 1E-12);

		virtual ~TSBootstrap();

	private:

		/// disallow the copy constructor and operator= functions
		DISALLOW_COPY_AND_ASSIGN(TSBootstrap);
	};

} /* namespace derivative */

#endif /* _DERIVATIVE_TERMSTRUCTURE_H_ */


