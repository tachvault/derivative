/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#include "GaussianQuadrature.hpp"
#include "Polynomial.hpp"

namespace derivative
{
	double GaussianQuadrature::integrate(std::function<double (double t)> f) const
	{
		int i;
		double result = 0.0;
		for (i=1;i<=n;i++) result += w(i)*f(x(i))/W(x(i));
		return result;     
	}

	/* Given n this routine initialises the Arrays of abscissas x (sorted largest first) and weights w for an n point Gauss-Hermite quadrature. */
	void GaussHermite::gausshermite()
	{
		throw std::logic_error("Implementation cannot be distributed due to copyright issues - an alternative, freely distributable implementation is planned for a future release");
	}

	double GaussHermite::W(double t) const
	{
		return std::exp(-t*t);       
	}

	double GaussLaguerre::W(double t) const
	{
		return std::exp(-t);       
	}

	/// Initialise the weights w and abscissas x.
	void GaussLaguerre::initialise()
	{
		int i;
		// Create Laguerre polynomial L_n of order n+1 using recurrence relation - L_{n+1} is required to calculate weights below
		Array<complex<double>,1> one(2);
		one = 1.0, -1.0;
		Polynomial currL(one); // 1-x
		Polynomial prevL;      // 1
		// 2n+1-x:
		Polynomial two_n_plus_one_minus_x(one);
		for (i=2;i<=n+1;i++)
		{
			two_n_plus_one_minus_x += 2.0;
			Polynomial tmp(currL);
			tmp   *= two_n_plus_one_minus_x;
			prevL *= -(i-1.0);
			tmp   += prevL;
			tmp   *= 1.0/double(i);
			prevL  = currL;
			currL  = tmp; 
		}
		// Abscissas x are the roots of the Laguerre polynomial
		const Array<complex<double>,1>& roots = prevL.roots();
		for (i=1;i<=n;i++) x(i) = roots(i-1).real(); // note that indexing starts from 1, not 0 in this particular implementation
		// Calculate weights w - using eq. 14 at http://mathworld.wolfram.com/Laguerre-GaussQuadrature.html
		for (i=1;i<=n;i++)
		{
			double tmp = (n+1)*currL(x(i)).real();
			w(i) = x(i) / (tmp*tmp);
		}
	}

	double GaussLobattoKronrod::W(double t) const
	{
		return std::exp(-t);       
	}

	/// Initialise the weights w and abscissas x.
	void GaussLobattoKronrod::initialise()
	{
		throw std::logic_error("GaussLobattoKronrod not yet implemented");
	}
}
