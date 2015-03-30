/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_ROOTSEARCH_H_
#define _DERIVATIVE_ROOTSEARCH_H_

#include <limits>
#include <stdexcept>
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

#undef min
#undef max

namespace derivative 
{	
	template <class F,class argtype,class rettype>
	class Rootsearch 
	{ 

	public:

		enum {TYPEID = CLASS_ROOTSEARCH_TYPE};

		/// Constructor.
		inline Rootsearch(F& function,                  ///< The function. Has to provide: rettype operator()(argtype x)
			rettype xtarget,              ///< Function target value.
			argtype xini,                 ///< Initial point in root search.
			argtype xiniwidth,            ///< Initial width of search interval for bisection algorithm.
			argtype xlb = -std::numeric_limits<argtype>::max(), ///< Lower bound for function argument.
			argtype xub =  std::numeric_limits<argtype>::max(), ///< Upper bound for function argument.
			double xeps = 1E-9, 
			unsigned long xmaxit = 100000)
			: f(function),target(xtarget),ini(xini),iniwidth(xiniwidth),lb(xlb),ub(xub),eps(xeps),maxit(xmaxit),trace(false)
		{ };
		inline void set_lower_limit(argtype lower) 
		{ 
			lb = lower; 
		};
		inline void set_upper_limit(argtype upper)
		{
			ub = upper; 
		};
		inline void set_target(argtype y) 
		{ 
			target = y; 
		};
		inline void set_initial(argtype x)
		{ 
			ini = x;
		};
		inline void set_initial_width(argtype x)
		{ 
			iniwidth = x; 
		};
		inline argtype get_lower_limit()
		{ 
			return lb; 
		};
		inline argtype get_upper_limit() 
		{ 
			return ub; 
		};
		argtype solve();
		inline void set_trace(bool arg = true)
		{ 
			trace = arg; 
		};

	private:
		F&                         f; ///< The function. Has to provide: rettype operator()(argtype x)
		argtype                   lb; ///< Lower bound for function argument.
		argtype                   ub; ///< Upper bound for function argument.
		rettype               target; ///< Function target value.
		argtype                  ini; ///< Initial point in root search.
		argtype             iniwidth; ///< Initial width of search interval for bisection algorithm.
		double                   eps;
		unsigned long          maxit;
		bool                   trace;
		inline argtype amin(argtype x,argtype y) const { return (x>y) ? y : x; };
		inline argtype amax(argtype x,argtype y) const { return (x>y) ? x : y; };
		inline rettype rabs(rettype x) const { return (x>0) ? x : -x; };
	};

	/// A simple rootsearch.
	template <class F,class argtype,class rettype>
	argtype Rootsearch<F,argtype,rettype>::solve()
	{
		if ((ini>ub)||(ini<lb)) throw std::out_of_range("Domain error");
		if (trace) std::cout << "target:, " << target << "\nini:, " << ini << ',' << std::flush;
		rettype fm = f(ini);
		unsigned long i = 0;
		argtype m = ini;
		argtype u = ini + 0.5*iniwidth;
		if (u>ub) u = ub;
		if (trace) std::cout << fm << "\nu:, " << u << ',' << std::flush;
		rettype fu = f(u);
		argtype l = ini - 0.5*iniwidth;
		if (l<lb) l = lb;
		if (trace) std::cout << fu << "\nl:, " << l << ',' << std::flush;
		rettype fl = f(l);
		if (trace) std::cout << fl << std::endl;
		while ((rabs(fm-target)>eps)&&(i<maxit)&&(u-m>eps)&&(m-l>eps))
		{
			if (trace) std::cerr << std::endl << i << ' ' << rabs(fm-target) << ' ' << m << ' ' << fm << ' ' << u << std::endl;
			if (((target>fu)&&(target>fl))||((target<fu)&&(target<fl))) 
			{ // widen
				if (rabs(target-fu)>rabs(target-fl)) 
				{
					if (trace) std::cerr << "Pre-widening: " << l << ' ' << m << ' ' << u << std::endl << fl << ' ' << fm << ' ' << fu << std::endl;
					m  = l;
					fm = fl;
					l  = amax(lb,l-(u-l));
					fl = f(l);
					if (trace) std::cerr << "Post-widening: " << l << ' ' << m << ' ' << u << std::endl << fl << ' ' << fm << ' ' << fu << std::endl; 
					if (rabs(fl-target)<eps) return l; 
				}
				else
				{
					m  = u;
					fm = fu;
					u  = amin(ub,u+(u-l));
					fu = f(u);
					if (rabs(fu-target)<eps) return u;
				}
			}
			else
			{ // contract
				if (((target>fm)&&(target<fu))||((target<fm)&&(target>fu))) 
				{
					l  = m;
					m  = (u+l)/2.0;
					fl = fm;
					fm = f(m); 
				}
				else
				{
					u  = m;
					m  = (u+l)/2.0;
					fu = fm;
					fm = f(m); 
				}
			}
			i++; 
		}
		if (i==maxit) 
		{
			if (trace) std::cerr << "Number of iterations: " << i << std::endl;
			throw std::out_of_range("Maximum number of iterations exceeded"); 
		}
		if (trace) std::cerr << "Solved: " << m << ' ' << i << ' ' << f(m) << ' ' << target << std::endl;
		return(m);
	}
}

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

/* namespace derivative */

#endif /* _DERIVATIVE_ROOTSEARCH_H_ */
