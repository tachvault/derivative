/*
Copyright (C) Nathan Muruganantha 2013 - 2015
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

	namespace stat
	{
		double mean(Array<double, 1> dataSet)
		{
			double sum = 0.0;
			int i = 0;
			int k = dataSet.size();
			for (i = 0; i < k; ++i)
			{
				sum = sum + dataSet(i);
			}
			return sum / k;
		}

		double stdDev(Array<double, 1> dataSet)
		{
			double sumSq = 0.0;
			int k = dataSet.size();
			double avg = mean(dataSet);
			for (int i = 0; i < k; ++i)
			{
				sumSq = sumSq + (dataSet(i) - avg)*(dataSet(i) - avg);
			}

			return std::sqrt(sumSq / (k - 1));
		}

		double var(Array<double, 1> arr)
		{
			double sumSq = 0;
			int k = arr.size();
			double avg = mean(arr);
			for (int i = 0; i < k; ++i)
			{
				sumSq = sumSq + (arr(i) - avg)*(arr(i) - avg);
			}
			return sumSq / (k - 1);
		}

		// Function to calculate the mean value of a set of n Arrays each
		// of dimension n namely a (n x n) matrix
		Array<double, 1> mean(Array<double, 2> X, int n)
		{
			Array<double, 1> meanX(n);
			for (int i = 0; i <= n - 1; i++)
			{
				meanX(i) = 0.0;
				for (int j = 0; j <= n - 1; j++)
				{
					meanX(i) += X(i, j);
				}
				meanX(i) = meanX(i) / n;
			}
			return meanX;
		}

		// Sum of a Array
		double sum(Array<double, 1> x)
		{
			int n = x.size();
			double Sum = 0.0;
			for (int i = 0; i <= n - 1; i++)
				Sum += x(i);
			return Sum;
		}

		// Calculates unbiased sample variance
		double variance(Array<double, 1> x)
		{
			double n = x.size();
			double sumM = 0.0;
			for (int i = 0; i <= n - 1; i++)
				sumM += x(i);
			double mean = sumM / n;
			double sumV = 0.0;
			for (int i = 0; i <= n - 1; i++)
				sumV += (x(i) - mean)*(x(i) - mean);
			return sumV / (n - 1);
		}
	}
}

/* namespace derivative */

#endif /* _DERIVATIVE_QFARRAYUTIL_H_ */
