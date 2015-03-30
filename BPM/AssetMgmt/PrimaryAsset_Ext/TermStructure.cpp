/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <functional>
#include "DeterministicCashflow.hpp"
#include "TSBootstrap.hpp"
#include "Rootsearch.hpp"
#include "Powell.hpp"
#include "QFUtil.hpp"
#include "Linesearch.hpp"
#include "QFArrayUtil.hpp"

using namespace std::placeholders;

namespace derivative
{
	int TermStructure::find_segment(double xi) const
	{
		if (xi<T(0)) throw std::out_of_range("cannot extrapolate");
		for (int i=1; i<T.extent(firstDim); i++) if (xi<=T(i)) return i-1;
		throw std::out_of_range("cannot extrapolate");
	}

	void TermStructure::approximate_fit(std::vector<DeterministicCashflow> cashflows,double eps)
	{
		fit_func fit_f(cashflows,*this);
		opt::GeneralBrentLinesearch<fit_func,double> ls;
		Array<double,1> currpos = B(blitz::Range(1,blitz::toEnd)).copy();
		currpos = inverse_logistic_transform(currpos,0.0,1.0);
		if (currpos.extent(firstDim)>1)
		{
			opt::GeneralPowell<fit_func,double,opt::GeneralBrentLinesearch<fit_func,double> > powell(fit_f,eps,ls,5000);
			powell.solve(currpos);
		}
		else
		{
			Array<double,1> direction(1);
			direction(0) = 1.0;
			ls(fit_f,currpos,direction,eps,5000);
		}
	}

	double TermStructure::fit_func::operator()(Array<double,1>& lnzcb)
	{
		int i;
		ts.B(blitz::Range(1,blitz::toEnd)) = logistic_transform(lnzcb,0.0,1.0);
		ts.reinitialise();
		double result = 0.0;
		for (i=0; i<(int)cashflows.size(); i++) 
		{
			double err = cashflows[i].market_value() - cashflows[i].NPV(ts.timeline());
			result += err*err;
		}
		return result;
	}

	void TermStructure::reinitialise()
	{ }

	Array<double,1> TermStructure::simple_rate(const Array<double,1>& xT,double delta) const
	{
		Array<double,1> erg(xT.extent(firstDim)-1);
		double rdelta = 1.0/delta;
		for (int i=0; i<erg.extent(firstDim); i++)
			erg(i) = rdelta * (operator()(xT(i))/operator()(xT(i+1)) - 1.0);
		return erg;
	}

	double TermStructure::simple_rate(double xT,double delta) const
	{
		double erg(1.0/delta);
		erg *= operator()(xT)/operator()(xT+delta) - 1.0;
		return erg;
	}

	Array<double,1> TermStructure::swaps(const Array<double,1>& xT,double delta) const
	{
		int n = xT.extent(firstDim)-1;
		Array<double,1> erg(n);
		int i = n - 1;
		erg(i) = operator()(xT(i+1));
		i--;
		while (i>=0) 
		{
			erg(i) = erg(i+1) + operator()(xT(i+1));
			i--;
		}
		erg *= delta;
		for (int j=0; j<n; j++) erg(j) = (operator()(xT(j)) - operator()(xT(n)))/erg(j);
		return erg;
	}

	double TermStructure::swap(double T0,int n,double delta) const
	{
		double erg = pvbp(T0,n,delta);
		return ((operator()(T0) - operator()(T0+n*delta)) / (delta * erg));
	}

	double TermStructure::swap(const Array<double,1>& tenor) const
	{
		return (operator()(tenor(0)) - operator()(tenor(tenor.extent(firstDim)-1))) / pvbp(tenor);
	}

	double TermStructure::pvbp(double T0,int n,double delta) const
	{
		double erg = 0.0;
		for (int i=1; i<=n; i++) erg += operator()(T0+i*delta);
		return erg;
	}

	double TermStructure::pvbp(const Array<double,1>& tenor) const
	{
		double erg = 0.0;
		for (int i=1; i<tenor.extent(firstDim); i++) erg += (tenor(i)-tenor(i-1))*operator()(tenor(i));
		return erg;
	}

	Array<double,1> TermStructure::ccfwd(const Array<double,1>& xT) const
	{
		int i;
		Array<double,1> tmpB(xT.extent(firstDim));
		for (i=0; i<tmpB.extent(firstDim); i++) tmpB(i) = operator()(xT(i));
		Array<double,1> F(xT.extent(firstDim)-1);
		for (i=0; i<F.extent(firstDim); i++) F(i) = std::log(tmpB(i)/tmpB(i+1))/(xT(i+1)-xT(i));
		return F;
	}

	double TermStructure::find_maturity(double target) const
	{
		int n = T.extent(firstDim);
		if ((target>B(0))||(target<B(n-1))) return -1.0;  // Maturity not found
		std::function<double (double)> objective_function = std::bind(&TermStructure::operator(),this,_1);
		Rootsearch<std::function<double (double)>,double,double> rs(objective_function,target,0.5*(T(0)+T(n-1)),0.25*(T(n-1)-T(0)),T(0),T(n-1));
		double result = rs.solve();
		return result;
	}

	/// For nested Visitor pattern.
	void TermStructure::accept(QFNestedVisitor& visitor) const
	{
		visitor.visit("Timeline",T);
		visitor.visit("ZCB",B);
	}

	TermStructure::~TermStructure()
	{
		LOG(INFO) << "Destructor is called" << std::endl;
	}
}