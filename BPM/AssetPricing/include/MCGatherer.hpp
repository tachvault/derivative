/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

/*
Monte Carlo algorithm which is implemented in MCGeneric.

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

#ifndef _DERIVATIVE_MCGATHERER_H_
#define _DERIVATIVE_MCGATHERER_H_

#include <memory>
#include <cmath>
#include <blitz/array.h>
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

#undef max;

namespace derivative
{
	using blitz::Array;
	using blitz::firstIndex;
	using blitz::secondIndex;
	using blitz::firstDim;
	using blitz::secondDim;

	/*
	The template argument T determines the type of the variable representing the
	payoff. For a single payoff this would typically be double, but that this also 
	could be Array<double,1>, for example, representing multiple payoffs evaluated
	in the same Monte Carlo	simulation.
	*/
	template <class T>
	class MCGatherer
	{
	public:

		enum {TYPEID = CLASS_MCGATHERER_TYPE};

		inline MCGatherer() : sum(0.0),sum2(0.0),c(0)
		{ };
		/// Constructor for types T which require a size argument on construction.
		inline MCGatherer(size_t array_size)
			: sum(array_size),sum2(array_size)
		{
			reset();
		};
		void set_histogram(std::shared_ptr<Array<T,1> > xhistogram_buckets);
		inline void reset()
		{
			sum = 0.0;
			sum2 = 0.0;
			c = 0;
			if (histogram_buckets) (*histogram_count) = 0.0;
		};
		inline void operator+=(T add)
		{
			sum += add;
			sum2 += add*add;
			c++;
			if (histogram_buckets) add2bucket(add);
		};
		inline void operator*=(T f)
		{
			sum *= f;
			sum2 *= f*f;
		};

		/// Monte Carlo estimate
		inline T mean() const
		{   return T(sum/double(c));
		};

		/// Monte Carlo standard deviation
		inline T stddev() const
		{
			return T(sqrt((sum2/double(c)-mean()*mean())/double(c-1)));
		};
		inline T variance() const
		{
			return T((sum2/double(c)-mean()*mean())/double(c-1));
		};

		inline double max_stddev() const; // Must be specialised!

		inline size_t number_of_simulations() const
		{
			return c;
		};
		std::shared_ptr<Array<double,2> > histogram() const;

	private:
		std::shared_ptr<Array<T,1> > histogram_buckets;
		std::shared_ptr<Array<size_t,1> > histogram_count;
		T    sum;
		T   sum2;
		size_t c;
		void add2bucket(T add);
	};

	template <class T>
	void MCGatherer<T>::set_histogram(std::shared_ptr<Array<T,1> > xhistogram_buckets)
	{
		histogram_buckets.reset(xhistogram_buckets);
		histogram_count.reset(new Array<T,1>(xhistogram_buckets->extent(firstDim)+1));
		(*histogram_count) = 0.0;
	}

	template <class T>
	void MCGatherer<T>::add2bucket(T add)
	{
		int i = 0;
		while ((i<histogram_buckets->extent(firstDim))&&((*histogram_buckets)(i)>add)) i++;
		(*histogram_count)(i)++;
	}

	template <class T>
	std::shared_ptr<Array<double,2> > MCGatherer<T>::histogram() const
	{
		int i;
		if (!histogram_buckets) throw std::logic_error("Histogram not generated");
		std::shared_ptr<Array<double,2> > result(new Array<double,2>(2,histogram_count->extent(firstDim)));
		double prev = std::numeric_limits<T>::min();
		for (i=0; i<histogram_buckets->extent(firstDim); i++)
		{
			(*result)(0,i) = (*histogram_buckets)(i);
			(*result)(1,i) = double((*histogram_count)(i))/(double(c)*((*histogram_buckets)(i)-prev));
			prev           = (*histogram_buckets)(i);
		}
		(*result)(0,i) = std::numeric_limits<T>::max();
		(*result)(1,i) = double((*histogram_count)(i))/(double(c)*(std::numeric_limits<T>::max()-prev));
		return result;
	}

	/// Specialisation of function returning the maximum standard deviation of a set of MC estimates; specialised for double.
	template <>
	inline double MCGatherer<double>::max_stddev() const
	{
		return stddev();
	}

	/// Specialisation of MCGatherer for array of values, allowing some values to be used as control variates.
	template <>
	class MCGatherer<Array<double,1> >
	{
	public:
		inline MCGatherer(int array_size)
			: sum(array_size),sum2(array_size),covar(array_size,array_size),weights(array_size,1),cv_values(array_size),cv_indices(array_size),
			CVon(false),weights_fixed(false)
		{
			reset();
		};
		inline void reset()
		{
			sum = 0.0;
			sum2 = 0.0;
			c = 0;
			covar = 0.0;
		};
		inline void set_control_variate(bool cv)
		{  
			CVon = cv;
			if (cv) weights_fixed = false;
		};

		/// implements steps 4 and 5 of Monte Carlo algorithm.
		inline void operator+=(const Array<double,1>& add);

		inline void operator*=(const Array<double,1>& f)
		{
			sum *= f;
			sum2 *= f*f;
			if (CVon) covar = covar(idx,jdx) * f(idx)*f(jdx);
		};
		inline Array<double,1> mean() const
		{
			return Array<double,1>(sum/double(c));
		};
		inline double mean(int i) const
		{
			return (mean())(i);
		};
		inline Array<double,1> stddev() const
		{
			return Array<double,1>(sqrt((sum2/double(c)-mean()*mean())/double(c-1)));
		};
		inline double stddev(int i) const
		{
			return (stddev())(i);
		};
		inline Array<double,1> variance() const
		{
			return Array<double,1>((sum2/double(c)-mean()*mean())/double(c-1));
		};
		inline double variance(int i) const
		{
			return (variance())(i);
		};
		inline size_t number_of_simulations() const
		{
			return c;
		};
		/// Control variate estimate when control variate weights have been fixed.
		PRICINGENGINE_DLL_API double CVestimate(int target,int CV,double CV_expectation) const;
		/// Control variate estimate standard deviation when control variate weights have been fixed.
		PRICINGENGINE_DLL_API double CVestimate_stddev(int target,int CV) const;
		PRICINGENGINE_DLL_API double CVestimate(int target,const Array<int,1>& CV,const Array<double,1>& CV_expectation) const;
		PRICINGENGINE_DLL_API double CVweight(int target,int CV) const;
		PRICINGENGINE_DLL_API Array<double,2> CVweight(int target,const Array<int,1>& CV) const;
		inline PRICINGENGINE_DLL_API double max_stddev() const
		{
			blitz::Array<double,1> tmp(stddev());
			return blitz::max(tmp);
		};
		PRICINGENGINE_DLL_API void fix_weights(int target,const Array<int,1>& CV,const Array<double,1>& CV_expectation);
		inline PRICINGENGINE_DLL_API int dimension() const
		{
			return sum.extent(blitz::firstDim);
		};

	private:
		Array<double,1>                   sum;
		Array<double,1>                  sum2;
		Array<double,1>             cv_values;
		size_t                              c;
		Array<double,2>         covar,weights;
		Array<int,1>               cv_indices;
		firstIndex                        idx;
		secondIndex                       jdx;
		bool                             CVon;
		bool                    weights_fixed;
		int                number_of_controls;
		int                      target_index;
	};

	inline void MCGatherer<Array<double,1> >::operator+=(const Array<double,1>& add)
	{
		int i;
		if (weights_fixed) { // Control variate estimate when control variate weights have been fixed.
			double cv = 0.0;
			for (i=0; i<number_of_controls; i++) cv += weights(i) * (add(cv_indices(i)) - cv_values(i));
			for (i=0; i<add.extent(blitz::firstDim); i++)
			{
				double tmp;
				if (i==target_index) tmp = add(i) - cv;
				else                 tmp = add(i);
				sum(i)  += tmp;
				sum2(i) += tmp*tmp;
			}
		}
		else
		{
			sum  += add;
			sum2 += add*add;
		}
		c++;
		if (CVon) covar = covar(idx,jdx) + add(idx)*add(jdx);
	}

} /* namespace derivative */

#endif /* _DERIVATIVE_MCGATHERER_H_ */