/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_GRAMCHARLIER_H_
#define _DERIVATIVE_GRAMCHARLIER_H_

#include <stdexcept>
#include <functional>

#include <blitz/array.h>

#include "MSWarnings.hpp"
#include "Polynomial.hpp"
#include "Constants.hpp"
#include "LogisticMap.hpp"
#include "QFArrayUtil.hpp"
#include "Powell.hpp"
#include "ClassType.hpp"

#if defined _WIN32 || defined __CYGWIN__
  #ifdef SERVICEUTIL_EXPORTS
    #ifdef __GNUC__
      #define SERVICE_UTIL_DLL_API __attribute__ ((dllexport))
    #else
      #define SERVICE_UTIL_DLL_API __declspec(dllexport)
    #endif
  #else
    #ifdef __GNUC__
      #define SERVICE_UTIL_DLL_API __attribute__ ((dllimport))
    #else
      #define SERVICE_UTIL_DLL_API __declspec(dllimport)
    #endif
  #endif
  #define ESERVICE_UTIL_DLL_LOCAL
#else
  #if __GNUC__ >= 4
    #define SERVICE_UTIL_DLL_API __attribute__ ((visibility ("default")))
    #define SERVICE_UTIL_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define SERVICE_UTIL_DLL_API
    #define SERVICE_UTIL_DLL_LOCAL
  #endif
#endif

namespace derivative
{ 
	using blitz::Array;
	using blitz::firstDim;
	using blitz::Range;
	using blitz::firstIndex;
	using blitz::secondIndex;

	/// Class for a distribution given by a Gram/Charlier expansion.
	class SERVICE_UTIL_DLL_API GramCharlier 
	{

	public:

		enum {TYPEID = CLASS_GRAMCHARLIER_TYPE};

	private:
		Array<double,1> coeff;     ///< Coefficients of the Gram/Charlier expansion - coefficients beyond length of array are deemed to be zero.
		Polynomial       poly;     ///< Polynomial part of the density.
		LogisticMap       map;     ///< Logistic mapping to enforce bounds for sigma;
		class kz
		{
		public:
			//inline kz() : double_f_double(std::sqrt(3.0)) { };
			inline kz()
			{ };
			virtual double operator()(double x) const; 
		};
		void initialise_poly();
		/// Set bounds for lambda in line search.
		void set_bounds(Array<double,1>& currpos,const Array<double,1>& direction,double& lambda_min,double& lambda_max);
		class GCcalibration_class 
		{
		private:
			GramCharlier&                                parent;
			std::function<double (double)> objective_function;
		public:
			inline GCcalibration_class(GramCharlier& xparent,std::function<double (double)> xobjective_function) 
				: parent(xparent),objective_function(xobjective_function)
			{ };
			double operator()(const Array<double,1>& xpar);
		};
	public:
		/// Default constructor (standard normal distribution).
		inline GramCharlier() : coeff(1),map(1e-6,10.0) 
		{ 
			coeff = 1.0;
		};
		/// Constructor.
		GramCharlier(const Array<double,1>& xcoeff     ///< Coefficients of the Gram/Charlier expansion - coefficients beyond length of array are deemed to be zero.
			);
		inline const Array<double,1>& coefficients() const
		{ 
			return coeff; 
		};
		void set_coefficient(int i,double ci);
		inline double get_coefficient(int i) const;
		/// Density.
		inline double operator()(double x) const 
		{ 
			return RSQRT2PI * std::exp(-x*x/2.0) * real(poly(x)); 
		};
		/// Query skewness.
		double skewness() const;
		/// Query excess kurtosis.
		double excess_kurtosis() const;
		/// Verify whether Gram/Charlier expansion is a valid density using Jondeau/Rockinger (2001).
		bool JRverify() const;
		/// Verify whether Gram/Charlier expansion is a valid density by checking polynomial roots.
		bool verify();
		/// For a given excess kurtosis, the skewness on the frontier defining the set of valid Gram/Charlier densities as described in Jondeau/Rockinger (2001).
		double JRfrontier_skewness(double k) const;
		/// Transform unconstrained variables into constrained skewness and excess kurtosis, yielding a valid Gram/Charlier density as described in Jondeau/Rockinger (2001).
		void JRmap(double& s,double& k) const;
		void logistic_map(double& x,double a,double b) const;
		inline int highest_moment() const
		{ 
			return coeff.extent(firstDim)-1;
		};
		/// Find GC coefficients to minimise a given objective function.
		double fit_coefficients(std::function<double (double)> objective_function,double sgm,int dim,double eps = 1e-12);
	};

	inline double GramCharlier::get_coefficient(int i) const
	{
		if (i<coeff.extent(firstDim)) return coeff(i);
		else return 0.0;
	}
}

/* namespace derivative */

#endif /* _DERIVATIVE_GRAMCHARLIER_H_ */