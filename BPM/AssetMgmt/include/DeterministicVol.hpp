/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_DETERMINISTICVOL_H_
#define _DERIVATIVE_DETERMINISTICVOL_H_

#include <blitz/array.h>
#include <memory>
#include "ClassType.hpp"
#include "Global.hpp"

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
	using blitz::Array;
	using blitz::firstDim;
	using blitz::secondDim;

	/** Abstract base class which defines additional functionality for deterministic volatility functions.
	*
	* It is used in particular for asset price processes modelled as geometric Brownian motion.
	*/
	class ASSET_PROPERTIES_API DeterministicAssetVol
	{
	public:

		enum { TYPEID = CLASS_DETERMINISTICASSETVOL_TYPE};

		/// define a virtual destructor
		virtual ~DeterministicAssetVol()
		{}

		/// Returns the corresponding volatility of a state variable in a Gaussian HJM model
		/// where the forward rate volatility is given by *this.
		virtual std::shared_ptr<DeterministicAssetVol> component_vol(int i) const = 0;

		/// clone this object
		virtual std::shared_ptr<DeterministicAssetVol>  Clone() = 0;

		virtual void output() const
		{}

		/// The integral over the scalar product between two volatility vectors.
		/// the return value is sigma^2.dt.
		virtual double volproduct(double t,double dt,const DeterministicAssetVol& xv) const = 0;

		virtual Array<double,1> integral(double t,double dt) const = 0;

		/// Dimension of the volatility vector.
		virtual int factors() const = 0;

		virtual int type() const;

		/// Division of time period into segments over which volatility is constant.
		virtual Array<double,1> segments(double t,double dt) const;

		/// Get constant volatility levels over time interval [t,T]. Returns false if volatility is not constant over this interval.
		virtual bool get_volatility_level(double t,double T,Array<double,1>& vol_lvl) const;

		virtual void interpolate(const std::shared_ptr<DeterministicAssetVol>& neibor, double factor)
		{
			throw std::logic_error("not supported");
		}

		/** Integral over the square of the forward zero coupon bond volatility.
		* The value of the integral \f[ \sqrt{\int_t^{T_1}(\sigma^*(u,T_2)-\sigma^*(u,T_1))^2du} \f] is
		* calculated in classes derived from this abstract base class.
		*/
		virtual double FwdBondVol(double t,double T1,double T2) const = 0;

		/// Integral over the scalar product between the bond volatility given by (*this) and a deterministic asset volatility.
		virtual double bondvolproduct(double t,double dt,double bondmat,const DeterministicAssetVol& xv) const = 0;

		/** Integral over the scalar product between the bond volatility given by (*this) and a
		bond volatility given by another DeterministicVol object. */
		virtual double bondbondvolproduct(double t,double dt,double T1,double T2,const DeterministicAssetVol& xv) const = 0;

		/// Function needed for the exponential-affine representation of zero coupon bond prices.
		virtual double A(double t,double T) const = 0;

		/// Function needed for the exponential-affine representation of zero coupon bond prices.
		virtual Array<double,1> B(double t,double T) const = 0;

		/// For Monte Carlo simulation: variance of increment from t to dt of state variable i.
		virtual double var(int i,double t,double dt) const = 0;

		/// For Monte Carlo simulation: covariance of increment from t to dt of state variable i with increment of Brownian motion \f$ \Delta W^{(i)} \f$.
		virtual double covar_dW(int i,double t,double dt) const = 0;

		/** For Monte Carlo simulation: Covariance between the j-th component of the state variable increment and the j-th
		component of the state variable increment for the DeterministicAssetVol xv */
		virtual double covar(int j,double t,double dt,const DeterministicAssetVol& xv) const;

		/** Integral over zero coupon bond volatility.

		\f[ \int_{t}^{t+\Delta t}\sigma^*(s,T)ds \f] */
		virtual Array<double,1> bondvolintegral(double t,double dt,double T) const = 0;

		/// Expected value of the state variable increments between t and t+dt under the time T forward measure.
		virtual Array<double,1> StateVariableMean(double t,double dt,double T) const = 0;

		virtual double bondvolexponential(double t,double dt,double T,const Array<double,1>& dZ,const Array<double,1>& dW) const;

		virtual double z_integral(int i,double t,double dt) const;

		virtual Array<double,1> z_bondintegral(double t,double dt,double T,const DeterministicAssetVol& xmodel) const;

		virtual Array<double,1> z_volintegral(double t,double dt,double T,const DeterministicAssetVol& fxvol) const;

		virtual double FwdFXexponential(double t,double dt,const Array<double,1>& dWj,const Array<double,1>& dZj0,const Array<double,1>& dZjk, \
			const std::shared_ptr<DeterministicAssetVol>& xv, const std::shared_ptr<DeterministicAssetVol>& fxvol) const;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_DETERMINISTICVOL_H_ */


