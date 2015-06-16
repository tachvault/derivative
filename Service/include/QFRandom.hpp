/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_QFRANDOM_H_
#define _DERIVATIVE_QFRANDOM_H_

#include <mutex>
#include <blitz/array.h>
#include "SpinLock.hpp"

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
	template <class random_number_generator_type, class rntype>
	class RandomWrapper
	{
	private:

		RandomWrapper& operator=(const RandomWrapper&) = delete;
		RandomWrapper& operator=(const RandomWrapper&) volatile = delete;
		
		std::shared_ptr<random_number_generator_type>  rng;
		
	public:
		inline RandomWrapper(const std::shared_ptr<random_number_generator_type>& xrng) : rng(xrng)
		{ };
		
		RandomWrapper(const RandomWrapper& other)
		{
			rng = other.rng;
		}
		
		/// Returns a draw from random_number_generator_type.
		inline rntype random()
		{
			return rng->random();
		};

		inline void random_reference(Array<double, 2>& cnt)
		{
			return rng->random_reference(cnt);
		};
	};

	/** Template for an array-valued random number generator.

	The template parameter class random_number_generator_type must implement a member function random(),
	which returns a realisation of the random variable of the type given by the template parameter class rntype.
	*/
	template <class random_number_generator_type, class rntype>
	class RandomArray
	{
	private:
		blitz::Array<rntype, 2>    contents;
		std::shared_ptr<random_number_generator_type> rng;

		void Clone(const RandomArray<random_number_generator_type, rntype>& src);

	public:
		inline RandomArray(const std::shared_ptr<random_number_generator_type>& xrng, int rows, int cols) : contents(rows, cols), rng(xrng)
		{ };

		RandomArray(const RandomArray<random_number_generator_type, rntype>& rhs);

		RandomArray<random_number_generator_type, rntype>& operator= (const RandomArray<random_number_generator_type, rntype>& rhs);

		/// Returns an array of draws from random_number_generator_type.
		blitz::Array<rntype, 2>& random();
		void random_reference(blitz::Array<rntype, 2>&cnt);
		inline blitz::Array<rntype, 2>& previous_draw()
		{
			return contents;
		};
	};

	template <class random_number_generator_type, class rntype>
	blitz::Array<rntype, 2>& RandomArray<random_number_generator_type, rntype>::random()
	{
		static int a = 0;
		int i, j;
		for (i = 0; i < contents.extent(firstDim); i++)
		{
			for (j = 0; j < contents.extent(secondDim); j++) contents(i, j) = rng->random();
		}
		//cout << rng << ", " << ++a << endl;
		return contents;
	}

	template <class random_number_generator_type, class rntype>
	void RandomArray<random_number_generator_type, rntype>::random_reference(blitz::Array<rntype, 2>&cnt)
	{
		static int a = 0;
		int i, j;
		if ((contents.extent(firstDim) != cnt.extent(firstDim)) || (contents.extent(secondDim) != cnt.extent(secondDim)))
		{
			cnt.resize(contents.extent(firstDim), contents.extent(secondDim));
		}
		for (i = 0; i < cnt.extent(firstDim); i++)
		{
			for (j = 0; j < cnt.extent(secondDim); j++) cnt(i, j) = rng->random();
		}
	}

	template <class random_number_generator_type, class rntype>
	RandomArray<random_number_generator_type, rntype>::RandomArray(const RandomArray<random_number_generator_type, rntype>& rhs)
	{
		Clone(rhs);
	}

	template <class random_number_generator_type, class rntype>
	RandomArray<random_number_generator_type, rntype>& RandomArray<random_number_generator_type, rntype>::operator= \
		(const RandomArray<random_number_generator_type, rntype>& rhs)
	{
		/// if rhs this the same object as this then
		/// return reference to this
		if (*this == rhs)
		{
			return *this;
		}

		/// If rhs is different from this
		/// then clone the rhs and reference to this
		Clone(rhs);

		return *this;
	}

	template <class random_number_generator_type, class rntype>
	void RandomArray<random_number_generator_type, rntype>::Clone(const RandomArray<random_number_generator_type, rntype>& src)
	{
		contents.resize(src.contents.extent(blitz::firstDim), src.contents.extent(blitz::secondDim));
		rng = src.rng;
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
