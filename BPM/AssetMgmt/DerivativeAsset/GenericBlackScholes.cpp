/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#include "GenericBlackScholes.hpp"

namespace derivative 
{

	double GenericBlackScholes::operator()(double S1,double S2,double vol,int sign) const
	{
		double sd  = std::sqrt(vol);
		double h1  = (std::log(S1/S2) + 0.5*vol) / sd;
		return sign * (S1*boost::math::cdf(N,sign*h1)-S2*boost::math::cdf(N,sign*(h1-sd)));
	}   
}