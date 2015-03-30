/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#define _USE_MATH_DEFINES

#include <boost/math/distributions/normal.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include "MultivariateNormal.hpp"
#include "InterfaceCubature.hpp"
#include "InterfaceCLAPACK.hpp"

#undef min

using namespace derivative;

double bivarcumnorm(double x, double y, double rho); 
double trivarcumnorm(double x, double y, double z, double correl12, double correl13, double correl23);

NormalUnit<double> MultivariateNormal_default_rnd;

MultivariateNormal::MultivariateNormal(const Array<double,2>& covar,NormalUnit<double>& xrnd)
	: rnd(xrnd),dim(covar.extent(firstDim)),lambda(covar.extent(firstDim)),eigval(covar.extent(firstDim)),inverse(covar.extent(firstDim),covar.extent(secondDim)),
	eigvec(covar.extent(firstDim),covar.extent(secondDim)),v(covar.extent(firstDim),1),covariance_matrix(covar)
{
	initialise(covar);   
}

MultivariateNormal::MultivariateNormal(const Array<double,2>& covar)
	: rnd(MultivariateNormal_default_rnd),dim(covar.extent(firstDim)),lambda(covar.extent(firstDim)),inverse(covar.extent(firstDim),covar.extent(secondDim)),
	eigvec(covar.extent(firstDim),covar.extent(secondDim)),v(covar.extent(firstDim),1),eigval(covar.extent(firstDim)),covariance_matrix(covar)
{
	initialise(covar);   
}

void MultivariateNormal::initialise(const Array<double,2>& covar)
{    
	int i;
	interfaceCLAPACK::SymmetricEigenvalueProblem(covar,eigval,eigvec);
	eigdim = eigval.extent(firstDim);
	v.resize(eigdim,1);
	detR = 1.0;
	for (i=0;i<eigdim;i++) 
	{
		if (eigval(i)<0) eigval(i) = 0.0;
		detR *= eigval(i); 
	}
	lambda.resize(eigdim);
	lambda = sqrt(eigval);
	interfaceCLAPACK::MoorePenroseInverse(covar,inverse);
}

void MultivariateNormal::operator()(Array<double,1>& x)
{
	blitz::firstIndex i;
	blitz::secondIndex j;
	blitz::thirdIndex k;
	int c;
	for (c=0;c<eigdim;c++) x(c) = rnd.random();
	transform_random_variables(x);
}

void MultivariateNormal::test(const Array<double,2>& covar,int n)
{
	int i;
	blitz::firstIndex idx;
	blitz::secondIndex jdx;
	Array<double,2> tmp(dim,dim);
	Array<double,2> sum(dim,dim);
	Array<double,2> sum2(dim,dim);
	Array<double,1> x(dim);
	sum  = 0.0;
	sum2 = 0.0;
	for (i=0;i<n;i++)  
	{
		(*this)(x);
		tmp   = x(idx)*x(jdx);  
		sum  += tmp;
		sum2 += tmp * tmp;
	}
	sum  /= n;
	sum2 /= n;
	std::cout << "Input: " << covar << std::endl;
	std::cout << "Simulated: " << sum << std::endl;
	tmp = (sum2-sum*sum)/(n-1);
	tmp = sqrt(tmp);
	tmp = (covar-sum)/tmp;
	std::cout << "Each absolute value should be less than 2 with 95% probability: " << tmp << std::endl;
}

double MultivariateNormal::CDF(const Array<double,1>& d,unsigned long n,bool quasi)
{
	int j;
	unsigned long i;
	if (eigdim==1) 
	{
		double min_d = d(0)/(eigvec(0,0)*lambda(0));
		j = 1;
		while (j<dim) 
		{
			min_d = std::min(min_d,d(j)/(eigvec(j,0)*lambda(0)));
			j++; 
		}
		boost::math::normal normal;
		return boost::math::cdf(normal,min_d); 
	}
	if (d.extent(firstDim)==2) 
	{  // use bivariate implementation of West 
		double sgm1 = std::sqrt(covariance_matrix(0,0));
		double sgm2 = std::sqrt(covariance_matrix(1,1));
		return bivarcumnorm(d(0)/sgm1,d(1)/sgm2,covariance_matrix(0,1)/(sgm1*sgm2)); 
	}
	if (d.extent(firstDim)==3) 
	{  // use trivariate implementation of West 
		double sgm1 = std::sqrt(covariance_matrix(0,0));
		double sgm2 = std::sqrt(covariance_matrix(1,1));
		double sgm3 = std::sqrt(covariance_matrix(2,2));
		return trivarcumnorm(d(0)/sgm1,d(1)/sgm2,d(2)/sgm3,covariance_matrix(0,1)/(sgm1*sgm2),covariance_matrix(0,2)/(sgm1*sgm3),covariance_matrix(1,2)/(sgm3*sgm2));
	}
	if (d.extent(firstDim)<=7) 
	{  // use cubature 
		Array<double,1> xmin(dim);
		xmin = 0.0;
		Array<double,1> xmax(dim);
		xmax = 1.0;
		Array<double,1> val(1),err(1);
		unsigned maxEval = n; 
		double reqAbsError = 1e-12; 
		double reqRelError = 1e-9; 
		std::function<void (const Array<double,1>&,Array<double,1>&)> f = std::bind(&MultivariateNormal::PDF4CDF,this,std::placeholders::_1,std::placeholders::_2,d);
		cubature(f,xmin,xmax,maxEval,reqAbsError,reqRelError,val,err);
		return val(0); 
	}
	// otherwise use Monte Carlo
	unsigned long count = 0;
	Array<double,1> x(dim);
	if (!quasi) 
	{ // pseudo-random Monte Carlo
		for (i=0;i<n;i++) 
		{
			(*this)(x);
			bool below = true;
			for (j=0;j<dim;j++) below = below && (x(j)<=d(j));
			if (below) count++; 
		}
	}
	else 
	{ // quasi-random Monte Carlo
		SobolArrayNormal qrng(eigdim,1,n);
		for (i=0;i<n;i++) 
		{
			Array<double,2>& y = qrng.random();
			// associate lead dimensions with highest eigenvalues, so fill x backwards
			for (j=0;j<eigdim;j++) x(eigdim-1-j) = y(j,int(0));
			transform_random_variables(x);
			bool below = true;
			for (j=0;j<dim;j++) below = below && (x(j)<=d(j));
			if (below) count++; 
		}
	}
	return (double(count)/n);
}

void MultivariateNormal::PDF4CDF(const Array<double,1>& x,Array<double,1>& result,const Array<double,1>& c)
{
	Array<double,1> xc(1.0-x);
	Array<double,1> x1(c-x/xc);
	Array<double,2> tmp(1,dim),X(1,dim),product(1,1);
	blitz::firstIndex i;
	blitz::secondIndex j;
	blitz::thirdIndex k;
	int n;
	for (n=0;n<dim;n++) X(0,n) = x1(n);
	tmp = blitz::sum(X(i,k)*inverse(k,j),k);
	product = blitz::sum(tmp(i,k)*X(j,k),k);
	double factor = 1.0;
	for (n=0;n<dim;n++) factor *= xc(n)*xc(n);
	result(0) = 1.0/(std::pow(2.0*M_PI,eigdim/2.0)*std::sqrt(detR)) * std::exp(-0.5*product(0,0)) * 1.0/factor;
}

#define min(a,b)  ((a < b) ? a : b)
