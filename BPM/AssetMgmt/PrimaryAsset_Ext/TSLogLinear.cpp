/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include "DeterministicCashflow.hpp"
#include "TSBootstrap.hpp"
#include "Rootsearch.hpp"
#include "Powell.hpp"
#include "QFUtil.hpp"
#include "Linesearch.hpp"
#include "QFArrayUtil.hpp"
#include "TSLogLinear.hpp"

namespace derivative
{
	std::shared_ptr<TermStructure> TSLogLinear::pointer_to_copy() const
	{
		return std::make_shared<TSLogLinear>(T,B);
	}

	/** Function returning the interpolated value, which for term structures of
	interest rates is the maturity t, time T(0) forward zero coupon bond price. */
	double TSLogLinear::operator()(double xi) const
	{
		int idx = find_segment(xi);
		if (xi==T(idx+1)) return B(idx+1);
		else return B(idx) * std::exp((xi-T(idx))/(T(idx+1)-T(idx))*std::log(B(idx+1)/B(idx)));
	}

	const std::string& TSLogLinear::name() const
	{
		static const std::string str("TSLogLinear");
		return str;
	}	
}