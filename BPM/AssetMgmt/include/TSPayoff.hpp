/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_TSPAYOFF_H_
#define _DERIVATIVE_TSPAYOFF_H_

#include "TermStructure.hpp"

#if defined _WIN32 || defined __CYGWIN__
#ifdef PRIMARYASSET_EXT_EXPORTS
#ifdef __GNUC__
#define PRIMARYASSET_EXT_API __attribute__ ((dllexport))
#else
#define PRIMARYASSET_EXT_API __declspec(dllexport)
#endif
#else
#ifdef __GNUC__
#define PRIMARYASSET_EXT_API __attribute__ ((dllimport))
#else
#define PRIMARYASSET_EXT_API __declspec(dllimport)
#endif
#endif
#define PRIMARYASSET_EXT_LOCAL
#else
#if __GNUC__ >= 4
#define PRIMARYASSET_EXT_API __attribute__ ((visibility ("default")))
#define PRIMARYASSET_EXT_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define PRIMARYASSET_EXT_API
#define PRIMARYASSET_EXT_LOCAL
#endif
#endif

namespace derivative
{
	class ZCBoption
	{
	private:
		double    T;
		double    K;
		int    sign;
	public:
		inline ZCBoption(double xK,double mat,int xsign = 1) : T(mat),K(xK),sign(xsign)
		{ };
		double operator()(const TermStructure& ts);
	};

	template <class T>
	class TSEarlyExercise 
	{
	private:
		T& payoff;
	public:
		inline TSEarlyExercise(T& xpayoff) : payoff(xpayoff) 
		{ };
		double operator()(double continuationValue,const TermStructure& ts);
	};

	template <class T>
	double TSEarlyExercise<T>::operator()(double continuationValue,const TermStructure& ts)
	{
		return std::max(continuationValue,payoff(ts));
	}

} /* namespace derivative */

#endif /* _DERIVATIVE_TSPAYOFF_H_ */