/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/
#include <iostream>
#include <algorithm>
#include <functional> 
#include "GramCharlier.hpp"
#include "Rootsearch.hpp"
#include "Linesearch.hpp"

#undef min
#undef max

using namespace std::placeholders;

namespace derivative
{
	GramCharlier::GramCharlier(const Array<double,1>& xcoeff     ///< Coefficients of the Gram/Charlier expansion - coefficients beyond length of array are deemed to be zero.
		) : coeff(xcoeff.copy()),map(1e-6,10.0)
	{ 
		// Eliminate zero coefficients                          
		int i = coeff.extent(firstDim)-1;
		while ((i>0)&&(std::abs(coeff(i))<1e-7)) i--;
		if (i<coeff.extent(firstDim)-1) coeff.resizeAndPreserve(i+1);
		initialise_poly();
	}

	void GramCharlier::initialise_poly()
	{
		int i = coeff.extent(firstDim)-1;
		Polynomial tmppoly;
		// Initialise polynomial part of density.
		poly = tmppoly;
		poly *= coeff(0);
		if (i>0) 
		{
			Array<complex<double>,1> one(2);
			one = 0.0, 1.0;
			Polynomial H1(one);
			Polynomial tmp(H1);
			tmp *= coeff(1);
			poly += tmp;
			int j;
			Polynomial prevH(H1); // x
			Polynomial prevprevH; // 1
			for (j=2;j<=i;j++) 
			{
				Polynomial currH(prevH);
				currH     *= H1;
				prevprevH *= double(j-1);
				currH     -= prevprevH;
				Polynomial tmp(currH);
				tmp       *= coeff(j);
				poly      += tmp;
				prevprevH = prevH;
				prevH     = currH; 
			}
		} 
	}

	void GramCharlier::set_coefficient(int i,double ci)
	{
		int j;
		int prev_length = coeff.extent(firstDim);
		if (i>=prev_length) 
		{
			coeff.resizeAndPreserve(i+1);
			for (j=prev_length;j<i;j++) coeff(j) = 0.0;  
		}
		coeff(i) = ci;
		initialise_poly();
	}

	/// Query skewness.
	double GramCharlier::skewness() const
	{
		if (coeff.extent(firstDim)>3)  return coeff(3)*6.0;
		else                           return 0.0;
	}

	/// Query excess kurtosis.
	double GramCharlier::excess_kurtosis() const
	{
		if (coeff.extent(firstDim)>4)  return coeff(4)*24.0;
		else                           return 0.0;
	}

	bool GramCharlier::JRverify() const
	{
		int i;
		for (i=5;i<coeff.extent(firstDim);i++) if (coeff(i)!=0.0) throw std::out_of_range("Cannot verify Gram/Charlier density");
		double skew = skewness();    
		double kurt = excess_kurtosis();  
		skew = (skew>0.0) ? skew : -skew;
		return (skew<=JRfrontier_skewness(kurt));
	}

	double GramCharlier::JRfrontier_skewness(double k) const
	{
		if ((k<0.0)||(k>4.0)) throw std::out_of_range("Excess kurtosis out of range");
		kz f;
		Rootsearch<kz,double,double> rs(f,k,2.5,0.8,std::sqrt(3.0),std::numeric_limits<double>::max(),1E-10,1000000);
		double    z = rs.solve();
		double   z2 = z*z;
		double   dz = ((z2-3.0)*z2+9.0)*z2+9.0;
		double    s = -24.0*z*(z2-3.0)/dz;
		s = (s>0.0) ? s : -s;
		return s;     
	}

	double GramCharlier::kz::operator()(double z) const
	{
		double z2 = z*z;
		double dz = ((z2-3.0)*z2+9.0)*z2+9.0;
		return 72.0*(z2-1.0)/dz;
	}

	void GramCharlier::JRmap(double& s,double& k) const
	{
		logistic_map(k,0.0,3.99);     
		double s_upper = JRfrontier_skewness(k);
		logistic_map(s,-s_upper,s_upper);     
	}

	void GramCharlier::logistic_map(double& x,double a,double b) const
	{
		x = a + (b-a)/(1.0+std::exp(-x));
	}

	/// Verify whether Gram/Charlier expansion is a valid density by checking polynomial roots.
	bool GramCharlier::verify()
	{
		int i;
		bool imaginary = true;
		if (poly.Degree()>0) 
		{
			Array<complex<double>,1> roots = poly.roots();
			for (i=0;i<roots.extent(firstDim);i++) imaginary = imaginary && (std::abs(imag(roots(i)))>1e-6); 
		}
		return imaginary;
	}

	/// Find GC coefficients to minimise a given objective function.
	double GramCharlier::fit_coefficients(std::function<double (double)> objective_function,double sgm,int dim,double eps)
	{
		int i;
		dim = std::max(2,2*(dim/2));
		std::cout << "Commence calibration... Highest moment: " << dim << std::endl;
		for (i=highest_moment()+1;i<=dim;i++) set_coefficient(i,0.0);
		for (i=dim+1;i<=highest_moment();i++) set_coefficient(i,0.0); 
		GCcalibration_class cal(*this,objective_function);
		std::function<void (Array<double,1>&,const Array<double,1>&,double&,double&)> xset_bounds = std::bind(std::mem_fn(&GramCharlier::set_bounds),this,_1,_2,_3,_4);
		opt::GeneralBrentLinesearch<GCcalibration_class,double> ls(xset_bounds);
		opt::GeneralPowell<GCcalibration_class,double,opt::GeneralBrentLinesearch<GCcalibration_class,double> > powell(cal,eps,ls);
		Array<double,1> currpos(dim-1);
		currpos(0) = sgm;
		map.inverse(currpos(0));
		if (dim>3) currpos(Range(1,dim-2)) = coeff(Range(3,dim));
		std::cout << "Start Powell optimisation..." << std::endl;
		double result = powell.solve(currpos);
		return result;
	}

	double GramCharlier::GCcalibration_class::operator()(const Array<double,1>& xpar ///< xpar(1) is skewness, xpar(2) is excess kurtosis, etc.
		)
	{
		int i;
		// Sigma is in logistically mapped coordinates. All other parameters are not transformed.
		double sgm = xpar(0);
		parent.map(sgm);
		for (i=1;i<xpar.extent(firstDim);i++) parent.set_coefficient(i+2,xpar(i));
		return objective_function(sgm);
	}

	/// Set bounds for lambda in line search.
	void GramCharlier::set_bounds(Array<double,1>& currpos,const Array<double,1>& direction,double& lambda_min,double& lambda_max)
	{
		int n = currpos.extent(firstDim)-1;
		Array<double,1> ccoeff(n);
		ccoeff = currpos(Range(1,n));
		Array<double,1> dir(n);
		dir = direction(Range(1,n));
		bool zero = true;
		int i;
		for (i=0;i<dir.extent(firstDim);i++) zero = zero && (std::abs(dir(i))<1e-7);
		if (zero)
		{
			lambda_min = -1.0e8;
			lambda_max = 1.0e8; 
		}
		else
		{
			double lambda = 1.0;
			double prevlambda = 0.0;
			double outlambda = 1.0e6;
			// Current Hermite coefficients
			Array<double,1> hcoeff(n+3);
			while (outlambda-prevlambda>1.0e-8) 
			{
				// Move coefficients in chosen direction
				hcoeff(0) = 1.0;
				hcoeff(1) = 0.0;
				hcoeff(2) = 0.0;
				hcoeff(Range(3,n+2))  = ccoeff;
				hcoeff(Range(3,n+2)) += lambda * dir;
				// Check roots
				GramCharlier gc(hcoeff);
				if (gc.verify()) 
				{
					prevlambda = lambda;
					lambda     = (outlambda+lambda)/2.0; 
				}
				else
				{
					outlambda  = lambda;
					lambda     = (prevlambda+lambda)/2.0; 
				}
			}
			lambda_max = lambda;
			// Reverse direction
			dir *= -1.0;
			lambda = 1.0;
			prevlambda = 0.0;
			outlambda = 1.0e6;
			while (outlambda-prevlambda>1.0e-8) 
			{
				// Move coefficients in chosen direction
				hcoeff(0) = 1.0;
				hcoeff(1) = 0.0;
				hcoeff(2) = 0.0;
				hcoeff(Range(3,n+2))  = ccoeff;
				hcoeff(Range(3,n+2)) += lambda * dir;
				// Check roots
				GramCharlier gc(hcoeff);
				if (gc.verify()) 
				{
					prevlambda = lambda;
					lambda     = (outlambda+lambda)/2.0; 
				}
				else 
				{
					outlambda  = lambda;
					lambda     = (prevlambda+lambda)/2.0; 
				}
			}
			lambda_min = -lambda; 
		}
	}
}

#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))

