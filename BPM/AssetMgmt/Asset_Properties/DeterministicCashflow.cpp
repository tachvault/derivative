/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#include <iostream>
#include "DeterministicCashflow.hpp"

namespace derivative
{
	DeterministicCashflow& DeterministicCashflow::operator=(const DeterministicCashflow& cf)
	{
		/// if cf this the same object as this then
		/// return reference to this
		if (this == &cf)
		{
			return *this;
		}

		/// deep copy from cf to this.
		timeline_ = cf.timeline_.copy();
		payments = cf.payments.copy();
		value = cf.value;

		return *this;
	};

	/// Calculate net present value of present and future (not past) cashflows.
	double DeterministicCashflow::NPV(const Array<double,1>& timeline) const
	{
		int i;
		double today = timeline(0);
		double npv = 0.0;
		for (i=1; i<timeline_.extent(firstDim); i++) 
		{
			if (timeline_(i)>=today) npv += payments(i-1)*timeline_(i);
		}
		return npv;
	}
}


