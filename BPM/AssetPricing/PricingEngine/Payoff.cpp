/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#include <algorithm>
#include <iostream>
#include "Payoff.hpp"

	namespace derivative
{
	double Payoff::operator()(double S)
	{
		double result = sign * (S-K);
		return std::max(0.0,result);      
	}

	double EarlyExercise::operator()(double continuationValue,double S)
	{
		return std::max(continuationValue,payoff(S));       
	}

	double KnockOut::operator()(double continuationValue,double S)
	{
		return ((S*direction<barrier*direction) ? continuationValue : 0.0);       
	}
}

