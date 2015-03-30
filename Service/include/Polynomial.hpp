/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_POLYNOMIAL_H_
#define _DERIVATIVE_POLYNOMIAL_H_

#include <stdexcept>
#include <complex>
#include "MSWarnings.hpp"
#include <blitz/array.h>
#include <blitz/tinyvec2.h>
#include <blitz/tinymat2.h>
#include "DException.hpp"
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
	using blitz::TinyVector;
	using blitz::TinyMatrix;
	using blitz::Array;
	using blitz::firstDim;
	using blitz::Range;
	using blitz::firstIndex;
	using blitz::secondIndex;
	using std::complex;

	class SERVICE_UTIL_DLL_API Polynomial 
	{	
	public:

		enum {TYPEID = CLASS_POLYNOMIAL_TYPE};

		/// Default constructor.
		inline Polynomial() : c(1),r(0),degree(0) 
		{
			roots_available = false; c(0) = 1.0; 
		};
		/// Constructor.
		inline Polynomial(const Array<complex<double>,1>& coefficients) 
			: c(coefficients.copy()),r(coefficients.extent(firstDim)-1),degree(coefficients.extent(firstDim)-1)
		{ 
			roots_available = false;
		};
		Polynomial(const Array<double,1>& coefficients); 
		/// Copy constructor.
		Polynomial(const Polynomial& p);
		/// Function evaluation.
		complex<double> operator()(const complex<double>& x) const;
		/// Root finding.
		const Array<complex<double>,1>& roots();
		/// Assignment operations.
		Polynomial& operator=(const Polynomial& p);
		Polynomial& operator+=(const Polynomial& p);
		Polynomial& operator-=(const Polynomial& p);
		Polynomial& operator*=(const Polynomial& p);
		Polynomial& operator+=(double d);
		Polynomial& operator-=(double d);
		Polynomial& operator*=(double d);
		/// Accessors.
		int Degree() const;
		inline const Array<complex<double>,1>& coefficients() const 
		{
			return c;
		};

	private:
		Array<complex<double>,1> c; ///< Polynomial coefficients in ascending order (i.e. coefficient of highest power last)
		Array<complex<double>,1> r; ///< Roots of the polynomial.
		bool roots_available;
		const static int MR = 8;
		const static int MT = 10;
		static double EPS;
		static double EPSS;
		int degree;
		complex<double> Laguerre(complex<double> x,const Array<complex<double>,1>& xc) const;
		void calc_roots();         
	};

	/// Arithmetic operations.
	Polynomial operator*(double c,const Polynomial& p);
}

/* namespace derivative */

#endif /* _DERIVATIVE_POLYNOMIAL_H_ */
