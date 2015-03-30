/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_PAYOFF_H_
#define _DERIVATIVE_PAYOFF_H_

#include "ClassType.hpp"

#if defined _WIN32 || defined __CYGWIN__
#ifdef PRICINGENGINE_EXPORTS
#ifdef __GNUC__
#define PRICINGENGINE_DLL_API __attribute__ ((dllexport))
#else
#define PRICINGENGINE_DLL_API __declspec(dllexport)
#endif
#else
#ifdef __GNUC__
#define PRICINGENGINE_DLL_API __attribute__ ((dllimport))
#else
#define PRICINGENGINE_DLL_API __declspec(dllimport)
#endif
#endif
#define PRICINGENGINE_LOCAL
#else
#if __GNUC__ >= 4
#define PRICINGENGINE_DLL_API __attribute__ ((visibility ("default")))
#define PRICINGENGINE_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define PRICINGENGINE_DLL_API
#define PRICINGENGINE_LOCAL
#endif
#endif

namespace derivative
{
	class PRICINGENGINE_DLL_API Payoff 
	{
	public:
		enum {TYPEID = CLASS_PAYOFF_TYPE};

		inline Payoff(double xK,int xsign = 1) 
			: K(xK),sign(xsign) 
		{ };
		double operator()(double S);

	private:
		double    K;
		int    sign;

	};

	class PRICINGENGINE_DLL_API EarlyExercise 
	{
	public:

		enum {TYPEID = CLASS_EARLYEXERCISE_TYPE};

		inline EarlyExercise(Payoff& xpayoff) 
			: payoff(xpayoff) 
		{ };
		double operator()(double continuationValue,double S);

	private:
		Payoff& payoff;	
	};

	class PRICINGENGINE_DLL_API KnockOut 
	{
	public:

		enum {TYPEID = CLASS_KNOCKOUT_TYPE};

		// Constructor. Down-and-out (direction = -1) is the default; setting direction = 1 results in an up-and-out option.
		inline KnockOut(double xbarrier,int xdirection = -1) 
			: barrier(xbarrier),direction(xdirection)
		{ };
		double operator()(double continuationValue,double S);

	private:
		double   barrier;
		int    direction;
	};

} /* namespace derivative */

#endif /* _DERIVATIVE_PAYOFF_H_ */