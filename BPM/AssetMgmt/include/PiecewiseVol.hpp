/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#pragma once
#ifndef _DERIVATIVE_PIECEWISEVOL_H_
#define _DERIVATIVE_PIECEWISEVOL_H_

#include "DeterministicVol.hpp"

#if defined _WIN32 || defined __CYGWIN__
#ifdef ASSET_PROPERTIES_EXPORTS
#ifdef __GNUC__
#define ASSET_PROPERTIES_API __attribute__ ((dllexport))
#else
#define ASSET_PROPERTIES_API __declspec(dllexport)
#endif
#else
#ifdef __GNUC__
#define ASSET_PROPERTIES_API __attribute__ ((dllimport))
#else
#define ASSET_PROPERTIES_API __declspec(dllimport)
#endif
#endif
#define ASSET_PROPERTIES_LOCAL
#else
#if __GNUC__ >= 4
#define ASSET_PROPERTIES_API __attribute__ ((visibility ("default")))
#define ASSET_PROPERTIES_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define ASSET_PROPERTIES_API
#define ASSET_PROPERTIES_LOCAL
#endif
#endif

namespace derivative
{
	class ASSET_PROPERTIES_API PiecewiseConstVol : public DeterministicAssetVol
	{
	public:

		enum {TYPEID = CLASS_PIECEWISECONSTVOL_TYPE};

		/// Constructor.
		PiecewiseConstVol(const Array<double,1>& xT, ///< Time line defining calendar time segments on which parameters are constant.
			const Array<double,2>& xv  ///< Matrix of volatility scale parameters. Dimensions: time segment (row) by driving factor (column).
			);

		/// clone this object
		virtual std::shared_ptr<DeterministicAssetVol>  Clone() const;

		virtual ~PiecewiseConstVol()
		{
		}

		std::shared_ptr<DeterministicAssetVol> component_vol(int i) const;

		/// Get constant volatility levels over time interval [t,T]. Returns false if volatility is not constant over this interval.
		virtual bool get_volatility_level(double t,double T,Array<double,1>& vol_lvl) const;

		/// The integral over the scalar product between two volatility vectors.
		virtual double volproduct(double t,double dt,const DeterministicAssetVol& xv) const;

		virtual Array<double,1> integral(double t,double dt) const;

		virtual void interpolate(const std::shared_ptr<DeterministicAssetVol>& neibor, double factor);

		/// Dimension of the volatility vector.
		virtual int factors() const;

		virtual double timeframe() const;

		/// Division of time period into segments over which volatility is constant.
		virtual Array<double,1> segments(double t,double dt) const;

		virtual double FwdBondVol(double t,double T1,double T2) const;

		/// Integral over the scalar product between the bond volatility given by (*this) and a deterministic asset volatility.
		virtual double bondvolproduct(double t,double dt,double bondmat,const DeterministicAssetVol& xv) const;

		/** Integral over the scalar product between the bond volatility given by (*this) and a
		bond volatility given by another DeterministicVol object. */
		virtual double bondbondvolproduct(double t,double dt,double T1,double T2,const DeterministicAssetVol& xv) const;

		/// Function needed for the exponential-affine representation of zero coupon bond prices.
		virtual double A(double t,double T) const;

		/// Function needed for the exponential-affine representation of zero coupon bond prices.
		virtual Array<double,1> B(double t,double T) const;

		/// For Monte Carlo simulation: variance of increment from t to dt of state variable i.
		virtual double var(int i,double t,double dt) const;

		/// For Monte Carlo simulation: covariance of increment from t to dt of state variable i with increment of Brownian motion \f$ \Delta W^{(i)} \f$.
		virtual double covar_dW(int i,double t,double dt) const;

		/** Integral over zero coupon bond volatility.

		\f[ \int_{t}^{t+\Delta t}\sigma^*(s,T)ds \f] */
		virtual Array<double,1> bondvolintegral(double t,double dt,double T) const;

		/// Expected value of the state variable increments between t and t+dt under the time T forward measure.
		virtual Array<double,1> StateVariableMean(double t,double dt,double T) const;

		virtual void output() const
		{
			cout << "Timeline " << timeline << endl;
			cout << " Vol " << v << endl;
		}

	private:
		///< Time line defining calendar time segments on which parameters are constant.
		Array<double,1> timeline; 

		///< Matrix of volatility scale parameters. Dimensions: time segment (row) by driving factor (column).
		Array<double,2> v;

		///< Scalar products of the volatility scale parameter vector for each time segment with itself.
		Array<double,1> vol_sq;

		/// disallow the copy constructor and operator= functions
		DISALLOW_COPY_AND_ASSIGN(PiecewiseConstVol);
	};

} /* namespace derivative */

#endif /* _DERIVATIVE_PIECEWISEVOL_H_ */


