/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schl�gl
*/

#ifndef _DERIVATIVE_QFRANDOM_H_
#define _DERIVATIVE_QFRANDOM_H_

#include <blitz/array.h>

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
	using blitz::firstDim;
	using blitz::secondDim;

	/** Wrap random_number_generator_type which implements operator() to return a realisation of the random variable of the type given by the template parameter class rntype
	to return a realisation of the random variable via the member function random()
	*/
	template <class random_number_generator_type,class rntype>
	class RandomWrapper 
	{
	private:
		random_number_generator_type&  rng;
	public:
		inline RandomWrapper(random_number_generator_type& xrng) : rng(xrng) 
		{ };
		/// Returns a draw from random_number_generator_type.
		inline rntype random() 
		{ 
			return rng(); 
		};
	};

	/** Template for an array-valued random number generator.

	The template parameter class random_number_generator_type must implement a member function random(), 
	which returns a realisation of the random variable of the type given by the template parameter class rntype.
	*/
	template <class random_number_generator_type,class rntype>
	class RandomArray 
	{
	private:
		blitz::Array<rntype,2>    contents;
		random_number_generator_type&  rng;
	public:
		inline RandomArray(random_number_generator_type& xrng,int rows,int cols) : contents(rows,cols),rng(xrng)
		{ };
		/// Returns an array of draws from random_number_generator_type.
		blitz::Array<rntype,2>& random();
		inline blitz::Array<rntype,2>& previous_draw() 
		{ 
			return contents; 
		};
	};

	template <class random_number_generator_type,class rntype>
	blitz::Array<rntype,2>& RandomArray<random_number_generator_type,rntype>::random()
	{
		int i,j;
		for (i=0;i<contents.extent(firstDim);i++) 
		{
			for (j=0;j<contents.extent(secondDim);j++) contents(i,j) = rng.random(); 
		}
		return contents;
	}

	/// Antithetic variables mirrored around zero.
	template <class T> 
	T normal_antithetic(T arg)
	{
		return T(-arg);
	}

	/// Antithetic for uniform [0,1].
	template <class T> 
	T uniform_antithetic(T arg)
	{
		return T(1.0 - arg);
	}
}

/* namespace derivative */

#endif /* _DERIVATIVE_QFRANDOM_H_ */