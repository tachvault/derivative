/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_LINESEARCH_H_
#define _DERIVATIVE_LINESEARCH_H_

#include <stdexcept>
#include <limits>
#include <blitz/array.h>
#include "QFUtil.hpp"
#include "QFArrayUtil.hpp"
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
	namespace opt 
	{
		using blitz::Array;
		using blitz::firstDim;

		template <class rettype>
		double ParabolicFit(double x1,double x2,double x3,rettype f1,rettype f2,rettype f3)
		{
			double delta1 = x2 - x1;
			double delta3 = x2 - x3;
			double fdelta1 = f2 - f1;
			double fdelta3 = f2 - f3;
			double result = x2 - 0.5 * (delta1*delta1*fdelta3-delta3*delta3*fdelta1)/(delta1*fdelta3-delta3*fdelta1);
			return result;
		}

		template <class F,class rettype>
		bool GeneralBracketMinimum(F& f,Array<rettype,1>& point1,Array<rettype,1>& point2,Array<rettype,1>& point3,rettype& f1,rettype& f2,rettype& f3,double lambda1,double lambda2,double lambda3,const Array<double,1>& direction,double lambda_min,double lambda_max)
		{
			f1 = f(point1);   
			f2 = f(point2);   
			if (f1<f2) 
			{
				derivative::swap(point1,point2);
				std::swap(f1,f2); 
				std::swap(lambda1,lambda2); 
			}
			lambda3 = std::min(lambda_max,std::max(lambda_min,lambda2+2.0*(lambda2-lambda1)));
			point3 = point2 + (lambda3-lambda2) * direction;
			f3 = f(point3);
			while ((f3<f2)&&(lambda3>lambda_min)&&(lambda3<lambda_max)) 
			{
				point1  = point2.copy();
				f1      = f2;
				lambda1 = lambda2;
				point2  = point3.copy();
				f2      = f3;
				lambda2 = lambda3;
				lambda3 = std::min(lambda_max,std::max(lambda_min,lambda2+2.0*(lambda2-lambda1)));
				point3  = point2 + (lambda3-lambda2) * direction;
				f3      = f(point3); 
			}
			return ((f2<=f3)&&(f2<=f1));
		}

		template <class F,class rettype>
		class GeneralBrentLinesearch 
		{ 
		public:

			enum {TYPEID = CLASS_GENERALBRENTLINESEARCH_TYPE};

			inline GeneralBrentLinesearch(std::function<void (Array<rettype,1>&,const Array<rettype,1>&,double&,double&)> xset_bounds =  nullptr)
				: set_bounds(xset_bounds),lambda_min(-std::numeric_limits<double>::max()),lambda_max(std::numeric_limits<double>::max()) 
			{ };
			rettype operator()(F& f,Array<rettype,1>& currpos,const Array<rettype,1>& direction,rettype eps,unsigned long maxit = 10000);

		private:
			std::function<void (Array<rettype,1>&,const Array<rettype,1>&,double&,double&)> set_bounds;
			double lambda_min,lambda_max;

		};

		template <class F,class rettype>
		rettype GeneralBrentLinesearch<F,rettype>::operator()(F& f,Array<rettype,1>& currpos,const Array<rettype,1>& xdirection,rettype eps,unsigned long maxit)
		{
			Array<rettype,1> direction = xdirection.copy();
			direction /= sqrt(sum(sqr(xdirection))); // normalise vector
			if (set_bounds)
			{ // set_bounds is non-NULL if search domain is bounded
				set_bounds(currpos,direction,lambda_min,lambda_max);
				if (lambda_max-lambda_min<1e-7) return f(currpos);
				if (lambda_max<1e-7) { // reverse direction
					direction *= -1.0;
					lambda_min *= -1.0; 
					lambda_max *= -1.0; 
				}
			}
			unsigned long i;
			int  dim = currpos.extent(firstDim);
			int axis = 0;
			while ((direction(axis)==0)&&(axis<dim)) axis++;
			rettype fbracket1,fbracket2,fleastsofar;
			Array<rettype,1> bracket1   = currpos.copy();
			Array<rettype,1> leastsofar = currpos.copy();
			leastsofar += std::min(1.0,lambda_max/2.0) * direction;
			Array<rettype,1> bracket2   = currpos.copy();
			bool bracketed = GeneralBracketMinimum(f,bracket1,leastsofar,bracket2,fbracket1,fleastsofar,fbracket2,0.0,std::min(1.0,lambda_max/2.0),0.0,direction,lambda_min,lambda_max);
			if (!bracketed) { // return minimum at extreme end if minimum not bracketed
				if (fbracket1<fbracket2)
				{
					currpos = bracket1;
					return fbracket1;
				}
				else 
				{
					currpos = bracket2;
					return fbracket2;
				}
			}
			Array<rettype,1> secondleastsofar      = (fbracket1>fbracket2) ? bracket2.copy() : bracket1.copy();
			rettype          fsecondleastsofar     = (fbracket1>fbracket2) ? fbracket2 : fbracket1;
			Array<rettype,1> prevsecondleastsofar  = (fbracket1>fbracket2) ? bracket1.copy() : bracket2.copy();
			rettype          fprevsecondleastsofar = (fbracket1>fbracket2) ? fbracket1 : fbracket2;
			Array<rettype,1> latesteval            = bracket2.copy();
			rettype          flatesteval           = fbracket2;
			double           stepbeforelast        = 0.0;
			double           laststep              = stepbeforelast;
			for (i=0;i<maxit;i++)
			{
				double delta2 = (prevsecondleastsofar(axis)-leastsofar(axis))/direction(axis);  
				double delta3 = (secondleastsofar(axis)-leastsofar(axis))/direction(axis);  
				double lambda = ParabolicFit(0.0,delta2,delta3,fleastsofar,fprevsecondleastsofar,fsecondleastsofar);
				/* To be acceptable, the parabolic step must fall within the bounding interval 
				and imply less than half the movement of the step before last.
				Otherwise, bisect the larger segment */
				delta2 = (bracket1(axis)-leastsofar(axis))/direction(axis);  
				delta3 = (bracket2(axis)-leastsofar(axis))/direction(axis);  
				if (!(between(lambda,delta2,delta3)&&(std::abs(lambda)<0.5*stepbeforelast)&&(std::abs(lambda)>eps))) lambda = 0.5 * ((std::abs(delta2)>std::abs(delta3)) ? delta2 : delta3);
				// Return if done.
				if (std::abs(lambda)<eps)
				{
					currpos = latesteval;
					return flatesteval;
				}
				latesteval     = leastsofar + lambda*direction; 
				flatesteval    = f(latesteval);
				// Housekeeping
				stepbeforelast = laststep;
				laststep       = std::abs(lambda);
				if (flatesteval<fleastsofar) 
				{
					if (lambda*delta2>0.0)
					{
						bracket2  = leastsofar;
						fbracket2 = fleastsofar; 
					}
					else
					{
						bracket1  = leastsofar;
						fbracket1 = fleastsofar;
					}
				}
				else
				{
					if (lambda*delta2>0.0)
					{
						bracket1  = latesteval;
						fbracket1 = flatesteval; 
					}
					else 
					{
						bracket2  = latesteval;
						fbracket2 = flatesteval;
					}
				}
				if (flatesteval<fprevsecondleastsofar) 
				{
					if (flatesteval<fsecondleastsofar)
					{
						prevsecondleastsofar  = secondleastsofar;
						fprevsecondleastsofar = fsecondleastsofar;
						if (flatesteval<fleastsofar)
						{
							secondleastsofar  = leastsofar;
							fsecondleastsofar = fleastsofar;
							leastsofar        = latesteval;
							fleastsofar       = flatesteval; 
						}
						else
						{
							secondleastsofar  = latesteval;
							fsecondleastsofar = flatesteval;
						}
					}
					else
					{
						prevsecondleastsofar  = latesteval;
						fprevsecondleastsofar = flatesteval; 
					}
				}
			}
			if ((fsecondleastsofar-fleastsofar)/(std::abs(fleastsofar)+1e-6)<1e-6) 
			{
				currpos = leastsofar;
				return fleastsofar;
			}
			else
			{
				currpos = leastsofar;
				return fleastsofar; 
			}
			throw std::runtime_error("Brent minimisation failed to converge");
		}
	}
}

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

/* namespace derivative */

#endif /* _DERIVATIVE_LINESEARCH_H_ */
