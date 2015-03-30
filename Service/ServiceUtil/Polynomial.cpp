/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/
#include <iostream>
#include <cmath>
#include "Polynomial.hpp"
#include "DException.hpp"

namespace derivative
{
	double Polynomial::EPS = 2.0e-12;
	double Polynomial::EPSS = 1.0e-14;

	Polynomial::Polynomial(const Polynomial& p)
		: c(p.c.copy()),r(p.r.copy()),degree(p.Degree()),roots_available(p.roots_available)
	{ }

	Polynomial::Polynomial(const Array<double,1>& coefficients)
		: c(coefficients.extent(firstDim)),r(coefficients.extent(firstDim)-1),degree(coefficients.extent(firstDim)-1)
	{
		int idx;
		for (idx=0;idx<=degree;idx++) c(idx) = coefficients(idx);
	}

	complex<double> Polynomial::operator()(const complex<double>& x) const
	{
		int i;
		complex<double> y;
		y = c(Degree());
		for (i=Degree()-1;i>=0;i--) {
			y *= x;
			y += c(i); }
		return y; 
	}

	/// Calculate root using Laguerre's method given starting point x.
	complex<double> Polynomial::Laguerre(complex<double> x,const Array<complex<double>,1>& xc) const
	{
		throw std::logic_error("Implementation cannot be distributed due to copyright issues - an alternative, freely distributable implementation is planned for a future release");
	}

	void Polynomial::calc_roots()
	{
		int j;
		bool poly = (degree>0);
		if (poly) {
			bool allzero = true;
			for (j=1;j<c.extent(firstDim);j++) {
				if (c(j)!=0.0) allzero = false; }
			poly = !allzero; }
		if (poly) {
			throw std::logic_error("Implementation cannot be distributed due to copyright issues - an alternative, freely distributable implementation is planned for a future release"); }
		else throw DegeneratePolynomial();
	}

	int Polynomial::Degree() const 
	{ 
		int j;
		bool allzero = true;
		for (j=1;j<c.extent(firstDim);j++) {
			if (c(j)!=0.0) allzero = false; }
		return (allzero) ? 0 : degree; 
	}

	Polynomial& Polynomial::operator=(const Polynomial& p)
	{
		int i;
		if (degree!=p.Degree()) {
			c.resize(p.Degree()+1);   
			degree = p.Degree();     
			r.resize(p.Degree()); }
		for (i=0;i<=p.Degree();i++) c(i) = p.c(i);
		roots_available = p.roots_available;
		if (roots_available) for (i=0;i<p.Degree();i++) r(i) = p.r(i);
		return *this;
	}

	Polynomial& Polynomial::operator+=(const Polynomial& p)
	{
		int i;
		if (degree<p.Degree()) {
			c.resizeAndPreserve(p.Degree()+1);   
			for (i=degree+1;i<=p.Degree();i++) c(i) = 0.0;   
			degree = p.Degree();     
			r.resize(p.Degree()); }
		for (i=0;i<=p.Degree();i++) c(i) += p.c(i);
		roots_available = false;
		return *this;
	}

	Polynomial& Polynomial::operator-=(const Polynomial& p)
	{
		int i;
		if (degree<p.Degree()) {
			c.resizeAndPreserve(p.Degree()+1);   
			for (i=degree+1;i<=p.Degree();i++) c(i) = 0.0;   
			degree = p.Degree();     
			r.resize(p.Degree()); }
		for (i=0;i<=p.Degree();i++) c(i) -= p.c(i);
		roots_available = false;
		return *this;
	}

	Polynomial& Polynomial::operator*=(const Polynomial& p)
	{
		int i,j;
		int newdegree = degree + p.Degree();
		Array<complex<double>,1> nc(newdegree+1);   
		nc = 0.0;   
		r.resizeAndPreserve(newdegree); 
		for (i=0;i<=p.Degree();i++) {
			for (j=0;j<=degree;j++) nc(i+j) += p.c(i)*c(j); }
		c.resize(nc.shape());
		c = nc;
		if (roots_available && p.roots_available) {
			for (i=0;i<p.Degree();i++) r(i+degree) = p.r(i); }
		else roots_available = false;
		degree = newdegree;
		return *this;
	}

	Polynomial& Polynomial::operator+=(double d)
	{
		c(0) += d;
		roots_available = false;
		return *this;
	}

	Polynomial& Polynomial::operator-=(double d)
	{
		c(0) -= d;
		roots_available = false;
		return *this;
	}

	Polynomial& Polynomial::operator*=(double d)
	{
		int j;
		for (j=0;j<=Degree();j++) c(j) *= d; 
		return *this;
	}

	Polynomial operator*(double c,const Polynomial& p)
	{
		Polynomial y(p);
		y *= c;
		return y;           
	}

	const Array<complex<double>,1>& Polynomial::roots() 
	{ 
		if (Degree()==0) throw std::logic_error("No roots in polynomial of degree zero");
		//if (!roots_available) calc_roots(); 
		return r; 
	}
}
