/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#include "DeterministicVol.hpp"
#include "DeterministicVolMediator.hpp"
#include "QFArrayUtil.hpp"

namespace derivative
{	
	/// clone this object
    std::shared_ptr<DeterministicAssetVol> ConstVol::Clone()
	{
		std::shared_ptr<DeterministicAssetVol> obj = std::make_shared<ConstVol>(std::forward<Array<double,1> >(lvl));
		return obj;
	}
	
	double ConstVol::volproduct(double t,double dt,const DeterministicAssetVol& xv) const
	{
		return DeterministicVolMediator::volproduct_ConstVol(t,dt,lvl,xv);
	}

	Array<double,1> ConstVol::integral(double t,double dt) const
	{
		Array<double,1> result(dt*lvl);
		return result;
	}

	std::shared_ptr<DeterministicAssetVol> ConstVol::component_vol(int i) const
	{
		Array<double,1> clvl(lvl.extent(firstDim));
		clvl = 0.0;
		clvl(i) = lvl(i);
		std::shared_ptr<DeterministicAssetVol> result = std::make_shared<ConstVol>(clvl);
		return result;
	}

	int ConstVol::factors() const
	{
		return lvl.extent(firstDim);
	}

	/** Integral over the square of the forward zero coupon bond volatility.
	*
	* In the case of a constant volatility vector v, the value of the
	* integral \f[ \sqrt{\int_t^{T_1}(\sigma^*(u,T_2)-\sigma^*(u,T_1))^2du} \f] is given
	* by \f[ \sqrt{v^2(T_2-T_1)^2(T_1-t)} \f]
	*/
	double ConstVol::FwdBondVol(double t,double T1,double T2) const
	{
		return sqrt(blitz::sum(lvl*lvl) * (T1-T2) * (T1-T2) * (T1-t));
	}

	/** Integral over the scalar product between the bond volatility given by (*this) and a deterministic asset volatility.

	Here this is
	\f[ \int_t^{t+\Delta t}-\sigma_B\cdot(T-s)\cdot\sigma_Xds=-\sigma_B\sigma_X\left(T\Delta t-\frac12((t+\Delta t)^2-t^2)\right) \f]
	*/
	double ConstVol::bondvolproduct(double t,double dt,double bondmat,const DeterministicAssetVol& xv) const
	{
		return DeterministicVolMediator::bondvolproduct_ConstVol(t,dt,bondmat,lvl,xv);
	}

	/** Integral over the scalar product between the bond volatility given by (*this) and a
	bond volatility given by another DeterministicAssetVol object. */
	double ConstVol::bondbondvolproduct(double t,double dt,double T1,double T2,const DeterministicAssetVol& xv) const
	{
		return DeterministicVolMediator::bondbondvolproduct_ConstVol(t,dt,T1,T2,lvl,*this,xv);
	}

	/** Function needed for the exponential-affine representation of zero coupon bond prices.
	*
	* Zero coupon bond prices can be expressed as an exponential-affine function of the
	* state variables, i.e. as \f[ B(t,T) = \frac{B(0,T)}{B(0,t)}{\mathcal{A}}(t,T)\exp\{-{\mathcal{B}}(t,T)Z(t)\} \f]
	* This is the function \f$ {\mathcal{A}}(t,T) \f$, which for the case of a constant
	* volatility vector v is given by \f[ \exp\left\{-\frac12v^2Tt(T-t)\right\} \f]
	*/
	double ConstVol::A(double t,double T) const
	{
		return exp(-0.5*blitz::sum(lvl*lvl)*T*t*(T-t));
	}

	/** Function needed for the exponential-affine representation of zero coupon bond prices.
	*
	* Zero coupon bond prices can be expressed as an exponential-affine function of the
	* state variables, i.e. as \f[ B(t,T) = \frac{B(0,T)}{B(0,t)}{\mathcal{A}}(t,T)\exp\{-{\mathcal{B}}(t,T)Z(t)\} \f]
	* This is the function \f$ {\mathcal{B}}(t,T) \f$, which for the case of a constant
	* volatility vector v is given by \f$ v(T-t). \f$
	*/
	Array<double,1> ConstVol::B(double t,double T) const
	{
		Array<double,1> result(lvl.extent(firstDim));
		result = T-t;
		return result;
	}

	/// For Monte Carlo simulation: variance of increment from t to dt of state variable i.
	double ConstVol::var(int i,double t,double dt) const
	{
		return lvl(i)*lvl(i)*dt;
	}

	/// For Monte Carlo simulation: covariance of increment from t to dt of state variable i with increment of Brownian motion \f$ \Delta W^{(i)} \f$.
	double ConstVol::covar_dW(int i,double t,double dt) const
	{
		return lvl(i)*dt;
	}

	Array<double,1> ConstVol::bondvolintegral(double t,double dt,double T) const
	{
		Array<double,1> result(lvl.copy());
		double tdt = t+dt;
		result *= 0.5*(tdt*tdt-t*t) - T*dt;
		return result;
	}

	Array<double,1> ConstVol::StateVariableMean(double t,double dt,double T) const
	{
		Array<double,1> result(lvl*bondvolintegral(t,dt,T));
		return result;
	}

	bool ConstVol::get_volatility_level(double t,double T,Array<double,1>& vol_lvl) const
	{
		if (lvl.extent(firstDim)!=vol_lvl.extent(firstDim)) throw std::logic_error("Volatility dimension mismatch");
		vol_lvl = lvl;
		return true;
	}

	double ConstVol::z_integral(int i,double t,double dt) const
	{
		return lvl(i)*dt;
	}

	Array<double,1> ConstVol::z_bondintegral(double t,double dt,double T,const DeterministicAssetVol& xmodel) const
	{
		Array<double,1> result(lvl*xmodel.bondvolintegral(t,dt,T));
		return result;
	}

	Array<double,1> ConstVol::z_volintegral(double t,double dt,double T,const DeterministicAssetVol& fxvol) const
	{
		Array<double,1> result(lvl*fxvol.integral(t,dt));
		return result;
	}	
}

