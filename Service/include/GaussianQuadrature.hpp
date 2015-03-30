/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_GAUSSIANQUADRATURE_H_
#define _DERIVATIVE_GAUSSIANQUADRATURE_H_

#include <stdexcept>
#include <blitz/array.h>
#include <boost/function.hpp>

#include "Constants.hpp"
#include "MSWarnings.hpp"
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
	using blitz::Range;
	using blitz::firstIndex;
	using blitz::secondIndex;

	class SERVICE_UTIL_DLL_API GaussianQuadrature 
	{
	public:

		enum { TYPEID = CLASS_GAUSSIANQUADRATURE_TYPE};

		inline GaussianQuadrature(int xn,double xeps = 3.0e-14);
		inline const Array<double,1>& abscissas() const 
		{ 
			return x; 
		};
		inline const Array<double,1>& weights() const 
		{ 
			return w; 
		};
		double integrate(std::function<double (double t)> f) const;
		virtual double W(double t) const = 0;

	protected:
		int             n;
		Array<double,1> x;                     ///< abscissas
		Array<double,1> w;                     ///< weights
		double        eps;                     ///< Relative precision.
		static const int    maxit = 10;        ///< Maximum iterations.
	};

	inline GaussianQuadrature::GaussianQuadrature(int xn,double xeps) : eps(xeps),x(xn+1),w(xn+1),n(xn) 
	{ }

	class SERVICE_UTIL_DLL_API GaussHermite : public GaussianQuadrature
	{	
	public:

		enum { TYPEID = CLASS_GAUSSHERMITE_TYPE};

		inline GaussHermite(int xn);
		virtual double W(double t) const;

	private:
		void gausshermite();
	};

	inline GaussHermite::GaussHermite(int xn) 
		: GaussianQuadrature(xn)
	{
		gausshermite();
	}

	class SERVICE_UTIL_DLL_API GaussLaguerre : public GaussianQuadrature 
	{

	public:
		enum { TYPEID = CLASS_GAUSSLAGUERRE_TYPE};

		inline GaussLaguerre(int xn);
		virtual double W(double t) const;

	private:
		void initialise();
	};

	inline GaussLaguerre::GaussLaguerre(int xn) 
		: GaussianQuadrature(xn)
	{
		initialise();
	}

	class SERVICE_UTIL_DLL_API GaussLobattoKronrod : public GaussianQuadrature
	{	
	public:

		enum { TYPEID = CLASS_GAUSSLOBATTOKRONROD_TYPE};

		inline GaussLobattoKronrod(int xn);
		virtual double W(double t) const;

	private:
		void initialise();
	};

	inline GaussLobattoKronrod::GaussLobattoKronrod(int xn) 
		: GaussianQuadrature(xn)
	{
		initialise();
	}
}

/* namespace derivative */

#endif /* _DERIVATIVE_GAUSSIANQUADRATURE_H_ */