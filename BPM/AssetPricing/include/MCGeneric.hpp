/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_MCGENERIC_H_
#define _DERIVATIVE_MCGENERIC_H_

#include <future>
#include <memory>
#include <math.h>

#include <boost/function.hpp>
#include <boost/math/distributions/normal.hpp>
#include "MSWarnings.hpp"
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
	— typically, this 	would be double in the univariate case or Array<double,1>
	in the multivariate case.

	rettype - type of the variable representing the payoff.

	random_number_generator_type - random number generator type, which must be
	a class with a member function argtype random() which returns a draw of the random
	variable of the required type — in the univariate, normally distributed case, one
	could use ranlib::Normal<double>	from the Blitz++ library.
	*/
	template <class argtype, class rettype, class random_number_generator_type>
	class MCGeneric
	{
	public:
		inline MCGeneric(std::function<rettype(argtype)> func, random_number_generator_type& rng, size_t max_sim = 100000)
			: f(func), random_number_generator(rng), max_simulation(max_sim)
		{ };
		inline MCGeneric(std::function<rettype(argtype)> func, random_number_generator_type& rng, std::function<argtype(argtype)> antifunc, size_t max_sim = 100000)
			: f(func), random_number_generator(rng), antithetic(antifunc), max_simulation(max_sim)
		{ };
		inline void set_antithetic(std::function<rettype(argtype)> antifunc)
		{
			antithetic = antifunc;
		};

		/// run within an async call. Reply on the return value optimization..
		std::shared_ptr<MCGatherer<rettype> > simulate(size_t dim, unsigned long number_of_simulations);

		void simulate(MCGatherer<rettype>& mcgatherer, unsigned long number_of_simulations);

		double simulate(MCGatherer<rettype>& mcgatherer, unsigned long initial_number_of_simulations, double required_accuracy, double confidence_level = 0.95);

	private:
		boost::math::normal                                     N;
		std::function<rettype(argtype)>                      f;  ///< Functor mapping a draw of the random variable to the Monte Carlo payoff(s).
		std::function<argtype(argtype)>             antithetic;  ///< Functor mapping a draw of the random variable to their antithetic values.
		random_number_generator_type      random_number_generator;
		
		/// represents maximum simulations per one async call
		std::size_t max_simulation;
	};

	template <class argtype, class rettype, class random_number_generator_type>
	std::shared_ptr<MCGatherer<rettype> > MCGeneric<argtype, rettype, random_number_generator_type>::simulate(size_t dim, unsigned long number_of_simulations)
	{
		std::shared_ptr<MCGatherer<rettype> > gatherer = std::make_shared<MCGatherer<rettype> >(dim);
		random_number_generator_type random_number_generator_copy = random_number_generator;
		if (!antithetic)
		{
			for (long long i = 0; i < number_of_simulations; i++)
			{
				*gatherer += f(random_number_generator_copy.random());
			}
		}
		else
		{
			for (long long i = 0; i < number_of_simulations; i++)
			{
				argtype rnd = random_number_generator_copy.random();
				rettype res = f(rnd);
				res = 0.5 * (res + f(antithetic(rnd)));
				*gatherer += res;
			}
		}
		return gatherer;
	}

	template <class argtype, class rettype, class random_number_generator_type>
	void MCGeneric<argtype, rettype, random_number_generator_type>::simulate(MCGatherer<rettype>& mcgatherer, unsigned long number_of_simulations)
	{
		std::vector<std::future<std::shared_ptr<MCGatherer<rettype> > > >futures;
		/// get the number of asyn call required
		int func_calls = std::ceil((double)(number_of_simulations) / max_simulation);
		for (int i = 0; i < func_calls; ++i)
		{
			auto simulations = (number_of_simulations > max_simulation) ? max_simulation : number_of_simulations;
			number_of_simulations -= simulations;
			futures.push_back(std::async(static_cast<std::shared_ptr<MCGatherer<rettype> >(\
				MCGeneric<argtype, rettype, random_number_generator_type>::*)(size_t, unsigned long)>\
				(&MCGeneric<argtype, rettype, random_number_generator_type>::simulate), this, mcgatherer.dimension(), simulations));
		}

		for (auto &val : futures)
		{
			mcgatherer += *(val.get());
		}
	}

	template <class argtype, class rettype, class random_number_generator_type>
	double MCGeneric<argtype, rettype, random_number_generator_type>::simulate(MCGatherer<rettype>& mcgatherer,
		unsigned long initial_number_of_simulations,
		double required_accuracy,
		double confidence_level)
	{
		unsigned long n = initial_number_of_simulations;
		simulate(mcgatherer, initial_number_of_simulations);
		double d = boost::math::quantile(N, confidence_level); // was N.inverse(confidence_level);
		double current_accuracy = d * mcgatherer.max_stddev();
		while (required_accuracy < current_accuracy)
		{
			double q = current_accuracy / required_accuracy;
			q *= q;
			unsigned long additional_simulations = n * q - n;
			if (!additional_simulations) break;
			simulate(mcgatherer, additional_simulations);
			n += additional_simulations;
			current_accuracy = d * mcgatherer.max_stddev();
		}
		return current_accuracy;
	}

} /* namespace derivative */

#endif /* _DERIVATIVE_MCGENERIC_H_ */
