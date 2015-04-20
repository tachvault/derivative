/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_MCMAPPING_H_
#define _DERIVATIVE_MCMAPPING_H_

#include <vector>
#include <list>
#include <memory>

#include "MSWarnings.hpp"
#include "MCGatherer.hpp"
#include "TermStructure.hpp"
#include "ClassType.hpp"
#include "MCPayOff.hpp"

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
	using blitz::firstDim;
	using blitz::secondDim;

	template <class target_price_process,class controlvariate_price_process,class cv_random_variable> class MCControlVariateMapping;

	/** Template that maps a set of random numbers to a stochastic process to a Monte Carlo payoff.

	The template parameter class price_process must implement a member function dimension(),
	which returns the number of assets (i.e. the dimension) of the price process.
	It must also implement the member function set_timeline(const Array<double,1>& timeline),
	which sets the timeline for which asset prices will be generated.
	Furthermore, it must provide operator()(Array<double,2>& underlying_values,const random_variable& x,const TermStructure& ts)
	and operator()(Array<double,2>& underlying_values,Array<double,1>& numeraire_values,const random_variable& x,const TermStructure& ts,int numeraire_index),
	which generate the values of the price process at the dates in the required timeline using
	a draw x of the random variable (the type of which is given by the template parameter random_variable),
	under the martingale measure associated with, respectively, deterministic bond prices or a chosen numeraire.
	These requirements are for example implemented by the class GeometricBrownianMotion.

	@see GeometricBrownianMotion
	*/
	template <class price_process,class mapped_random_variable>
	class MCMapping 
	{
	public:

		enum {TYPEID = CLASS_MCMAPPING_TYPE};

		/// Constructor.
		MCMapping(MCPayoff& xpayoff,price_process& xprocess,const TermStructure& xts,int xnumeraire_index = -1);

		/// Choose the numeraire asset for the equivalent martingale measure under which the simulation is carried out.
		bool set_numeraire_index(int xnumeraire_index);

		/// The function mapping a realisation of the (often multidimensional) random variable x to the discounted payoff.
		double mapping(mapped_random_variable x);

		/// The function mapping a realisation of the (often multidimensional) random variable x to multiple discounted payoffs.
		Array<double,1> mappingArray(mapped_random_variable x);

		template <class target_price_process,class controlvariate_price_process,class cv_random_variable> friend class MCControlVariateMapping;

		inline void print_index()
		{
			std::cout << payoff.index << std::endl;
		};

	private:
		/** Index of underlying asset to use as the numeraire. If less than zero, use deterministic discounting
		via the initial term structure. */
		int        numeraire_index;
		price_process&                   process;   ///< Stochastic process driving the underlying asset dynamics.
		const TermStructure&                  ts;   ///< Initial term structure.
		MCPayoff&                         payoff;   ///< Specification of the Monte Carlo payoff.
		Array<double,2>        underlying_values;
		Array<double,1> mapped_underlying_values;
		Array<double,1>         numeraire_values;
	};

	template <class price_process,class mapped_random_variable>
	MCMapping<price_process,mapped_random_variable>::MCMapping(MCPayoff& xpayoff,price_process& xprocess,const TermStructure& xts,int xnumeraire_index)
		: payoff(xpayoff),process(xprocess),ts(xts),
		underlying_values(xprocess.dimension(),xpayoff.timeline.extent(firstDim)),mapped_underlying_values(xpayoff.index.extent(secondDim)),
		numeraire_values(xpayoff.timeline.extent(firstDim))
	{
		if (!set_numeraire_index(xnumeraire_index)) 
		{
			throw std::logic_error("Unable to set numeraire index in MCMapping");
		}
		process.set_timeline(xpayoff.timeline);
	}

	template <class price_process,class mapped_random_variable>
	bool MCMapping<price_process,mapped_random_variable>::set_numeraire_index(int xnumeraire_index)
	{
		int i;
		numeraire_index = xnumeraire_index;
		if (numeraire_index == -1) 
		{ 
			// Deterministic discounting using initial term structure.
			for (i=0; i<numeraire_values.extent(firstDim); i++) numeraire_values(i) = 1.0/ts(payoff.timeline(i));
			return true;
		}
		else return process.set_numeraire(xnumeraire_index);
	}

	template <class price_process,class mapped_random_variable>
	double MCMapping<price_process,mapped_random_variable>::mapping(mapped_random_variable x)
	{
		/* Using the random draw x, generate the values of the price process at the dates in the required timeline,
		under the martingale measure associated with the chosen numeraire. */
		process(underlying_values,numeraire_values,x,ts,numeraire_index);

		// Map underlying values to the payoff.

		for (int i=0; i<mapped_underlying_values.extent(firstDim); i++) 
		{
			mapped_underlying_values(i) = underlying_values(payoff.index(0,i),payoff.index(1,i));
		}
		// Calculate the discounted payoff.
		return payoff(mapped_underlying_values,numeraire_values);
	}

	template <class price_process,class mapped_random_variable>
	Array<double,1> MCMapping<price_process,mapped_random_variable>::mappingArray(mapped_random_variable x)
	{
		/* Using the random draw x, generate the values of the price process at the dates in the required timeline,
		under the martingale measure associated with the chosen numeraire. */
		if (numeraire_index >= 0) 
		{
			process(underlying_values,numeraire_values,x,ts,numeraire_index);
		}
		else
		{
			process(underlying_values,x,ts);
		}

		// Map underlying values to the payoff.
		for (int i=0; i<mapped_underlying_values.extent(firstDim); i++)
		{
			mapped_underlying_values(i) = underlying_values(payoff.index(0,i),payoff.index(1,i));
		}

		// Calculate the discounted payoff.
		return payoff.payoffArray(mapped_underlying_values,numeraire_values);
	}

} /* namespace derivative */

#endif /* _DERIVATIVE_MCMAPPING_H_ */
