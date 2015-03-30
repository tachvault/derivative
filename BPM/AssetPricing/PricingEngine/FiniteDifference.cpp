/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#include "FiniteDifference.hpp"
#include "InterfaceCLAPACK.hpp"

namespace derivative
{
	void LinearExtrapolationBC::operator()(Array<double,1>& x)
	{
		x(0) = 2.0*x(1) - x(2);
		int n = x.extent(firstDim)-1;
		x(n) = 2*x(n-1) - x(n-2);    
	}

	void DirichletBC::operator()(Array<double,1>& x)
	{
		x(0) = lc;
		int n = x.extent(firstDim)-1;
		x(n) = uc;    
	} 

	FiniteDifference::FiniteDifference(const std::shared_ptr<BlackScholesAssetAdapter>& xS,double xr,double xT,int xN,int xNj)
		: S(xS),r(xr),T(xT),N(xN),Nj(xNj),gridslice(2*xNj+1),tmp(2*xNj+1),boundary_condition(linear_extrapolationBC)
	{
		// precompute constants
		double sgm = S->GetVolatility(0.0,T);
		dt = T / N;
		nu = r - 0.5 * sgm*sgm;
		/* Adjust nu for dividend yield - note that average dividend yield over the life of the option is used. This is OK for European options,
		but inaccurate for path-dependent (including American) options when dividends actually are time-varying. */
		nu -= S->GetDivYield(0.0,T);
		dx = sgm * std::sqrt(3.0 * dt);
		edx = std::exp(dx);
		double sgmdx = sgm/dx;
		sgmdx *= sgmdx;
		pu = 0.5 * dt * (sgmdx + nu / dx);
		pm = 1.0 - dt * sgmdx - r * dt;
		pd = 0.5 * dt * (sgmdx - nu / dx);
	}

	void FiniteDifference::rollback(int from,int to)
	{
		int i,j;
		for (i=from-1;i>=to;i--) {
			for (j=1;j<=2*Nj-1;j++) tmp(j) = pu * gridslice(j+1) + pm * gridslice(j) + pd * gridslice(j-1);
			gridslice = tmp;
			// boundary conditions
			boundary_condition(gridslice); }
	}

	void FiniteDifference::rollback(int from,int to,std::function<double (double,double)> f)
	{
		int i,j;
		Array<double,1> u(2*Nj+1);
		for (i=from-1;i>=to;i--) 
		{
			for (j=1;j<=2*Nj-1;j++) tmp(j) = pu * gridslice(j+1) + pm * gridslice(j) + pd * gridslice(j-1);
			gridslice = tmp;
			// boundary conditions
			boundary_condition(gridslice); 
			// apply early exercise condition
			underlying (i,u);
			for (j=0;j<=2*Nj;j++) gridslice(j) = f(gridslice(j),u(j));
		}
	}

	double FiniteDifference::delta() const
	{
		double dS = S->GetAssetValue()->GetTradePrice() * (edx - 1.0/edx);
		return (gridslice(Nj+1) - gridslice(Nj-1))/dS;
	}

	double FiniteDifference::gamma() const
	{
		double dSdown = S->GetAssetValue()->GetTradePrice() * (1.0 - 1.0/edx);
		double dSup   = S->GetAssetValue()->GetTradePrice() * (edx - 1.0);
		double deltadown =  (gridslice(Nj) - gridslice(Nj-1))/dSdown;
		double deltaup   =  (gridslice(Nj+1) - gridslice(Nj))/dSup;
		return (deltaup-deltadown)/(S->GetAssetValue()->GetTradePrice() * 0.5 * (edx - 1.0/edx));
	}

	ImplicitFiniteDifference::ImplicitFiniteDifference(const std::shared_ptr<BlackScholesAssetAdapter>& xS,double xr,double xT,int xN,int xNj)
		: FiniteDifference(xS,xr,xT,xN,xNj),tridiag(2*xNj+1,3),rhs(2*xNj+1,1),sol(2*xNj+1,1)
	{
		int i;
		// precompute constants
		double sgm = S->GetVolatility(0.0,T);
		dx = sgm * std::sqrt(3.0 * dt);
		edx = std::exp(dx);
		double sgmdx = sgm/dx;
		sgmdx *= sgmdx;
		pu = -0.5 * dt * (sgmdx + nu / dx);
		pm = (1.0 + dt * sgmdx + r * dt);
		pd = -0.5 * dt * (sgmdx - nu / dx);
		// set up matrix of coefficients of tridiagonal system
		int n = tridiag.extent(blitz::firstDim);
		tridiag(0,1) = -1.0;
		tridiag(0,2) = 1.0;
		tridiag(n-1,0) = -1.0;
		tridiag(n-1,1) = 1.0;
		for (i=1;i<n-1;i++)
		{
			tridiag(i,0) = pd;
			tridiag(i,1)   = pm;
			tridiag(i,2) = pu;
		}
	}

	void ImplicitFiniteDifference::rollback(int from,int to)
	{
		int i,j;
		int n = rhs.extent(blitz::firstDim);
		sol(Range::all(),0) = gridslice;
		for (j=from-1;j>=to;j--) 
		{
			// set up right-hand side of system of equations
			rhs(0,0)   = lambda_lower;
			rhs(n-1,0) = lambda_upper;
			for (i=1;i<n-1;i++) {
				rhs(i,0) = sol(i,0); 
			}
			// solve the tridiagonal system
			interfaceCLAPACK::SolveTridiagonalSparse(tridiag,sol,rhs); 
		}
		gridslice = sol(Range::all(),0);
	}

	void ImplicitFiniteDifference::rollback(int from,int to,std::function<double (double,double)> f)
	{
		int i,j;
		Array<double,1> u(2*Nj+1);
		int n = rhs.extent(blitz::firstDim);
		sol(Range::all(),0) = gridslice;
		for (j=from-1;j>=to;j--) 
		{
			// set up right-hand side of system of equations
			rhs(0,0)   = lambda_lower;
			rhs(n-1,0) = lambda_upper;
			for (i=1;i<n-1;i++) 
			{
				rhs(i,0) = sol(i,0); 
			}
			// solve the tridiagonal system
			interfaceCLAPACK::SolveTridiagonalSparse(tridiag,sol,rhs); 
			// apply early exercise condition
			underlying(j,u);
			for (i=0;i<=2*Nj;i++) sol(i,0) = f(sol(i,0),u(i)); 
		}
		gridslice = sol(Range::all(),0);
	}

	CrankNicolson::CrankNicolson(const std::shared_ptr<BlackScholesAssetAdapter>& xS,double xr,double xT,int xN,int xNj)
		: FiniteDifference(xS,xr,xT,xN,xNj),tridiag(2*xNj+1,3),rhs(2*xNj+1,1),sol(2*xNj+1,1)
	{
		int i;
		// precompute constants
		double sgm = S->GetVolatility(0.0,T);
		dx = sgm * std::sqrt(3.0 * dt);
		edx = std::exp(dx);
		double sgmdx = sgm/dx;
		sgmdx *= sgmdx;
		pu = -0.25 * dt * (sgmdx + nu / dx);
		pm = 1.0 + 0.5 * dt * sgmdx + 0.5 * r * dt;
		pd = -0.25 * dt * (sgmdx - nu / dx);
		// set up matrix of coefficients of tridiagonal system
		int n = tridiag.extent(blitz::firstDim);
		tridiag(0,1) = -1.0;
		tridiag(0,2) = 1.0;
		tridiag(n-1,0) = -1.0;
		tridiag(n-1,1) = 1.0;
		for (i=1;i<n-1;i++) 
		{
			tridiag(i,0) = pd;
			tridiag(i,1)   = pm;
			tridiag(i,2) = pu; 
		}
	}

	void CrankNicolson::rollback(int from,int to)
	{
		int i,j;
		int n = rhs.extent(blitz::firstDim);
		sol(Range::all(),0) = gridslice;
		for (j=from-1;j>=to;j--)
		{
			// set up right-hand side of system of equations
			rhs(0,0)   = sol(1,0) - sol(0,0);
			rhs(n-1,0) = sol(n-1,0) - sol(n-2,0);
			for (i=1;i<n-1;i++) 
			{
				rhs(i,0) = -pu*sol(i+1,0) - (pm-2.0)*sol(i,0) - pd*sol(i-1,0); 
			}
			// solve the tridiagonal system
			interfaceCLAPACK::SolveTridiagonalSparse(tridiag,sol,rhs); 
		}
		gridslice = sol(Range::all(),0);
	}

	void CrankNicolson::rollback(int from,int to,std::function<double (double,double)> f)
	{
		int i,j;
		Array<double,1> u(2*Nj+1);
		int n = rhs.extent(blitz::firstDim);
		sol(Range::all(),0) = gridslice;
		for (j=from-1;j>=to;j--) 
		{
			// set up right-hand side of system of equations
			rhs(0,0)   = sol(1,0) - sol(0,0);
			rhs(n-1,0) = sol(n-1,0) - sol(n-2,0);
			for (i=1;i<n-1;i++) 
			{
				rhs(i,0) = -pu*sol(i+1,0) - (pm-2.0)*sol(i,0) - pd*sol(i-1,0);
			}
			// solve the tridiagonal system
			interfaceCLAPACK::SolveTridiagonalSparse(tridiag,sol,rhs); 
			// apply early exercise condition
			underlying(j,u);
			for (i=0;i<=2*Nj;i++) sol(i,0) = f(sol(i,0),u(i)); 
		}
		gridslice = sol(Range::all(),0);
	}

	LinearExtrapolationBC FiniteDifference::linear_extrapolationBC;
}
