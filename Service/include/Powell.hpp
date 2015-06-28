/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_POWELL_H_
#define _DERIVATIVE_POWELL_H_

#include <stdexcept>
#include <blitz/array.h>
#include <blitz/tinyvec2.h>
#include <blitz/tinymat2.h>

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

#undef min
#undef max

namespace derivative
{
	using blitz::TinyVector;
	using blitz::TinyMatrix;
	using blitz::Array;
	using blitz::Range;
	using blitz::firstIndex;
	using blitz::secondIndex;

	namespace opt
	{
		template <class F, class rettype, class linesearch, int dim>
		class Powell
		{
		private:
			class done
			{ };

		public:

			enum { TYPEID = CLASS_POWELL_TYPE };

			inline Powell(F& xf, rettype xeps, linesearch xlsearch, unsigned long xmaxit = 10000000)
				: f(xf), eps(xeps), lsearch(xlsearch), maxit(xmaxit), converged(false)
			{ };
			rettype solve(TinyVector<rettype, dim>& currpos);

		private:
			F&                           f; ///< The function. Has to provide: rettype operator()(Array<rettype,1> x)
			/** Line search object. Moves the current point (currpos) to the point that minimises the function (f)
			along a given direction. Should not change currpos if the function return value changes by less than eps.
			Has to provide: rettype operator()(F& f,TinyVector<rettype,dim>& currpos,const TinyVector<rettype,dim>& direction,rettype eps) */
			linesearch&            lsearch;
			TinyVector<rettype, dim> minarg; ///< Argument minimising the function.
			double                     eps;
			unsigned long            maxit;
			bool                 converged;
		};

		template <class F, class rettype, class linesearch, int dim>
		rettype Powell<F, rettype, linesearch, dim>::solve(TinyVector<rettype, dim>& currpos)
		{
			unsigned long n, m, k;
			rettype                   result = 0;
			TinyVector<rettype, dim> startpos = currpos;
			int                            N = dim;
			Array<TinyVector<rettype, dim>, 1> directions(dim);
			converged = false;
			try
			{
				for (m = 0; m < std::max((long unsigned int)1, maxit / (N*N)); m++)
				{
					// initialise directions to basis vectors
					directions = 0;
					for (k = 0; k < N; k++) (directions(k))(k) = 1;
					for (k = 0; k < N; k++) {
						for (n = 0; n<N; n++) result = lsearch(f, currpos, directions((n + k) % N), eps, maxit);
						directions(k) = currpos - startpos;
						// check for convergence
						if (eps>sqrt(sum(sqr(directions(k)))))
						{
							converged = true;
							minarg = currpos;
							throw done();
						}
						directions(k) *= 100.0;
						result = lsearch(f, currpos, directions(k), eps, maxit);
						startpos = currpos;
					}
				}
			}
			catch (done)
			{
				return result;
			}
			if (!converged) throw std::runtime_error("Powell minimisation failed to converge");
		}

		/********************************************************************
		**      Powell minimisation general version using Array<>          **
		********************************************************************/
		template <class F, class rettype, class linesearch>
		class GeneralPowell
		{
		private:
			F&                           f; ///< The function. Has to provide: rettype operator()(Array<rettype,1> x)
			/** Line search object. Moves the current point (currpos) to the point that minimises the function (f)
			along a given direction. Should not change currpos if the function return value changes by less than eps.
			Has to provide: rettype operator()(F& f,Array<rettype,1>& currpos,const Array<rettype,1>& direction,rettype eps) */
			linesearch&            lsearch;
			Array<rettype, 1>        minarg; ///< Argument minimising the function.
			double                     eps;
			int                      maxit;
			bool                 converged;
			class done
			{ };
		public:
			inline GeneralPowell(F& xf, rettype xeps, linesearch& xlsearch, unsigned long xmaxit = 10000000)
				: f(xf), eps(xeps), lsearch(xlsearch), maxit(xmaxit), converged(false)
			{ };
			rettype solve(Array<rettype, 1>& currpos);
		};

		template <class F, class rettype, class linesearch>
		rettype GeneralPowell<F, rettype, linesearch>::solve(Array<rettype, 1>& currpos)
		{
			int n, m, k;
			firstIndex i;
			secondIndex j;
			rettype                   result = 0;
			Array<rettype, 1>        startpos = currpos.copy();
			rettype               prevresult = f(startpos);
			int                            N = currpos.extent(firstDim);
			Array<rettype, 2> directions(N, N);
			converged = false;
			for (m = 0; m < std::max(1, maxit / (N*N)); m++)
			{
				// initialise directions to basis vectors
				directions = (i == j);
				for (k = 0; k<N; k++)
				{
					for (n = 0; n<N; n++) result = lsearch(f, currpos, directions((n + k) % N, Range::all()), eps, std::max(10, maxit / 1000));
					directions(k, Range::all()) = currpos - startpos;
					// check for convergence
					if (eps>sqrt(sum(sqr(directions(k, Range::all())))))
					{
						converged = true;
						minarg = currpos;
						break;
					}
					result = lsearch(f, currpos, directions(k, Range::all()), eps, std::max(10, maxit / 1000));
					startpos = currpos;
				}
				if (eps>prevresult - result)
				{
					converged = true;
					minarg = currpos;
					break;
				}
				else prevresult = result;
			}
			if (converged)
			{
				return result;
			}
			else
			{
				std::cerr << "Powell minimisation failed to converge\nCurrent position: " << currpos << "\nResult: " << result << std::endl;
				return result; // remove this!!!
				throw std::runtime_error("Powell minimisation failed to converge");
			}
		}
	}
}

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

/* namespace derivative */

#endif /* _DERIVATIVE_POWELL_H_ */

