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

#undef min
#undef max

namespace derivative
{	
	/** Bootstrap term structure from market value of deterministic cash flows.
	Note that the vector of cashflows needs to be passed by value, so that we have a copy, which we can sort.

	This bootstrap algorithm assumes that the cashflow times overlap with each other. That is in the case of 
	coupon bonds, each cashflow will have T array consists of all the periods that overlap with other. But shorter
	cash flows don't need to have zero cash flows after the final non-zero cash flow. Look at the unit test example
	in the file primaryasset_unittest.cpp.
	*/
	void TSBootstrap::bootstrap(std::vector<std::shared_ptr<DeterministicCashflow> > cashflows,double eps)
	{
		/// Get the last cash flow. The last cash flow should contain all the possible cash flow
		/// time lines. If the last flow does not have a cash flow in a time slot then it should have
		/// been filled with zero. This is a precondition for this algorithm to work.
		auto comp = [](const std::shared_ptr<DeterministicCashflow>& a, const std::shared_ptr<DeterministicCashflow>& b)
		{
			return (a->num_cashflow() < b->num_cashflow());
		};
		std::sort(cashflows.begin(),cashflows.end(), comp);
		
		const Array<double,1>& t = (cashflows[cashflows.size() -1])->timeline();

		/// resize the timeline array and initialize the last timeline and cashflow.
		/// note that the first cash flow of all the timeline should overlap. otherwise
		/// the algorithm cannot proceed.
		T = t;
		B.resize(T.extent(firstDim));
		B = std::numeric_limits<double>::min();
		B(0) = 1.0;
		int last = 0;		
		/// now bootstrap the cash flows starting from the first bond.
		/// It should be again noted that the timeline of each cash flow
		/// should be a subset of T (as previously assigned).
		for (int i=0; i<(int)cashflows.size(); i++)
		{
			std::shared_ptr<DeterministicCashflow> cf = cashflows[i];
			const Array<double,1>& t = cf->timeline();
			const Array<double,1>& payments = cf->cashflow();
			double target_npv = cashflows[i]->market_value();
			blitz::Range c(0,last);
			blitz::Range s(last,t.extent(firstDim)-1);
			bootstrap_class b(t,payments,s,c,t.extent(firstDim) -1,this);
			Rootsearch<bootstrap_class,double,double> rs(b,target_npv,B(last),0.1*B(last),0.0);
			rs.solve();
			for (int i = last; i < t.extent(firstDim) -1; ++i)
			{
				auto x =  t.extent(firstDim) -1;
				if (B(i) == std::numeric_limits<double>::min())
				{
					B(i) = B(last) + ((T(i) - T(last))*(B(t.extent(firstDim) -1) - B(last)))/((T(t.extent(firstDim) -1)) - T(last));
				}
			}				
			last = t.extent(firstDim) -1;
		}
	}

	double TSBootstrap::bootstrap_class::operator()(double b)
	{
		int i;
		ts_->B(idx) = b;
		double npv = 0.0;
		/// first use the completed discount factors from last
		/// bond to to discount the cash flows of this bond.
		for (i=0; i< completed.length(); i++)
		{
			npv += p_(completed(i)) * ts_->operator()(t_(completed(i)));
		}

		/// now move onto discount remaining cash flows. 
		int _last = slice.length() -1;
		for (i= 1; i< slice.length(); i++)
		{
			auto x1 = ts_->t(i) -  ts_->t(0);
			auto x2 = ts_->t(_last) -  ts_->t(0);
			auto x3 = ts_->operator()(t_(slice(_last))) - ts_->operator()(t_(slice(0)));
			npv += p_(slice(i)) * (ts_->operator()(t_(slice(0))) + (x1/x2)*x3);
		}
		return npv;
	}

	TSBootstrap::~TSBootstrap()
	{
		LOG(INFO) << "Destructor is called" << std::endl;
	}
}

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))
