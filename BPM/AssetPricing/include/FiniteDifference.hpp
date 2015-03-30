/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_FINITEDIFFERENCE_H_
#define _DERIVATIVE_FINITEDIFFERENCE_H_

#include <stdexcept>
#include <functional>
#include "MSWarnings.hpp"
#include <blitz/array.h>
#include <blitz/tinyvec2.h>
#include <blitz/tinymat2.h>
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
	using blitz::TinyVector;
	using blitz::TinyMatrix;
	using blitz::Array;
	using blitz::Range;
	using blitz::firstIndex;
	using blitz::secondIndex;

	class PRICINGENGINE_DLL_API BoundaryCondition
	{ 
	public:
		virtual void operator()(Array<double,1>& x) = 0;
	};

	class PRICINGENGINE_DLL_API LinearExtrapolationBC : public BoundaryCondition 
	{
	public:
		virtual void operator()(Array<double,1>& x);
	};

	class PRICINGENGINE_DLL_API DirichletBC : public BoundaryCondition 
	{
	private:
		double lc;  ///< Constant lower boundary value.
		double uc;  ///< Constant upper boundary value.
	public:
		inline DirichletBC(double xlc,double xuc) : lc(xlc),uc(xuc) 
		{ };
		virtual void operator()(Array<double,1>& x);
	};

	class PRICINGENGINE_DLL_API FiniteDifference 
	{
	public:

		enum {TYPID = CLASS_FINITEDIFFERENCE_TYPE};

		FiniteDifference(const std::shared_ptr<BlackScholesAssetAdapter>& xS,double xr,double xT,int xN,int xNj);   

		/// Calculate the value of the underlying asset at a given grid point.
		inline double underlying(int i, ///< Time index
			int j  ///< State index
			) const
		{ 
			return S->GetAssetValue()->GetTradePrice() * std::exp((j-Nj) * dx); 
		};

		inline void underlying(int i,Array<double,1>& u) const;

		inline void apply_payoff(int i,std::function<double (double)> f);    

		virtual void rollback(int from,int to);

		/// Rollback with early exercise (or similar) condition.
		virtual void rollback(int from,int to,std::function<double (double,double)> f);

		inline double result() const
		{ 
			return gridslice(Nj); 
		};

		double delta() const; 

		double gamma() const; 

	protected:
		Array<double,1>                           gridslice; ///< The current time slice of the finite difference grid.
		Array<double,1>                                 tmp;
		BoundaryCondition&               boundary_condition;
		static LinearExtrapolationBC linear_extrapolationBC;
		const std::shared_ptr<BlackScholesAssetAdapter> S;
		double             r;
		double             T;
		int                N;
		int               Nj;
		double            dt;
		double            nu;
		double            dx;
		double            pu;
		double            pm;
		double            pd;
		double           edx;
	};

	inline void FiniteDifference::underlying(int i,Array<double,1>& u) const
	{
		int j;
		u(0) = S->GetAssetValue()->GetTradePrice() * std::exp(-Nj * dx);      
		for (j=1;j<=2*Nj;j++) u(j) = u(j-1) * edx;
	}

	inline void FiniteDifference::apply_payoff(int i,std::function<double (double x)> f) 
	{
		int j;
		Array<double,1> u(2*Nj+1);
		underlying (i,u);
		for (j=0;j<=2*Nj;j++) gridslice(j) = f(u(j));      
	}

	class PRICINGENGINE_DLL_API ImplicitFiniteDifference : public FiniteDifference
	{
	public:

		enum { TYPEID = CLASS_IMPLICIT_FINITEDIFFERENCE_TYPE};

		ImplicitFiniteDifference(const std::shared_ptr<BlackScholesAssetAdapter>& xS,double xr,double xT,int xN,int xNj);

		inline void set_derivative_at_boundary(double lower,double upper);

		virtual void rollback(int from,int to);

		/// Rollback with early exercise (or similar) condition.
		virtual void rollback(int from,int to,std::function<double (double,double)> f);

	private:
		Array<double,2> tridiag; ///< Coefficients of tridiagonal system of equations to be solved for each time step.
		Array<double,2>     rhs; ///< Right-hand side of tridiagonal system of equations to be solved for each time step.
		Array<double,2>     sol; ///< Space to hold the solution of tridiagonal system of equations to be solved for each time step.
		double lambda_lower,lambda_upper;
	};

	inline void ImplicitFiniteDifference::set_derivative_at_boundary(double lower,double upper) 
	{ 
		lambda_lower = lower * (underlying(0,1) - underlying(0,0)); 
		lambda_upper = upper * (underlying(0,2*Nj) - underlying(0,2*Nj-1)); 
	}

	class PRICINGENGINE_DLL_API CrankNicolson : public FiniteDifference 
	{
	public:

		enum {TYPEID = CLASS_CRANKNICOLSON_TYPE};

		CrankNicolson(const std::shared_ptr<BlackScholesAssetAdapter>& xS,double xr,double xT,int xN,int xNj);

		virtual void rollback(int from,int to);

		/// Rollback with early exercise (or similar) condition.
		virtual void rollback(int from,int to,std::function<double (double,double)> f);

	private:
		Array<double,2> tridiag; ///< Coefficients of tridiagonal system of equations to be solved for each time step.
		Array<double,2>     rhs; ///< Right-hand side of tridiagonal system of equations to be solved for each time step.
		Array<double,2>     sol; ///< Space to hold the solution of tridiagonal system of equations to be solved for each time step.
	};

}  /* namespace derivative */

#endif /* _DERIVATIVE_FINITEDIFFERENCE_H_ */
