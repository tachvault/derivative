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
#include "TSLinear.hpp"

namespace derivative
{
	std::shared_ptr<TermStructure> TSLinear::pointer_to_copy() const
	{
		return std::make_shared<TSLinear>(T,B);
	}

	/** Function returning the interpolated value, which for term structures of
	interest rates is the maturity t, time T(0) forward zero coupon bond price. */
	double TSLinear::operator()(double xi) const
	{
		int   idx = find_segment(xi);
		double alpha = (xi-T(idx))/(T(idx+1)-T(idx));
		return (1.0-alpha)*B(idx) + alpha*B(idx+1);
	}

	const std::string& TSLinear::name() const
	{
		static const std::string str("TSLinear");
		return str;
	}

	TSLinear::TSLinear(const Array<double,1>& xT,const TermStructure& xts)
		: TSBootstrap(xT.copy(),xT.copy())
	{
		int i;
		for (i=0; i<xT.extent(firstDim); i++) B(i) = xts(xT(i));
	}

	TSLinear::~TSLinear()
	{
		LOG(INFO) << "Destructor is called" << std::endl;
	}
}