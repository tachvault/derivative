/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#include "TSInstruments.hpp"

namespace derivative
{
	double Caplet::payoff(const TermStructure& ts)
	{
		double bond = ts(maturity()+delta);
		return notional * std::max(0.0,1.0-bond*(1.0+delta*lvl));   
	}
}
