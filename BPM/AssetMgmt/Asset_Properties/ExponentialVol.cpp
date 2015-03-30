/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#include "ExponentialVol.hpp"
#include "DeterministicVolMediator.hpp"

namespace derivative
{

	/// clone this object
    std::shared_ptr<DeterministicAssetVol> ExponentialVol::Clone()
	{
		std::shared_ptr<DeterministicAssetVol> obj = \
			std::make_shared<ExponentialVol>(std::forward<Array<double,1> >(lvl), std::forward<Array<double,1> >(decay));
		return obj;
	}

	/// Returns the corresponding volatility of a state variable in a Gaussian HJM model where the forward rate volatility is given by *this - note that the sign of the mean reversion coefficient changes in this case.
	std::shared_ptr<DeterministicAssetVol> ExponentialVol::component_vol(int i) const
	{
		Array<double,1> clvl(lvl.extent(firstDim)),cdecay(decay.extent(firstDim));
		clvl = 0.0;
		clvl(i) = lvl(i);
		cdecay = decay;
		std::shared_ptr<DeterministicAssetVol> result = std::make_shared<ExponentialVol>(clvl,cdecay);
		return result;
	}

	double ExponentialVol::volproduct(double t,double dt,const DeterministicAssetVol& xv) const
	{
		return DeterministicVolMediator::volproduct_ExponentialVol(t,dt,lvl,decay,*this,xv);
	}

	Array<double,1> ExponentialVol::integral(double t,double dt) const
	{
		Array<double,1> result(lvl/decay * exp(decay*t) * (exp(decay*dt)-1.0));
		return result;
	}

	int ExponentialVol::factors() const
	{
		return lvl.extent(firstDim);
	}

	int ExponentialVol::type() const
	{
		return DeterministicVolMediator::EXPONENTIAL;
	}

	double ExponentialVol::volproduct_ExponentialVol(double t,double dt,const Array<double,1>& xlvl,const Array<double,1>& xdecay) const
	{
		Array<double,1> nlvl(xlvl*lvl);
		Array<double,1> ndecay(xdecay+decay);
		ExponentialVol  nvol(nlvl,ndecay);
		return blitz::sum(nvol.integral(t,dt));
	}

	/** Integral over the square of the forward zero coupon bond volatility.
	*
	* In the case of exponentially decaying volatility, the value of the
	* integral \f[ \sqrt{\int_t^{T_1}(\sigma^*(u,T_2)-\sigma^*(u,T_1))^2du} \f] is given
	* by \f[ \sqrt{\sum_{i=1}^d\frac{v_i^2}{a_i^2}\left(\frac1{2a_i}(e^{-2a_i(T_2-T_1)}-e^{-2a_i(T_2-t)}+1-e^{-2a_i(T_1-t)})-\frac1{a_i}e^{-a_i(T_1+T_2)}(e^{2a_iT}-e^{2a_it})\right)} \f]
	*/
	double ExponentialVol::FwdBondVol(double t,double T1,double T2) const
	{
		int i;
		double erg = 0.0;
		for (i=0; i<lvl.extent(firstDim); i++) 
		{
			double a  = decay(i);
			double e1 = exp(-a*T1);
			double e2 = exp(-a*T2);
			double et = exp(-a*t);
			erg += lvl(i)*lvl(i)/(a*a) * ((e2*e2/(e1*e1) - e2*e2/(et*et) + 1.0 - e1*e1/(et*et))/(2.0*a) - e1*e2*(1.0/(e1*e1)-1.0/(et*et))/a);
		}
		return sqrt(erg);
	}

	double ExponentialVol::bondvolproduct(double t,double dt,double bondmat,const DeterministicAssetVol& xv) const
	{
		return DeterministicVolMediator::bondvolproduct_ExponentialVol(t,dt,bondmat,lvl,decay,*this,xv);
	}

	/** Integral over the scalar product between the bond volatility given by (*this) and a
	bond volatility given by another DeterministicAssetVol object. */
	double ExponentialVol::bondbondvolproduct(double t,double dt,double T1,double T2,const DeterministicAssetVol& xv) const
	{
		return DeterministicVolMediator::bondbondvolproduct_ExponentialVol(t,dt,T1,T2,lvl,decay,*this,xv);
	}

	/** Integral over the scalar product between the bond volatility given by (*this) and a
	bond volatility given by another ExponentialVol object. */
	double ExponentialVol::bondbondvolproduct_ExponentialVol(double t,double dt,double T1,double T2,const ExponentialVol& xv) const
	{
		int i;
		double tdt = t+dt;
		double erg = 0.0;
		for (i=0; i<lvl.extent(firstDim); i++)
		{
			double ai  = decay(i);
			double bi  = xv.decay(i);
			double tmp = exp(-ai*T1-bi*T2)/(ai+bi) * exp((ai+bi)*t) * (exp((ai+bi)*dt)-1.0);
			tmp -= exp(-ai*T1)/ai * exp(ai*t) * (exp(ai*dt)-1.0);
			tmp -= exp(-bi*T2)/bi * exp(bi*t) * (exp(bi*dt)-1.0);
			erg += lvl(i)*xv.lvl(i)/(ai*bi) * (tmp+dt);
		}
		return erg;
	}

	/** Integral over the scalar product between the bond volatility given by (*this) and a
	volatility (not bond volatility) given by another ExponentialVol object. */
	double ExponentialVol::bondvolproduct_ExponentialVol(double t,double dt,double T,const ExponentialVol& xv) const
	{
		int i;
		double tdt = t+dt;
		double erg = 0.0;
		for (i=0; i<lvl.extent(firstDim); i++) 
		{
			double ai  = decay(i);
			double bi  = xv.decay(i);
			double tmp = exp(-ai*T)/(ai+bi) * (exp((ai+bi)*(tdt)) - exp((ai+bi)*t));
			tmp -= 1.0/bi * (exp(bi*tdt) - exp(bi*t));
			erg += lvl(i)*xv.lvl(i)/ai * tmp;
		}
		return erg;
	}

	/** Function needed for the exponential-affine representation of zero coupon bond prices.
	*
	* Zero coupon bond prices can be expressed as an exponential-affine function of the
	* state variables, i.e. as \f[ B(t,T) = \frac{B(0,T)}{B(0,t)}{\mathcal{A}}(t,T)\exp\{-{\mathcal{B}}(t,T)Z(t)\} \f]
	* This is the function \f$ {\mathcal{A}}(t,T) \f$, which for the case of exponentially decaying volatility
	* is given by \f[ \exp\left\{\sum_{i=1}^d\frac{v_i^2}{a_i^2}\left(\frac1{a_i}(e^{-a_i(T-t)}-1-e^{-a_iT}+e^{-a_it})-\frac1{4a_i}(e^{-2a_i(T-t)}-1-e^{-2a_iT}+e^{-2a_it})\right)\right\} \f]
	*/
	double ExponentialVol::A(double t,double T) const
	{
		int i;
		double erg = 0.0;
		for (i=0; i<lvl.extent(firstDim); i++)
		{
			double a  = decay(i);
			double eT = exp(-a*T);
			double et = exp(-a*t);
			double ed = eT / et;
			erg += lvl(i)*lvl(i)/(a*a*a) * (ed - 1.0 - eT + et - 0.25 * (ed*ed - 1.0 - eT*eT + et*et));
		}
		return exp(erg);
	}

	/** Function needed for the exponential-affine representation of zero coupon bond prices.
	*
	* Zero coupon bond prices can be expressed as an exponential-affine function of the
	* state variables, i.e. as \f[ B(t,T) = \frac{B(0,T)}{B(0,t)}{\mathcal{A}}(t,T)\exp\{-{\mathcal{B}}(t,T)Z(t)\} \f]
	* This is the function \f$ {\mathcal{B}}(t,T) \f$, which for the case of exponentially decaying volatility
	* is given (component-wise) by \f[ \frac{v_i}{a_i}(e^{-a_it}-e^{-a_iT}) \f]
	*/
	Array<double,1> ExponentialVol::B(double t,double T) const
	{
		Array<double,1> result(1.0/decay * (exp(-decay*t) - exp(-decay*T)));
		return result;
	}

	double ExponentialVol::volatility_level(int i,double T1,double T2) const
	{
		return lvl(i);
	}

	/// For Monte Carlo simulation: variance of increment from t to dt of state variable i.
	double ExponentialVol::var(int i,double t,double dt) const
	{
		throw std::logic_error("Function ExponentialVol::var not defined");
		double a2 = 2.0*decay(i);
		return lvl(i)*lvl(i)/a2 * std::exp(a2*t) * (std::exp(a2*dt) - 1.0);
	}

	/// For Monte Carlo simulation: covariance of increment from t to dt of state variable i with increment of Brownian motion \f$ \Delta W^{(i)} \f$.
	double ExponentialVol::covar_dW(int i,double t,double dt) const
	{
		throw std::logic_error("Function ExponentialVol::covar_dW not defined");
		return lvl(i)/decay(i) * std::exp(decay(i)*t) * (std::exp(decay(i)*dt) - 1.0);
	}

	Array<double,1> ExponentialVol::bondvolintegral(double t,double dt,double T) const
	{
		int i;
		Array<double,1> result(lvl.copy());
		for (i=0; i<result.extent(firstDim); i++) {
			double tmp = std::exp(-decay(i)*(T-t));
			result(i) *= ((std::exp(decay(i)*dt)-1.0)*tmp/decay(i) - dt) / decay(i);
		}
		return result;
	}

	Array<double,1> ExponentialVol::StateVariableMean(double t,double dt,double T) const
	{
		throw std::logic_error("Function ExponentialVol::StateVariableMean not defined");
		int i;
		Array<double,1> result(lvl.copy());
		for (i=0; i<result.extent(firstDim); i++)
		{
			double a = decay(i);
			double eat = std::exp(a*t);
			double eadt = std::exp(a*dt);
			double eaT = std::exp(a*T);
			result(i) *= result(i)/(a*a) * (0.5*eat*eat/eaT*(eadt*eadt-1.0) - eat*(eadt-1.0));
		}
		return result;
	}

	double ExponentialVol::bondvolexponential(double t,double dt,double T,const Array<double,1>& dZ,const Array<double,1>& dW) const
	{
		int j;
		Array<double,1> mu(StateVariableMean(t,dt,T));
		double result = 0.0;
		for (j=0; j<lvl.extent(firstDim); j++)
		{
			double a = decay(j);
			result += (std::exp(-a*T)*(dZ(j)-mu(j)) - lvl(j)*dW(j)) / a;
		}
		return std::exp(result);
	}

	double ExponentialVol::z_integral(int i,double t,double dt) const
	{
		throw std::logic_error("Function ExponentialVol::z_integral not defined");
		return lvl(i)/decay(i)*(std::exp(decay(i)*(t+dt))-std::exp(decay(i)*t));
	}

	Array<double,1> ExponentialVol::z_bondintegral(double t,double dt,double T,const DeterministicAssetVol& xmodel) const
	{
		throw std::logic_error("Function ExponentialVol::z_bondintegral not defined");
		return DeterministicVolMediator::z_bondintegral_ExponentialVol(t,dt,T,lvl,decay,*this,xmodel);
	}

	Array<double,1> ExponentialVol::z_volintegral(double t,double dt,double T,const DeterministicAssetVol& fxvol) const
	{
		throw std::logic_error("Function ExponentialVol::z_volintegral not defined");
		return DeterministicVolMediator::z_volintegral_ExponentialVol(t,dt,T,lvl,decay,*this,fxvol);
	}

	Array<double,1> ExponentialVol::z_bondintegral_ExponentialVol(double t,double dt,double T,const ExponentialVol& bondvol) const
	{
		throw std::logic_error("Function ExponentialVol::z_bondintegral_ExponentialVol not defined");
		int i;
		Array<double,1> result(lvl*bondvol.lvl/bondvol.decay);
		double tdt = t+dt;
		for (i=0; i<result.extent(firstDim); i++) 
		{
			double tmp = std::exp(-bondvol.decay(i)*T+(decay(i)+bondvol.decay(i))*tdt)/(decay(i)+bondvol.decay(i));
			tmp -= std::exp(decay(i)*tdt)/decay(i);
			tmp -= std::exp(-bondvol.decay(i)*T+(decay(i)+bondvol.decay(i))*t)/(decay(i)+bondvol.decay(i));
			tmp += std::exp(decay(i)*t)/decay(i);
			result(i) *= tmp;
		}
		return result;
	}

	double ExponentialVol::FwdFXexponential(double t,double dt,const Array<double,1>& dWj,const Array<double,1>& dZj0,const Array<double,1>& dZjk,\
		const std::shared_ptr<DeterministicAssetVol>& xv, const std::shared_ptr<DeterministicAssetVol>& fxvol) const
	{
		throw std::logic_error("Function ExponentialVol::FwdFXexponential not defined");
		return DeterministicVolMediator::FwdFXexponential_ExponentialVol(t,dt,dWj,dZj0,dZjk,lvl,decay,this,xv,fxvol);
	}

	double ExponentialVol::covar(int j,double t,double dt,const DeterministicAssetVol& xv) const
	{
		throw std::logic_error("Function ExponentialVol::covar not defined");
		return DeterministicVolMediator::covar_ExponentialVol(j,t,dt,lvl(j),decay(j),*this,xv);
	}
}

