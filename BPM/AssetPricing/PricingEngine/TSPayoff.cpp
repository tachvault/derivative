/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#include <algorithm>
#include "TSPayoff.hpp"

namespace derivative
{
	double ZCBoption::operator()(const TermStructure& ts)
	{
		double result = sign * (ts(T)-K);
		return std::max(0.0,result);      
	}
}


