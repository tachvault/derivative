/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schl�gl
*/

#ifndef _DERIVATIVE_TSINSTRUMENTS_H_
#define _DERIVATIVE_TSINSTRUMENTS_H_

#include "TermStructure.hpp"

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
	class PRICINGENGINE_DLL_API TSEuropeanInstrument 
	{
	public:

		enum {TYPEID = CLASS_TSEUROPEANINSTRUMENT_TYPE};

		inline TSEuropeanInstrument(double xp,double xt,double xT) : p(xp),t(xt),T(xT)
		{ };
		virtual double payoff(const TermStructure& ts) = 0;
		inline double& price() 
		{ 
			return p; 
		};
		inline double price() const 
		{ 
			return p; 
		};
		inline double& maturity() 
		{ 
			return T; 
		};
		inline double maturity() const 
		{ 
			return T; 
		};
		inline double& today() 
		{ 
			return t; 
		};
		inline double today() const 
		{ 
			return t; 
		};
		/// Predicate for sorting instruments by maturity
		class PRICINGENGINE_DLL_API longer_maturity 
		{
		public:
			inline bool operator()(std::shared_ptr<TSEuropeanInstrument> first, \
				std::shared_ptr<TSEuropeanInstrument> second) 
			{ 
				return (first->T > second->T);
			}
		};
		/// Predicate for sorting instruments by maturity
		class PRICINGENGINE_DLL_API shorter_maturity 
		{
		public:
			inline bool operator()(std::shared_ptr<TSEuropeanInstrument> first,\
				std::shared_ptr<TSEuropeanInstrument> second) 
			{ 
				return (first->T < second->T);
			}
		};

	private:
		double p; ///< Current price.
		double t; ///< Time at which this price is valid.
		double T; ///< Maturity.
	};

	class PRICINGENGINE_DLL_API  Caplet : public TSEuropeanInstrument 
	{

	public:

		enum {TYPEID = CLASS_CAPLET_TYPE};

		inline Caplet(double xp,double xt,double xT,double xlvl,double xdelta,double xnotional = 1.0) 
			: TSEuropeanInstrument(xp,xt,xT),lvl(xlvl),notional(xnotional),delta(xdelta) 
		{ };
		virtual double payoff(const TermStructure& ts);

	private:
		double      lvl;
		double notional;
		double    delta;  ///< Length of accrual period.
	};

} /* namespace derivative */

#endif /* _DERIVATIVE_TSINSTRUMENTS_H_ */