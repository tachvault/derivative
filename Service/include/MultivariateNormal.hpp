/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_MULTIVARIATENORMAL_H_
#define _DERIVATIVE_MULTIVARIATENORMAL_H_

#include "MSWarnings.hpp"
#include <blitz/array.h>
#include <random/normal.h>
#include "QFQuasiRandom.hpp"
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
	using blitz::secondDim;
	using ranlib::NormalUnit;

	/// Random number generator for multivariate normal distribution with zero mean and given covariance matrix.
	class SERVICE_UTIL_DLL_API MultivariateNormal 
	{
	public:

		enum {TYPEID = CLASS_MULTIVARIATENORMAL_TYPE};

		/// Constructor.
		MultivariateNormal(const Array<double,2>& covar,NormalUnit<double>& xrnd);
		MultivariateNormal(const Array<double,2>& covar);
		inline void transform_random_variables(Array<double,1>& x);
		/// Fill array x with the next set of variates.
		void operator()(Array<double,1>& x);
		/// Evaluate multivariate normal CDF by Monte Carlo integration
		double CDF(const Array<double,1>& d,unsigned long n = 1000000,bool quasi = false);
		void test(const Array<double,2>& covar,int n);
		// Accessors
		inline const Array<double,1>& get_lambda() const 
		{ 
			return lambda; 
		}; 
		inline const Array<double,1>& eigenvalues() const 
		{ 
			return eigval; 
		};
		inline int rank() const 
		{ 
			return eigdim;
		};

	private:
		int                  dim;     ///< Number of variates.
		int               eigdim;     ///< Number of significant eigenvalues.
		NormalUnit<double>&  rnd;     ///< Univariate normal random number generator.
		Array<double,1>   lambda;     ///< Square roots of non-zero eigenvalues of covariance matrix.
		Array<double,1>   eigval;     ///< Non-zero eigenvalues of covariance matrix.
		Array<double,2>   eigvec;     ///< Eigenvectors of the covariance matrix.
		Array<double,2>        v;     ///< Workspace.
		Array<double,2> covariance_matrix;     
		Array<double,2>  inverse;     ///< Moore-Penrose inverse of the covariance matrix.
		double              detR;     ///< Rank R determinant of the covariance matrix.
		void initialise(const Array<double,2>& covar);
		void PDF4CDF(const Array<double,1>& x,Array<double,1>& result,const Array<double,1>& c);

	};

	inline void MultivariateNormal::transform_random_variables(Array<double,1>& x)
	{
		blitz::firstIndex i;
		blitz::secondIndex j;
		blitz::thirdIndex k;
		int c;
		for (c=0;c<eigdim;c++)
		{
			double tmp = x(c);
			v(c,0) = tmp * lambda(c); 
		}
		Array<double,2> tmp(dim,1);
		tmp = sum(eigvec(i,k)*v(k,j),k);
		x = tmp(blitz::Range::all(),0);
	}
}

/* namespace derivative */

#endif /* _DERIVATIVE_MULTIVARIATENORMAL_H_ */