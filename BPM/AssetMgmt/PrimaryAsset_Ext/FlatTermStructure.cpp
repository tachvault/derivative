/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "FlatTermStructure.hpp"
#include "DeterministicCashflow.hpp"
#include "TSBootstrap.hpp"
#include "Rootsearch.hpp"
#include "Powell.hpp"
#include "QFUtil.hpp"
#include "Linesearch.hpp"
#include "QFArrayUtil.hpp"

namespace derivative
{
	/// Flat term structure constructor.
	FlatTermStructure::FlatTermStructure(double lvl,    ///< Flat level of continuously compounded interest rates.
		double tzero,  ///< Start of maturity time line.
		double horizon ///< End of maturity time line.
		) : TermStructure(2),level(lvl)
	{
		T(0) = tzero;
		T(1) = horizon;
		B(0) = 1.0;
		B(1) = std::exp(-lvl*(horizon-tzero));
	}

	void FlatTermStructure::reinitialise()
	{
		level = -log(B(1))/T(1);
	}

	std::shared_ptr<TermStructure> FlatTermStructure::pointer_to_copy() const
	{
		return std::make_shared<FlatTermStructure>(level,T(0),T(T.extent(firstDim)-1));
	}

	/** Function returning the interpolated value, which for term structures of
	interest rates is the maturity t, time T(0) forward zero coupon bond price. */
	double FlatTermStructure::operator()(double t) const
	{
		if (t<T(0)) throw std::logic_error("Requested maturity before beginning of term structure");
		return std::exp(-level*(t-T(0)));
	}

	/// For nested Visitor pattern.
	void FlatTermStructure::accept(QFNestedVisitor& visitor) const
	{
		TermStructure::accept(visitor);
		visitor.visit("RateLevel",level);
	}

	const std::string& FlatTermStructure::name() const
	{
		static const std::string str("FlatTermStructure");
		return str;
	}	
}