/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schl�gl
*/

#ifndef _DERIVATIVE_MCGENERIC_H_
#define _DERIVATIVE_MCGENERIC_H_

#include "MSWarnings.hpp"
#include <boost/function.hpp>
#include <boost/math/distributions/normal.hpp>
#include "MCGatherer.hpp"

#if defined PRICINGENGINE_EXPORTS
#define PRICINGENGINE_DLL_API __declspec(dllexport)
#else
#define PRICINGENGINE_DLL_API __declspec(dllimport)
#endif

/*
Template for generic Monte Carlo simulation.

The template parameter class random_number_generator_type must implement a member function random(),
which returns a realisation of the random variable of the type given by the template parameter class argtype.

Alogorithm
1. Initialise the variables RunningSum = 0
RunningSumSquared = 0
i = 0
2. Generate the value of the stochastic process X at each date
T
k relevant to evaluate the payoff.
3. Based on the values generated in step 2, calculate the payoff.
4. Add the payoff to RunningSum and the square of the payoff
to RunningSumSquared.
5. Increment the counter i.
6. If i is less than the maximum number of iterations, go to
step 2.
7. Calculate the simulated mean by dividing RunningSum by
the total number of iterations.
8. Calculate the variance of the simulations by dividing RunningSumSquared by the total number of iterations and subtracting the square of the mean.
*/
namespace derivative
{	
	/* 
	
	argtype - type of random variable needed to evaluate the payoff 
	� typically, this 	would be double in the univariate case or Array<double,1> 
	in the multivariate case. 
	
	rettype - type of the variable representing the payoff. 
	
	random_number_generator_type - random number generator type, which must be
	a class with a member function argtype random() which returns a draw of the random
	variable of the required type � in the univariate, normally distributed case, one 
	could use ranlib::Normal<double>	from the Blitz++ library.
	*/
	template <class argtype,class rettype,class random_number_generator_type>
	class MCGeneric
	{
	public:
		inline MCGeneric(std::function<rettype (argtype)> func,random_number_generator_type& rng)
			: f(func),random_number_generator(rng)
		{ };
		inline MCGeneric(std::function<rettype (argtype)> func,random_number_generator_type& rng,std::function<argtype (argtype)> antifunc)
			: f(func),random_number_generator(rng),antithetic(antifunc)
		{ };
		inline void set_antithetic(std::function<rettype (argtype)> antifunc) 
		{
			antithetic = antifunc;
		};
		void simulate(MCGatherer<rettype>& mcgatherer,unsigned long number_of_simulations);
		double simulate(MCGatherer<rettype>& mcgatherer,unsigned long initial_number_of_simulations,double required_accuracy,double confidence_level = 0.95);

	private:
		boost::math::normal                                     N;
		std::function<rettype (argtype)>                      f;  ///< Functor mapping a draw of the random variable to the Monte Carlo payoff(s).
		std::function<argtype (argtype)>             antithetic;  ///< Functor mapping a draw of the random variable to their antithetic values.
		random_number_generator_type      random_number_generator;
	};

	template <class argtype,class rettype,class random_number_generator_type>
	void MCGeneric<argtype,rettype,random_number_generator_type>::simulate(MCGatherer<rettype>& mcgatherer,unsigned long number_of_simulations)
	{
		unsigned long i;
		if (!antithetic) 
		{
			for (i=0; i<number_of_simulations; i++) 
			{
				mcgatherer += f(random_number_generator.random());
			}
		}
		else 
		{
			for (i=0; i<number_of_simulations; i++)
			{
				argtype rnd = random_number_generator.random();
				rettype res = f(rnd);
				res = 0.5 * (res + f(antithetic(rnd)));
				mcgatherer += res;
			}
		}
	}

	template <class argtype,class rettype,class random_number_generator_type>
	double MCGeneric<argtype,rettype,random_number_generator_type>::simulate(MCGatherer<rettype>& mcgatherer,
		unsigned long initial_number_of_simulations,
		double required_accuracy,
		double confidence_level)
	{
		unsigned long n = initial_number_of_simulations;
		simulate(mcgatherer,initial_number_of_simulations);
		double d = boost::math::quantile(N,confidence_level); // was N.inverse(confidence_level);
		double current_accuracy = d * mcgatherer.max_stddev();
		while (required_accuracy<current_accuracy) 
		{
			double q = current_accuracy/required_accuracy;
			q *= q;
			unsigned long additional_simulations = n * q - n;
			if (!additional_simulations) break;
			simulate(mcgatherer,additional_simulations);
			n += additional_simulations;
			current_accuracy = d * mcgatherer.max_stddev();
		}
		return current_accuracy;
	}

} /* namespace derivative */

#endif /* _DERIVATIVE_MCGENERIC_H_ */