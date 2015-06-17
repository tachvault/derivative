/*
Modified and ported: 2013, Nathan Muruganantha. All rights reserved.
*/

#include <cmath>
#include "GroupRegister.hpp"
#include "EntityManager.hpp"
#include "DException.hpp"
#include "GARCH.hpp"
#include "StatisticsUtil.hpp"
#include "IAssetValue.hpp"
#include "IAsset.hpp"
#include "ConstVol.hpp"

namespace derivative
{	
	GARCH::GARCH(AssetClassTypeEnum assetCls, const std::shared_ptr<IAssetValue>& asset, const dd::date& processDate)
		:m_assetClass(assetCls),
		m_asset(asset),
		m_processedDate(processDate),
		m_initialized(false)
	{}
		
	std::shared_ptr<DeterministicAssetVol> GARCH::GetVolatility()
	{
		if (!m_initialized)
		{
			Array<double, 1> values = GetAssetValues();
			Array<double, 1> returns(values.size() - 1, 0.0);
			auto size = values.size();
			for (int i = 1; i < values.size(); ++i)
			{
				returns(i - 1) = std::log(values(i)/values(i - 1));
			}
			double variance = stat::var(returns);
			auto vol = m_garch.omega + m_garch.alpha*returns(returns.size() - 1)*returns(returns.size() - 1) + m_garch.beta*variance;
			auto sigma = std::sqrt(252 * vol);
			m_vol = std::make_shared<ConstVol>(sigma);
		}
		return m_vol;
	}

	void GARCH::Build()
	{	
		int N = 3;						// Number of GARCH(1,1) parameters
		double NumIters = 1;			// First Iteration
		double MaxIters = 1e2;			// Maximum number of iterations
		double Tolerance = 1e-15;		// Tolerance on best and worst function values

		//Array<Array<double,1> > s(N, Array<double,1>(N + 1));
		Array<double, 2> s(N, N + 1);

		// Vertice 0	Vertice 1		Vertice 2		Vertice 3		Vertice 4
		s(0, 0) = 0.00002;	s(0, 1) = 0.00001;	s(0, 2) = 0.00000015;		s(0, 3) = 0.0000005;
		s(1, 0) = 0.10;		s(1, 1) = 0.11;		s(1, 2) = 0.09;			s(1, 3) = 0.15;
		s(2, 0) = 0.85;		s(2, 1) = 0.87;		s(2, 2) = 0.90;			s(2, 3) = 0.83;

		// Array<double,1> NM = NelderMead(LogLikelihood, N, NumIters, MaxIters, Tolerance, s);
		Array<double, 1> NM = NelderMead(N, NumIters, MaxIters, Tolerance, s);

		m_garch.omega = NM(0);
		m_garch.alpha = NM(1);
		m_garch.beta = NM(2);
		m_garch.r = 1 - NM(1) - NM(2);
		m_garch.V = NM(0) / m_garch.r;
	}

	// Nelder Mead Algorithm
	Array<double, 1> GARCH::NelderMead(int N, double NumIters, double MaxIters,
		double Tolerance, Array<double, 2> x)
	{
		int i, j;

		// Value of the function at the vertices
		vector<vector<double> > F(N + 1, vector<double>(2));

		// Step 0.  Ordering and Best and Worst points
		// Order according to the functional values, compute the best and worst points
	step0:
		NumIters = NumIters + 1;
		for (j = 0; j <= N; j++)
		{
			Array<double, 1> z(N, 0.0);								// Create Array to contain
			for (i = 0; i <= N - 1; i++)
			{
				z(i) = x(i, j);
			}
			F[j][0] = LogLikelihood(z);		                 					// Function values
			F[j][1] = j;											// Original index positions
		}
		sort(F.begin(), F.end());

		// New vertices order first N best initial Arrays and
		// last (N+1)st vertice is the worst Array
		// y is the matrix of vertices, ordered so that the worst vertice is last
		Array<double, 2> y(N, N + 1);
		for (j = 0; j <= N; j++) 
		{
			for (i = 0; i <= N - 1; i++) 
			{
				y(i, j) = x(i, (int)(F[j][1]));
			}
		}

		//  First best Array y(1) and function value f1
		Array<double, 1> x1(N, 0.0); for (i = 0; i <= N - 1; i++) x1(i) = y(i, 0);
		double f1 = LogLikelihood(x1);

		// Last best Array y(N) and function value fn
		Array<double, 1> xn(N, 0.0); for (i = 0; i <= N - 1; i++) xn(i) = y(i, N - 1);
		double fn = LogLikelihood(xn);

		// Worst Array y(N+1) and function value fn1
		Array<double, 1> xn1(N, 0.0); for (i = 0; i <= N - 1; i++) xn1(i) = y(i, N);
		double fn1 = LogLikelihood(xn1);

		// z is the first N Arrays from y, excludes the worst y(N+1)
		Array<double, 2> z(N, N);
		for (j = 0; j <= N - 1; j++) 
		{
			for (i = 0; i <= N - 1; i++)
			{
				z(i, j) = y(i, j);
			}
		}

		// Mean of best N values and function value fm
		Array<double, 1> xm(N, 0.0); xm = stat::mean(z, N);
		double fm = LogLikelihood(xm);

		// Reflection point xr and function fr
		Array<double, 1> xr(N, 0.0); xr = xm + xm - xn1;
		double fr = LogLikelihood(xr);

		// Expansion point xe and function fe
		Array<double, 1> xe(N, 0.0); xe = xr + xr - xm;
		double fe = LogLikelihood(xe);

		// Outside contraction point and function foc
		Array<double, 1> xoc(N, 0.0);	xoc = xr / 2 + xm / 2;
		double foc = LogLikelihood(xoc);

		// Inside contraction point and function foc
		Array<double, 1> xic(N, 0.0);	xic = xm / 2 + xn1 / 2;
		double fic = LogLikelihood(xic);

		while ((NumIters <= MaxIters) && (abs(f1 - fn1) >= Tolerance))
		{
			// Step 1. Reflection Rule
			if ((f1 <= fr) && (fr < fn)) 
			{
				for (j = 0; j <= N - 1; j++)
				{
					for (i = 0; i <= N - 1; i++)  x(i, j) = y(i, j);
				}
				for (i = 0; i <= N - 1; i++)	x(i, N) = xr(i);
				goto step0;
			}

			// Step 2.  Expansion Rule
			if (fr < f1) 
			{
				for (j = 0; j <= N - 1; j++) 
				{
					for (i = 0; i <= N - 1; i++)  x(i, j) = y(i, j);
				}
				if (fe < fr)
					for (i = 0; i <= N - 1; i++)	x(i, N) = xe(i);
				else
					for (i = 0; i <= N - 1; i++)	x(i, N) = xr(i);
				goto step0;
			}

			// Step 3.  Outside contraction Rule
			if ((fn <= fr) && (fr < fn1) && (foc <= fr)) 
			{
				for (j = 0; j <= N - 1; j++) 
				{
					for (i = 0; i <= N - 1; i++)  x(i, j) = y(i, j);
				}
				for (i = 0; i <= N - 1; i++)	x(i, N) = xoc(i);
				goto step0;
			}

			// Step 4.  Inside contraction Rule
			if ((fr >= fn1) && (fic < fn1)) 
			{
				for (j = 0; j <= N - 1; j++)
				{
					for (i = 0; i <= N - 1; i++)  x(i, j) = y(i, j);
				}
				for (i = 0; i <= N - 1; i++)	x(i, N) = xic(i);
				goto step0;
			}

			// Step 5. Shrink Step
			for (i = 0; i <= N - 1; i++)
				x(i, 0) = y(i, 0);
			for (i = 0; i <= N - 1; i++) 
			{
				for (j = 1; j <= N; j++)
					x(i, j) = 0.5*(y(i, j) + x(i, 0));
			}
			goto step0;
		}

		// Output component
		Array<double, 1> out(N + 2);
		for (i = 0; i <= N - 1; i++)
		{
			out(i) = x1(i);
		}
		out(N) = f1;
		out(N + 1) = NumIters;
		return out;
	}

	// Log Likelihood for GARCH(1,1)
	// Returns the negative Log Likelihood, for minimization
	// Variance equation is
	// h(t) = B(0) + B(1)*ret2(i+1) + B(2)*h(i+1);
	// where B(0)=omega, B(1)=alpha, B(2)=beta
	double GARCH::LogLikelihood(Array<double, 1> B)
	{
		Array<double, 1> Price = GetAssetValues();
		int n = Price.size();
		Array<double, 1> ret(n - 1);					// Return
		Array<double, 1> ret2(n - 1);					// Return squared
		Array<double, 1> GARCH(n - 1, 0.0);				// GARCH(1,1) variance
		Array<double, 1> LogLike(n - 1, 0.0);
		// Penalty for non-permissible parameter values
		if ((B(0) < 0.0) || (B(1) < 0.0) || (B(2) < 0.0) || (B(1) + B(2) >= 1))
		{
			return 1e100;
		}
		else
		{
			// Construct the log likelihood
			for (int i = 0; i <= n - 2; i++)
			{
				ret(i) = log(Price(i) / Price(i + 1));
				ret2(i) = ret(i) * ret(i);
			}
		}
		GARCH(n - 2) = stat::variance(ret);
		LogLike(n - 2) = -log(GARCH(n - 2)) - ret2(n - 2) / GARCH(n - 2);
		for (int i = n - 3; i >= 0; i--) 
		{
			GARCH(i) = B(0) + B(1) * ret2(i + 1) + B(2) * GARCH(i + 1);
			LogLike(i) = -log(GARCH(i)) - ret2(i) / GARCH(i);
		}
		return -sum(LogLike);
	}
}