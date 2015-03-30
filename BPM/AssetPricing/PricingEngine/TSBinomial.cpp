/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#include <functional>
#include <algorithm>
#include "TSBinomial.hpp"
#include "Rootsearch.hpp"
#include "QFArrayUtil.hpp"

namespace derivative
{
	using namespace std::placeholders;

	TSBinomialMethod::~TSBinomialMethod()
	{		
	}

	TSBinomialMethod::TSBinomialMethod(const TermStructure& xinitial_ts,double xsgm,const Array<double,1>& xT)
		: T(xT.copy()),sgm(xT.extent(firstDim)-1),n(xT.extent(firstDim)-2),initial_ts(xinitial_ts), curr_instrument(nullptr),
		gridslice(xT.extent(firstDim)-1),tmp(xT.extent(firstDim)-1),dt(xT.extent(firstDim)-1),rfactor(xT.extent(firstDim)-1)
	{
		dt  = T(Range(1,n+1)) - T(Range(0,n));
		sgm = xsgm * sqrt(dt);
		allocateLattice();
		createLattice();
	}

	TSBinomialMethod::TSBinomialMethod(const TermStructure& xinitial_ts,const Array<double,1>& xsgm,const Array<double,1>& xT)
		: T(xT.copy()),sgm(xsgm.copy()),n(xT.extent(firstDim)-2),initial_ts(xinitial_ts), curr_instrument(nullptr),
		gridslice(xT.extent(firstDim)-1),tmp(xT.extent(firstDim)-1),dt(xT.extent(firstDim)-1),rfactor(xT.extent(firstDim)-1)
	{
		dt  = T(Range(1,n+1)) - T(Range(0,n));
		allocateLattice();
		createLattice();
	}

	void TSBinomialMethod::allocateLattice()
	{
		int i;
		std::vector<std::shared_ptr<TSBinomialLatticeNode> >::iterator j;
		p = 0.5;
		std::shared_ptr<std::vector<std::shared_ptr<TSBinomialLatticeNode> > > vec = std::make_shared<std::vector<std::shared_ptr<TSBinomialLatticeNode> > >();
		nodes.push_back(vec);
		std::shared_ptr<TSBinomialLatticeNode> newnode = std::make_shared<TSBinomialLatticeNode>();
		newnode->state_price() = 1.0;
		newnode->short_rate()  = initial_ts.simple_rate(T(0),dt(0));
		nodes[0]->push_back(std::move(newnode));
		for (i=0;i<n;i++)
		{
			std::shared_ptr<std::vector<std::shared_ptr<TSBinomialLatticeNode> > > newvec = \
				std::make_shared<std::vector<std::shared_ptr<TSBinomialLatticeNode> > >();
			newvec->reserve(i+1);
			nodes.push_back(newvec);
			for (j=nodes[i]->begin();j!=nodes[i]->end();j++) 
			{
				std::shared_ptr<TSBinomialLatticeNode> newnode = std::make_shared<TSBinomialLatticeNode>();
				nodes[i+1]->push_back(newnode); 
			}
			std::shared_ptr<TSBinomialLatticeNode> newnode = std::make_shared<TSBinomialLatticeNode>();
			nodes[i+1]->push_back(newnode); 
		}
	}

	void TSBinomialMethod::createLattice(int from,int to)
	{
		int i;
		std::vector<std::shared_ptr<TSBinomialLatticeNode> >::iterator j,k;
		rfactor = exp(sgm/std::sqrt(p*(1-p)));
		double lowest_rate = node(0,0).short_rate();
		std::function<double (double)> f;
		f = std::bind(std::mem_fun(&TSBinomialMethod::bond_r),this,_1);
		for (i=from;i<to;i++) 
		{
			// Calculate state prices for new nodes.
			double prev_part = 0.0;
			k = nodes[i+1]->begin();
			for (j=nodes[i]->begin();j!=nodes[i]->end();j++) 
			{
				double new_part = (*j)->state_price()/(1.0+dt(i)*(*j)->short_rate());
				(*k)->state_price() = (1.0-p)*prev_part + p*new_part;
				k++;
				prev_part = new_part;
			}
			(*k)->state_price() = (1.0-p)*prev_part;
			// Calculate lowest interest rate.
			set_current_period(i+1);
			Rootsearch<std::function<double (double)>,double,double> rs(f,initial_ts(T(i+2)),lowest_rate*0.8,lowest_rate*0.4,0.0);
			double r = lowest_rate = rs.solve();
			// Set short rates.
			for (j=nodes[current_period]->begin();j!=nodes[current_period]->end();j++) 
			{
				(*j)->short_rate() = r;
				r *= rfactor(i); 
			}
		}
	}

	double TSBinomialMethod::bond_r(double r)
	{
		double result = 0.0;      
		std::vector<std::shared_ptr<TSBinomialLatticeNode> >::iterator j;
		for (j=nodes[current_period]->begin();j!=nodes[current_period]->end();j++) {
			result += (*j)->state_price() / (1.0+dt(current_period)*r);
			r *= rfactor(current_period-1); }
		return result;
	}

	void TSBinomialMethod::rollbackTermstructures()
	{
		using std::exp;

		int       i;
		std::vector<std::shared_ptr<TSBinomialLatticeNode> >::iterator j;
		int    jidx;
		int       l;
		int sz = nodes[n]->size();
		Array<double,2> B(sz,sz);
		Array<double,2> prevB(sz,sz);
		// last period
		jidx = 0;
		B(0,Range::all()) = 1.0;
		prevB(0,Range::all()) = 1.0;
		for (j=nodes[n]->begin();j!=nodes[n]->end();j++)
		{
			B(1,jidx) = 1.0/(1.0+dt(n)*(*j)->short_rate()); 
			(*j)->ts.reset(new TSLogLinear(T(Range(n,n+1)),B(Range(0,1),jidx++))); 
		}
		// roll back
		for (i=n-1;i>=0;i--) 
		{
			prevB = B;
			jidx = 0;
			for (j=nodes[i]->begin();j!=nodes[i]->end();j++) 
			{
				double one_period_bond = 1.0/(1.0+dt(i)*(*j)->short_rate());
				B(0,jidx) = one_period_bond;
				// for (l=n-i+1;l>0;l--) {
				for (l=n-i;l>0;l--)
				{
					B(l,jidx) = one_period_bond * (p*prevB(l-1,jidx)+(1-p)*prevB(l-1,jidx+1));
				}
				// (*j)->ts  = new TSLogLinear(T(Range(i,n+1)),B(Range(0,n-i+1),jidx++)); }}
				(*j)->ts.reset(new TSLogLinear(T(Range(i,n)),B(Range(0,n-i),jidx++)));
			}
		}
	}

	bool TSBinomialMethod::verify() const
	{
		int i;
		bool result = false;
		if (node(0,0).ts) 
		{
			result = true;
			const TermStructure& ts = *(node(0,0).ts);
			for (i=0;i<T.extent(firstDim);i++)
			{
				std::cout << T(i) << ',' << initial_ts(T(i)) << ',' << ts(T(i)) << std::endl;
				if (std::abs(initial_ts(T(i))-ts(T(i)))>1e-8) result = false; 
			}
		}
		std::cout << "Using state prices:\n";
		std::vector<std::shared_ptr<TSBinomialLatticeNode> >::iterator j;
		for (i=0;i<=n;i++) 
		{
			double qsum = 0.0;
			for (j=nodes[i]->begin();j!=nodes[i]->end();j++) qsum += (*j)->state_price();
			std::cout << T(i) << ',' << initial_ts(T(i)) << ',' << qsum << std::endl;
			if (std::abs(initial_ts(T(i))-qsum)>1e-8) result = false;
		}
		return result;     
	}

	void TSBinomialMethod::rollback(int from,int to)
	{
		int i;
		tmp.resize(nodes[from-1]->size());
		std::vector<std::shared_ptr<TSBinomialLatticeNode> >::iterator j;
		for (i=from-1;i>=to;i--) 
		{
			int    jidx = 0;
			for (j=nodes[i]->begin();j!=nodes[i]->end();j++)
			{
				double one_period_bond = 1.0/(1.0+dt(i)*(*j)->short_rate());
				tmp(jidx) = one_period_bond * (p*gridslice(jidx) + (1-p)*gridslice(jidx+1)); 
				jidx++; }
			gridslice(Range(0,jidx-1)) = tmp(Range(0,jidx-1)); 
		}
	}

	void TSBinomialMethod::rollback(int from)
	{
		std::vector<std::shared_ptr<TSBinomialLatticeNode> >::iterator j = nodes[from]->begin();
		gridslice(0) *= (*j)->state_price();
		int i = 1;
		j++;
		for (;j!=nodes[from]->end();j++) 
		{
			gridslice(0) += gridslice(i++) * (*j)->state_price();
		}
	}

	void TSBinomialMethod::rollback(int from,int to,std::function<double (double,const TermStructure&)> f)
	{
		int i;
		tmp.resize(nodes[from-1]->size());
		std::vector<std::shared_ptr<TSBinomialLatticeNode> >::iterator j;
		for (i=from-1;i>=to;i--)
		{
			int    jidx = 0;
			for (j=nodes[i]->begin();j!=nodes[i]->end();j++) 
			{
				double one_period_bond = 1.0/(1.0+dt(i)*(*j)->short_rate());
				tmp(jidx) = one_period_bond * (p*gridslice(jidx) + (1-p)*gridslice(jidx+1)); 
				if (!((*j)->ts)) throw std::logic_error("Term structure not available in this node");
				tmp(jidx) = f(tmp(jidx),*((*j)->ts));
				jidx++; 
			}
			gridslice(Range(0,jidx-1)) = tmp(Range(0,jidx-1)); 
		}
	}

	double TSBinomialMethod::calibrate(std::vector<std::shared_ptr<TSEuropeanInstrument> > instruments)
	{
		int i;
	    std::sort(instruments.begin(),instruments.end(),TSEuropeanInstrument::shorter_maturity());
		std::vector<std::shared_ptr<TSEuropeanInstrument> >::iterator j;
		std::function<double (double)> f;
		f = std::bind(std::mem_fun(&TSBinomialMethod::price_current_instrument),this,_1);
		double prev_mat = T(0);
		int    prev_i   = 0;
		for (j=instruments.begin();j!=instruments.end();j++)
		{
			if ((*j)->today()!=T(0)) throw std::logic_error("Initial date mismatch");
			if ((*j)->maturity()<=prev_mat) throw std::logic_error("Too much overlap");
			prev_mat = (*j)->maturity();
			i = find_first(prev_mat,T);
			if (i<0) throw std::logic_error("Time line mismatch");
			// Calibrate to current instrument.
			set_current_instrument(*j,prev_i,i);
			Rootsearch<std::function<double (double)>,double,double> rs(f,(*j)->price(),sgm(i-1),0.05,0.000001);
			double fitted_sgm = rs.solve();
			sgm(Range(prev_i,i-1)) = fitted_sgm; 
			rfactor = exp(sgm/std::sqrt(p*(1-p)));
			prev_i = i; 
		}
		createLattice();
		rollbackTermstructures();
		// Calculate remaining squared error.
		double result = 0.0;
		for (j=instruments.begin();j!=instruments.end();j++) 
		{
			double err = price(**j) - (*j)->price();
			result += err*err;
		}
		return result;
	}

	double TSBinomialMethod::price(TSEuropeanInstrument& instrument)
	{
		std::function<double (const TermStructure&)> f;
		f = std::bind(std::mem_fun(&TSEuropeanInstrument::payoff),&instrument,_1);
		int i = find_first(instrument.maturity(),T);
		if (i<0) throw std::logic_error("Time line mismatch");
		apply_payoff(i,f);
		rollback(i);
		return result();         
	}

	double TSBinomialMethod::price_current_instrument(double try_sgm)
	{
		sgm(Range(prev_idx,idx-1)) = try_sgm;
		rfactor = exp(sgm/std::sqrt(p*(1-p)));
		createLattice(prev_idx,idx);
		resetTermstructures(idx);
		double result = price(*curr_instrument);
		return result;
	}

	void TSBinomialMethod::resetTermstructures(int i)
	{
		Array<double,1> B(2);
		B(0) = 1.0;
		std::vector<std::shared_ptr<TSBinomialLatticeNode> >::iterator j;
		for (j=nodes[i]->begin();j != nodes[i]->end();j++) 
		{
			B(1) = 1.0/(1.0+dt(i)*(*j)->short_rate());
			(*j)->ts.reset(new TSLogLinear(T(Range(i,i+1)),B));
		}
	}
}