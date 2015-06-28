/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#pragma once
#ifndef _DERIVATIVE_TSBINOMIAL_H_
#define _DERIVATIVE_TSBINOMIAL_H_

#include <stdexcept>
#include <memory>
#include <functional>

#include "MSWarnings.hpp"
#include <blitz/array.h>
#include "TSPayoff.hpp"
#include "TSInstruments.hpp"
#include "TSLogLinear.hpp"
#include "ClassType.hpp"

#if defined _WIN32 || defined __CYGWIN__
#ifdef PRICINGENGINE_EXPORTS
#ifdef __GNUC__
#define PRICINGENGINE_DLL_API __attribute__ ((dllexport))
#else
#define PRICINGENGINE_DLL_API __declspec(dllexport)
#endif
#else
#ifdef __GNUC__
#define PRICINGENGINE_DLL_API __attribute__ ((dllimport))
#else
#define PRICINGENGINE_DLL_API __declspec(dllimport)
#endif
#endif
#define PRICINGENGINE_LOCAL
#else
#if __GNUC__ >= 4
#define PRICINGENGINE_DLL_API __attribute__ ((visibility ("default")))
#define PRICINGENGINE_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define PRICINGENGINE_DLL_API
#define PRICINGENGINE_LOCAL
#endif
#endif

namespace derivative
{
	using blitz::Array;
	using blitz::Range;
	using blitz::firstIndex;
	using blitz::secondIndex;

	class PRICINGENGINE_DLL_API TSBinomialLatticeNode 
	{

	public:		

		enum {TYPEID = CLASS_TSBINOMIALLATTICENODE_TYPE};

		TSBinomialLatticeNode();

		inline double state_price() const
		{
			return Q;
		};  
		inline double& state_price() 
		{ 
			return Q; 
		};  
		inline double short_rate() const 
		{ 
			return r; 
		};  
		inline double& short_rate() 
		{ 
			return r;
		};  

	private:  
		double               Q; ///< State price of this node as viewed from the initial node.
		double               r; ///< Short rate realisation in this node.

		std::unique_ptr<TSLogLinear>        ts; ///< Term structure realisation in this node.

		friend class TSBinomialMethod;
	};

	inline TSBinomialLatticeNode::TSBinomialLatticeNode()
		: r(0.0),Q(0.0)
	{ }

	class PRICINGENGINE_DLL_API TSBinomialMethod 
	{

	public:

		enum {TYPEID = CLASS_TSBINOMIALMETHOD_TYPE};

		/// Constant volatility constructor.
		TSBinomialMethod(const TermStructure& xinitial_ts,double xsgm,const Array<double,1>& xT);

		/// Constructor with time dependent volatility.
		TSBinomialMethod(const TermStructure& xinitial_ts,const Array<double,1>& xsgm,const Array<double,1>& xT);

		~TSBinomialMethod();

		/// Calibrate to a given set of caplets or other compatible instruments. Returns remaining pricing error.
		double calibrate(std::vector<std::shared_ptr<TSEuropeanInstrument> > instruments);

		/// Create full term structures in each node by backward iteration through the lattice.
		void rollbackTermstructures();

		/// Test function for lattice.
		bool verify() const;

		/// Apply payoff f in period i
		inline void apply_payoff(int i,std::function<double (const TermStructure&)> f);    

		void rollback(int from,int to);

		/// Rollback all the way using state prices.
		void rollback(int from);

		/// Rollback with early exercise (or similar) condition.
		void rollback(int from,int to,std::function<double (double,const TermStructure&)> f);

		/// Access the calculated (rolled-back) price in node 0;
		inline double result() const
		{ 
			return gridslice(0); 
		};

		/// Price a European payoff
		double price(TSEuropeanInstrument& instrument);

		/// Price a Bermudan (or similarly path-dependent) payoff.
		double price(TSBermudanInstrument& instrument);
		/// Access short rates
		inline double short_rate(int time, int state) const 
		{ 
			return node(time, state).short_rate(); 
		};
		/// Access state prices
		inline double state_price(int time, int state) const 
		{ 
			return node(time, state).state_price(); 
		};

	private:

		inline void createLattice() 
		{
			createLattice(0,n); 
		};      

		void createLattice(int from,int to);      

		void allocateLattice();

		inline const TSBinomialLatticeNode& node(int i,int j) const 
		{
			return *(*nodes[i])[j]; 
		};

		inline const TSBinomialLatticeNode& node(int i,int j)
		{ 
			return *(*nodes[i])[j]; 
		};

		double bond_r(double r);

		inline void set_current_period(int i) 
		{
			current_period = i; 
		};

		inline void set_current_instrument(const std::shared_ptr<TSEuropeanInstrument>& instrument,int prev_i,int i)
		{
			curr_instrument = instrument; 
			prev_idx = prev_i; 
			idx = i; 
		};

		double price_current_instrument(double try_sgm);

		void resetTermstructures(int i);

	private:

		const TermStructure& initial_ts;
		Array<double,1>        T; ///< Time line.
		Array<double,1>       dt; ///< Period lengths.
		int                    n; ///< Number of time steps.
		Array<double,1>      sgm; ///< Volatility parameter.
		double                 p; ///< Probability of an up move.
		Array<double,1>  rfactor;
		int       current_period;
		std::vector<std::shared_ptr<std::vector<std::shared_ptr<TSBinomialLatticeNode> > > > nodes;
		Array<double,1> gridslice; ///< The current time slice of the finite difference grid.
		Array<double,1> tmp;

		// For volatility calibration.
		std::shared_ptr<TSEuropeanInstrument> curr_instrument;

		int prev_idx,idx;
	};

	inline void TSBinomialMethod::apply_payoff(int i,std::function<double (const TermStructure&)> f) 
	{
		gridslice.resize(nodes[i]->size());
		std::vector<std::shared_ptr<TSBinomialLatticeNode> >::iterator j;
		int jidx = 0;
		for (j=nodes[i]->begin();j!=nodes[i]->end();j++) 
		{
			if (!((*j)->ts)) throw std::logic_error("Term structure not available in this node");
			gridslice(jidx++) = f(*((*j)->ts)); 
		}
	}

} /* namespace derivative */

#endif /* _DERIVATIVE_TSBINOMIAL_H_ */
