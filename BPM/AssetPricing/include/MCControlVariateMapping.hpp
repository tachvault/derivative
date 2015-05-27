/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_MCCONTROLVARIATEMAPPING_H_
#define _DERIVATIVE_MCCONTROLVARIATEMAPPING_H_

#include <vector>
#include <list>
#include <memory>

#include "MSWarnings.hpp"
#include "MCGatherer.hpp"
#include "TermStructure.hpp"
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
	using blitz::firstDim;
	using blitz::secondDim;

	template <class target_price_process,class controlvariate_price_process,class cv_random_variable>
	class MCControlVariateMapping 
	{
	public:

		enum {TYPEID = CLASS_MCCONTROLVARIATEMAPPING_TYPE};

		/// Constructor.
		inline MCControlVariateMapping(MCMapping<target_price_process,cv_random_variable>& xtarget,
			MCMapping<controlvariate_price_process,cv_random_variable>& xcontrolvariate,
			const Array<double,1>& xcontrolvariate_values)
			: target(xtarget),controlvariate(xcontrolvariate),controlvariate_values(xcontrolvariate_values) 
		{
			controlvariate_values_sum = sum(controlvariate_values);
		};

		/// The function mapping a realisation of the (often multidimensional) random variable x to the discounted payoff.
		double mapping(cv_random_variable x);

		/// The function mapping a realisation of the (often multidimensional) random variable x to multiple discounted payoffs.

		Array<double,1> mappingArray(cv_random_variable x);

		inline void print_index()
		{
			std::cout << controlvariate.payoff.index << std::endl;
		};

	private:
		MCMapping<target_price_process,cv_random_variable>&                         target;
		MCMapping<controlvariate_price_process,cv_random_variable>&         controlvariate;
		const Array<double,1>&                                       controlvariate_values;
		double                                                   controlvariate_values_sum;
	};

	template <class target_price_process,class controlvariate_price_process,class cv_random_variable>
	double MCControlVariateMapping<target_price_process,controlvariate_price_process,cv_random_variable>::mapping(cv_random_variable x)
	{
		return target.mapping(x) - controlvariate.mapping(x) + controlvariate_values_sum;
	}

	template <class target_price_process,class controlvariate_price_process,class cv_random_variable>
	Array<double,1> MCControlVariateMapping<target_price_process,controlvariate_price_process,cv_random_variable>::mappingArray(cv_random_variable x)
	{
		return target.mappingArray(x) - controlvariate.mappingArray(x) + controlvariate_values;
	}

} /* namespace derivative */

#endif /* _DERIVATIVE_MCCONTROLVARIATEMAPPING_H_ */
