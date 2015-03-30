/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_NEWTONRAPHSON_H_
#define _DERIVATIVE_NEWTONRAPHSON_H_

#include <stdexcept>
#include <blitz/array.h>
#include <blitz/tinyvec-et.h>
#include <blitz/tinymat.h>
#include "InterfaceCLAPACK.hpp"

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
	using blitz::Range;
	using blitz::firstDim;
	using blitz::firstIndex;
	using blitz::secondIndex;
		
	namespace opt
	{

		template <class F> 
		class NumericalJacobian 
		{

		public:

			enum {TYPEID = CLASS_NUMERICALJACOBIAN_TYPE};

			inline NumericalJacobian(F& xf,double delta = 1E-6) : f(xf),d(delta) 
			{ };
			inline int argdim()
			{ 
				return f.argdim(); 
			};
			inline int retdim()
			{ 
				return f.retdim(); 
			};
			inline Array<double,1> operator()(Array<double,1>& x) 
			{ 
				return f(x); 
			};
			Array<double,2> Jacobian(Array<double,1>& x);    

		private:
			/** The function. Has to provide: Array<double,1> operator()(Array<double,1> x)
			int argdim()
			int retdim()  */
			F&                           f; 
			double                       d;  ///< The perturbation of x to calculate the Jacobian by finite differences.

		};

		template <class F>
		class NewtonRaphson
		{ 
		private:  
			class done 
			{ };

		public:
			inline NewtonRaphson(F& xf,double xeps,unsigned long xmaxit = 100000) 
				: f(xf),x_solution(xf.argdim()),target(xf.retdim()),eps(xeps),maxit(xmaxit),converged(false) 
			{ };
			Array<double,1> solve(Array<double,1>& currpos,const Array<double,1>& xtarget);

		private:

			/** The function. Has to provide: Array<double,1> operator()(Array<double,1> x)
			and Array<double,2> Jacobian(Array<double,1> x)
			int argdim()
			int retdim()  */
			F&                           f; 
			Array<double,1>     x_solution; ///< Argument yielding the target function value.
			Array<double,1>         target; ///< Target function value.
			double                     eps;
			unsigned long            maxit;
			bool                 converged;
		};

		template <class F>
		Array<double,1> NewtonRaphson<F>::solve(Array<double,1>& currpos,const Array<double,1>& xtarget)
		{
			unsigned long n;
			converged = false;
			Array<double,1> x = currpos.copy();
			Array<double,2> dx(x.extent(firstDim),1);
			target = xtarget;
			Array<double,1> y(target.extent(firstDim)); 
			Array<double,2> y_old(target.extent(firstDim),1);
			y_old(Range::all(),0) = target - f(x); 
			for (n=0;n<maxit;n++) 
			{
				Array<double,2> J = f.Jacobian(x);
				interfaceCLAPACK::SolveLinear(J,dx,y_old);
				x += dx(Range::all(),0);
				y  = target - f(x);
				if ((max(abs(dx))<eps)||(max(abs(y-y_old(Range::all(),0)))<eps))
				{
					converged  = true;
					x_solution = x;
					return x; 
				}
				y_old(Range::all(),0) = y; 
			}
			if (!converged) throw std::runtime_error("Multidimensional Newton-Raphson failed to converge");
		}

		template <class F>
		Array<double,2> NumericalJacobian<F>::Jacobian(Array<double,1>& x)
		{
			int i,j;
			Array<double,2> result(f.retdim(),f.argdim()); 
			Array<double,1> fx(f(x).copy());
			Array<double,1> dx(f.argdim());
			dx = 0.0;
			for (j=0;j<f.argdim();j++) 
			{
				dx(j) = d;
				Array<double,1> xdx(x + dx);
				Array<double,1> fxdx(f(xdx).copy());  
				for (i=0;i<f.retdim();i++) result(i,j) = (fxdx(i)-fx(i))/d;
				dx(j) = 0.0; 
			}
			return result;
		}
	}
}

/* namespace derivative */

#endif /* _DERIVATIVE_NEWTONRAPHSON_H_ */

