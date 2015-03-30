/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_REGRESSION_H_
#define _DERIVATIVE_REGRESSION_H_

#include <vector>
#include "MSWarnings.hpp"
#include <blitz/array.h>
#include <boost/function.hpp>
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
	using blitz::Range;
	using blitz::firstDim;
	using blitz::firstIndex;
	using blitz::secondIndex;

	class SERVICE_UTIL_DLL_API PolynomialLeastSquaresFit 
	{
	public:

		enum {TYPEID = CLASS_POLYNOMIALLEASTSQUARESFIT_TYPE};

		/// Polynomial regression constructor.
		PolynomialLeastSquaresFit(const Array<double,1>& y,const Array<double,1>& x,int degree = 1);
		double predict(double x) const;
		inline const Array<double,2>& coefficients() const
		{
			return coeff; 
		};

	private:
		Array<double,2> coeff;   ///< Regression coefficients as column vector.
	};

	class SERVICE_UTIL_DLL_API LeastSquaresFit
	{
	public:

		enum {TYPEID = CLASS_LEASTSQUARESFIT_TYPE};

		/// Regression constructor.
		LeastSquaresFit(const Array<double,1>& y,
			const Array<double,2>& x, ///< Independent variables: (number of points) X (number of independent variables)
			const std::vector<std::function<double (const Array<double,1>&)> >& xbasis_functions);
		double predict(const Array<double,1>& x) const;
		inline const Array<double,2>& coefficients() const 
		{ 
			return coeff; 
		};

	private:
		Array<double,2> coeff;   ///< Regression coefficients as column vector.
		std::vector<std::function<double (const Array<double,1>&)> > basis_functions;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_REGRESSION_H_ */

