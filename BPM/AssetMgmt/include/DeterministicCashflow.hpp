/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_DETERMINISTICCASHFLOW_H_
#define _DERIVATIVE_DETERMINISTICCASHFLOW_H_

#include "TermStructure.hpp"
#include "Global.hpp"

#if defined _WIN32 || defined __CYGWIN__
#ifdef ASSET_PROPERTIES_EXPORTS
#ifdef __GNUC__
#define ASSET_PROPERTIES_API __attribute__ ((dllexport))
#else
#define ASSET_PROPERTIES_API __declspec(dllexport)
#endif
#else
#ifdef __GNUC__
#define ASSET_PROPERTIES_API __attribute__ ((dllimport))
#else
#define ASSET_PROPERTIES_API __declspec(dllimport)
#endif
#endif
#define ASSET_PROPERTIES_LOCAL
#else
#if __GNUC__ >= 4
#define ASSET_PROPERTIES_API __attribute__ ((visibility ("default")))
#define ASSET_PROPERTIES_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define ASSET_PROPERTIES_API
#define ASSET_PROPERTIES_LOCAL
#endif
#endif

namespace derivative
{
	class ASSET_PROPERTIES_API DeterministicCashflow
	{
	public:

		enum { TYPEID = CLASS_DETERMINISTICCASHFLOW_TYPE};

		inline DeterministicCashflow()
		{ };

		inline DeterministicCashflow(const DeterministicCashflow& cf)
			: timeline_(cf.timeline_.copy()),payments(cf.payments.copy()),value(cf.value)
		{
			/* std::cout << timeline_ << std::endl; */
		};

		~DeterministicCashflow()
		{}

		inline DeterministicCashflow(const Array<double,1>& t,const Array<double,1>& p,double val)
			: timeline_(t.copy()),payments(p.copy()),value(val)
		{ };

		DeterministicCashflow& operator=(const DeterministicCashflow& cf);

		inline int num_cashflow() const
		{
			return timeline_.extent(firstDim);
		}

		inline const Array<double,1>& timeline() const
		{
			return timeline_;
		};
		inline const Array<double,1>& cashflow() const 
		{
			return payments;
		};
		inline double market_value() const
		{
			return value;
		};
		inline void set_market_value(double v) 
		{
			value = v;
		};

		/// Calculate net present value of present and future (not past) cashflows.
		/// The parameter timeline is the term structure timeline
		double NPV(const Array<double,1>& timeline) const;

		/// Predicate for sorting cashflows by length
		class longer_cashflow 
		{
		public:
			inline bool operator()(const DeterministicCashflow& first,const DeterministicCashflow& second)
			{
				return (first.timeline_(first.timeline_.extent(firstDim)-1)>second.timeline_(second.timeline_.extent(firstDim)-1));
			}
		};
		/// Predicate for sorting cashflows by length
		class shorter_cashflow 
		{
		public:
			inline bool operator()(const DeterministicCashflow& first,const DeterministicCashflow& second)
			{
				return (first.timeline_(first.timeline_.extent(firstDim)-1)<second.timeline_(second.timeline_.extent(firstDim)-1));
			}
		};

	private:

		Array<double,1> timeline_;
		Array<double,1> payments;
		double value;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_DETERMINISTICCASHFLOW_H_ */