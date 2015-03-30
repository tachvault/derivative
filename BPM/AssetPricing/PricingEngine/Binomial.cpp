/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#include "Binomial.hpp"
#include "QFUtil.hpp"

namespace derivative
{
	BinomialLattice::BinomialLattice(const std::shared_ptr<BlackScholesAssetAdapter>& xS,double xr,double xT,int xN)
		: S(xS),r(xr),T(xT),N(xN),gridslice(xN),tmpslice(xN)
	{
		// precompute constants
		dt = T / (N-1);
		discount = std::exp(-r*dt);
		set_CoxRossRubinstein();
	}

	void BinomialLattice::set_JarrowRudd()
	{
		double sgm = S->GetVolatility(0.0,T);
		double mu  = (r - S->GetDivYield(0.0,T) - 0.5*sgm*sgm) * dt;
		double sd  = sgm*std::sqrt(dt);
		u  = std::exp(mu+sd);
		d  = std::exp(mu-sd);
		p  = 0.5;
	}

	void BinomialLattice::set_CoxRossRubinstein()
	{
		double sgm = S->GetVolatility(0.0,T);
		u  = std::exp(sgm*std::sqrt(dt));
		d  = 1.0/u;
		p  = (1.0/discount*std::exp(-S->GetDivYield(0.0,T)*dt)-d)/(u-d);
	}

	void BinomialLattice::set_Tian()
	{
		double sgm = S->GetVolatility(0.0,T);
		double v   = std::exp(sgm*sgm*dt);
		double tmp = std::sqrt(v*v + 2.0*v - 3.0); 
		double R   = std::exp(-S->GetDivYield(0.0,T)*dt)/discount;
		u  = 0.5*v*R * (v + 1.0 + tmp);
		d  = 0.5*v*R * (v + 1.0 - tmp);
		p  = (R-d)/(u-d);
	}

	/// Binomial lattice method of Leisen/Reimer (1996). K is the target strike.
	void BinomialLattice::set_LeisenReimer(double K)
	{
		// N-1 is the number of steps. This must be odd.
		int half = N/2;
		N = 2*half;
		gridslice.resize(N);
		tmpslice.resize(N);
		dt = T / (N-1);
		discount = std::exp(-r*dt);
		double sgm    = S->GetVolatility(0.0,T);
		double R      = std::exp(-S->GetDivYield(0.0,T)*dt)/discount;
		double d2     = (std::log(S->GetAssetValue()->GetTradePrice()/K)+(r-S->GetDivYield(0.0,T)-0.5*sgm*sgm)*T)/(sgm*std::sqrt(T));
		double pprime = PeizerPratt(d2+sgm*std::sqrt(T));
		p  = PeizerPratt(d2);
		u  = pprime*R/p;
		d  = (R-p*u) / (1.0-p);
	}

	double BinomialLattice::PeizerPratt(double z) const
	{
		double tmp = z/(double(N-1)+1.0/3.0+0.1/N);
		return 0.5 + sign(z)*std::sqrt(0.25 - 0.25*std::exp(-tmp*tmp*(double(N-1)+1.0/6.0)));
	}

	std::ostream& operator<<(std::ostream& os,const BinLattice& bl)
	{
		int j,k;
		int m = bl.dimension();
		for (k=0;k<m;k++) {
			for (j=0;j<k;j++) os << bl(k,j) << ',';
			os << bl(k,j) << std::endl; }
		return os;
	}
}
