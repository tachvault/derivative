/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_QFARRAYUTIL_H_
#define _DERIVATIVE_QFARRAYUTIL_H_

#include <stdexcept>
#include <iostream>
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
	using blitz::Array;
	using blitz::firstIndex;
	using blitz::firstDim;
	using blitz::secondDim;

	double const_func(const Array<double, 1>& x);

	template <class T, int N>
	void swap(Array<T, N>& A, Array<T, N>& B)
	{
		Array<T, N> tmp = A.copy();
		A = B.copy();
		B = tmp;
	}

	template <int N>
	Array<double, N> logistic_transform(const Array<double, N>& x, double a, double b)
	{
		Array<double, N> result = x.copy();
		result = a + (b - a) / (1 + exp(-x));
		return result;
	}

	template <int N>
	Array<double, N> inverse_logistic_transform(const Array<double, N>& x, double a, double b)
	{
		Array<double, N> result = x.copy();
		result = -log((b - a) / (x - a) - 1);
		return result;
	}

	/** Assuming that T is a vector sorted in ascending order, find the index i such that
	T(i) < xi <= T(i+1). */
	template <class X>
	int find_segment(X xi, const Array<X, 1>& T)
	{
		if (xi < T(0)) throw std::out_of_range("cannot extrapolate");
		for (int i = 1; i < T.extent(firstDim); i++) if (xi <= T(i)) return i - 1;
		throw std::out_of_range("cannot extrapolate");
	}

	/** Assuming that T is a vector of doubles sorted in ascending order, find the index i such that
	T(i) < xi <= T(i+1). */
	int SERVICE_UTIL_DLL_API find_segment(double xi, const Array<double, 1>& T);

	template <class X>
	int find_first(X xi, const Array<X, 1>& T)
	{
		for (int i = 0; i < T.extent(firstDim); i++) if (xi == T(i)) return i;
		return -1;
	}

	template <class X>
	void partial_sum(Array<X, 1>& T)
	{
		int i;
		for (i = 1; i < T.extent(firstDim); i++) T(i) += T(i - 1);
	}

	template <class X>
	Array<X, 1> unique_merge(const Array<X, 1>& A, const Array<X, 1>& B)
	{
		int apos = 0;
		int bpos = 0;
		Array<X, 1> result(A.extent(firstDim) + B.extent(firstDim));
		// size of the merged arrays
		int len = 0;
		while ((apos < A.extent(firstDim)) || (bpos < B.extent(firstDim)))
		{
			if (apos < A.extent(firstDim))
			{
				if (bpos < B.extent(firstDim))
				{
					if (A(apos) <= B(bpos))
					{
						result(len) = A(apos);
						if (B(bpos) == A(apos)) bpos++;
						apos++;
					}
					else
					{
						result(len) = B(bpos);
						bpos++;
					}
				}
				else
				{
					result(len) = A(apos);
					apos++;
				}
			}
			else
			{
				result(len) = B(bpos);
				bpos++;
			}
			len++;
		}
		result.resizeAndPreserve(len);
		return result;
	}

	/// Determine whether A is a subset of B
	template <class X>
	bool subset(const Array<X, 1>& A, const Array<X, 1>& B)
	{
		int i;
		for (i = 0; i < A.extent(firstDim); i++) 
		{
			if (-1 == find_first(A(i), B)) return false;
		}
		return true;
	}

	void CSVwrite(std::ostream& os, const Array<double, 1>& v);
	void CSVwrite(std::ostream& os, const Array<double, 2>& A);

}

/* namespace derivative */

#endif /* _DERIVATIVE_QFARRAYUTIL_H_ */
