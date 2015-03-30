/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#include "GaussMarkovTermStructure.hpp"

namespace derivative
{
	GaussMarkovTermStructure::GaussMarkovTermStructure(double xtoday,const GaussianHJM& xhjm,const Array<double,1>& xstatevariables) 
		: TermStructure(2),today(xtoday),hjm(xhjm),statevariables(xstatevariables.copy())
	{
		T(0) = today;
		T(1) = hjm.time_horizon();
		B(0) = 1.0;
		B(1) = hjm.bond(statevariables,today,T(1)-T(0));
	}

	double GaussMarkovTermStructure::operator()(double t) const
	{
		double ttm = t - today;
		if (ttm<0.0) 
		{
			throw std::logic_error("ZCB maturity in the past in GaussMarkovTermStructure::operator()"); 
		}
		return hjm.bond(statevariables,today,ttm);
	}

	std::shared_ptr<TermStructure> GaussMarkovTermStructure::pointer_to_copy() const
	{
		return std::make_shared<GaussMarkovTermStructure>(today,hjm,statevariables);
	}

	void GaussMarkovTermStructure::reinitialise()
	{ }

	void GaussMarkovTermStructure::accept(QFNestedVisitor& visitor) const
	{
		throw std::logic_error("GaussMarkovTermStructure::accept not implemented");
	}

	const std::string& GaussMarkovTermStructure::name() const
	{
		static const std::string str("GaussMarkovTermStructure");
		return str;      
	}
}


