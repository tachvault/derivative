/*
Copyright (C) Nathan Muruganantha 2015
*/

#ifndef _DERIVATIVE_PSEUDORANDOMARRAY_H_
#define _DERIVATIVE_PSEUDORANDOMARRAY_H_

#include <mutex>
#include <blitz/array.h>
#include "PseudoRandom.hpp"

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

	/** Template for an array-valued random number generator of Pseudo Random type.
	*/
	template <class random_number_generator_type, class rntype>
	class PseudoRandomArray
	{
	private:
		blitz::Array<rntype, 2>    contents;
		
		PseudoRandom<random_number_generator_type, rntype>& rngGen;

		void Clone(const PseudoRandomArray<random_number_generator_type, rntype>& src);

	public:
		inline PseudoRandomArray(int rows, int cols) 
			: contents(rows, cols),
			rngGen(PseudoRandom<random_number_generator_type, rntype>::getInstance())
		{};

		PseudoRandomArray(const PseudoRandomArray<random_number_generator_type, rntype>& rhs);

		PseudoRandomArray<random_number_generator_type, rntype>& operator= (const PseudoRandomArray<random_number_generator_type, rntype>& rhs);

		/// Returns an array of draws from random_number_generator_type.
		blitz::Array<rntype, 2>& random();
		void random_reference(blitz::Array<rntype, 2>&cnt);
	};

	template <class random_number_generator_type, class rntype>
	blitz::Array<rntype, 2>& PseudoRandomArray<random_number_generator_type, rntype>::random()
	{
		static int a = 0;
		int i, j;
		for (i = 0; i < contents.extent(firstDim); i++)
		{
			for (j = 0; j < contents.extent(secondDim); j++) contents(i, j) = rngGen.random();
		}
		//cout << rng << ", " << ++a << endl;
		return contents;
	}

	template <class random_number_generator_type, class rntype>
	void PseudoRandomArray<random_number_generator_type, rntype>::random_reference(blitz::Array<rntype, 2>&cnt)
	{
		static int a = 0;
		int i, j;
		if ((contents.extent(firstDim) != cnt.extent(firstDim)) || (contents.extent(secondDim) != cnt.extent(secondDim)))
		{
			cnt.resize(contents.extent(firstDim), contents.extent(secondDim));
		}
		for (i = 0; i < cnt.extent(firstDim); i++)
		{
			for (j = 0; j < cnt.extent(secondDim); j++) cnt(i, j) = rng.random();
		}
	}

	template <class random_number_generator_type, class rntype>
	PseudoRandomArray<random_number_generator_type, rntype>::PseudoRandomArray(const PseudoRandomArray<random_number_generator_type, rntype>& rhs)
		:rngGen(rhs.rngGen)
	{
		Clone(rhs);
	}

	template <class random_number_generator_type, class rntype>
	PseudoRandomArray<random_number_generator_type, rntype>& PseudoRandomArray<random_number_generator_type, rntype>::operator= \
		(const PseudoRandomArray<random_number_generator_type, rntype>& rhs)
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
	void PseudoRandomArray<random_number_generator_type, rntype>::Clone(const PseudoRandomArray<random_number_generator_type, rntype>& src)
	{
		contents.resize(src.contents.extent(blitz::firstDim), src.contents.extent(blitz::secondDim));
	}	
}

/* namespace derivative */

#endif /* _DERIVATIVE_PSEUDORANDOMARRAY_H_ */
