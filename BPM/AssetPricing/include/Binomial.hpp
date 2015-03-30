/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_BINOMIAL_H_
#define _DERIVATIVE_BINOMIAL_H_

#include <stdexcept>
#include <iostream>
#include "MSWarnings.hpp"
#include <blitz/array.h>
#include <boost/function.hpp>
#include "BlackScholesAsset.hpp"
#include "BlackScholesAssetAdapter.hpp"
#include "ClassType.hpp"
#include "IAssetValue.hpp"

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

	class PRICINGENGINE_DLL_API BinomialLattice
	{
	public:

		enum {TYPID = CLASS_BINOMIAL_TYPE};

		BinomialLattice(const std::shared_ptr<BlackScholesAssetAdapter>& xS,double xr,double xT,int xN);   
		void set_CoxRossRubinstein();
		void set_JarrowRudd();
		void set_Tian();
		void set_LeisenReimer(double K);
		/// Calculate the value of the underlying asset at a given grid point.
		inline double underlying(int i, ///< Time index
			int j  ///< State index
			) const;
		inline void underlying(int i,Array<double,1>& un) const;
		inline void apply_payoff(int i,std::function<double (double)> f);    
		inline void rollback(int from,int to);
		/// Rollback with early exercise (or similar) condition.
		inline void rollback(int from,int to,std::function<double (double,double)> f);
		inline double result() const 
		{
			return gridslice(0); 
		};

	protected:
		Array<double,1>       gridslice; ///< The current time slice of the binomial lattice.
		Array<double,1>        tmpslice;
		const std::shared_ptr<BlackScholesAssetAdapter> S;
		double                        r;
		double                        T;
		int                           N;
		double                       dt;
		double                        p;
		double                        u;
		double                        d;
		double                 discount;
		double PeizerPratt(double z) const;
	};

	inline double BinomialLattice::underlying(int i,int j) const 
	{ 
		double tmp = 1.0;
		i -= j;
		while (j>0) 
		{
			tmp *= u;
			j--; 
		}
		while (i>0)
		{
			tmp *= d;
			i--; 
		}
		return S->GetAssetValue()->GetTradePrice() * tmp; 
	}

	inline void BinomialLattice::underlying(int i,Array<double,1>& un) const
	{
		int j;
		un(0) = underlying(i,0);      
		double tmp = u/d;
		for (j=1;j<=i;j++) un(j) = un(j-1) * tmp;
	}

	inline void BinomialLattice::apply_payoff(int i,std::function<double (double x)> f) 
	{
		int j;
		Array<double,1> un(i+1);
		underlying (i,un);
		for (j=0;j<=i;j++) gridslice(j) = f(un(j));      
	}

	inline void BinomialLattice::rollback(int from,int to)
	{
		int i,j;
		for (i=from-1;i>=to;i--) 
		{
			for (j=0;j<=i;j++) tmpslice(j) = discount * (p * gridslice(j+1) + (1-p) * gridslice(j));
			gridslice = tmpslice; 
		}
	}

	inline void BinomialLattice::rollback(int from,int to,std::function<double (double,double)> f)
	{
		int i,j;
		Array<double,1> un(from);
		for (i=from-1;i>=to;i--)
		{
			for (j=0;j<=i;j++) tmpslice(j) = discount * (p * gridslice(j+1) + (1-p) * gridslice(j));
			gridslice = tmpslice;
			// apply early exercise condition
			underlying (i,un);
			for (j=0;j<=i;j++) gridslice(j) = f(gridslice(j),un(j)); 
		}
	}

	class PRICINGENGINE_DLL_API BinLattice 
	{
	public:

		enum {TYPID = CLASS_BINLATTICE_TYPE};

		inline BinLattice(int xdim) : data((xdim*(xdim+1))/2),dim(xdim) 
		{ };
		inline double operator()(int i,int j) const
		{ 
			return data((i*(i+1))/2+j); 
		};
		inline double& operator()(int i,int j)
		{ 
			return data((i*(i+1))/2+j); 
		};
		inline BinLattice& operator=(double d)
		{
			data = d; return *this; 
		};
		inline BinLattice& operator=(const Array<double,1>& d) 
		{ 
			data = d; return *this; 
		};
		inline int dimension() const
		{
			return dim; 
		};
		inline const blitz::Array<double,1>& Array() const 
		{ 
			return data; 
		}; 

	private:
		blitz::Array<double,1> data;
		int              dim;

	};

	std::ostream& operator<<(std::ostream& os,const BinLattice& bl);

} /* namespace derivative */

#endif /* _DERIVATIVE_BINOMIAL_H_ */
