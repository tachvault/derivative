/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#include "DeterministicVolMediator.hpp"

namespace derivative
{
	double DeterministicVolMediator::volproduct_DeterministicAssetVolDiff(double t,double dt,
		const DeterministicAssetVol& xv,
		const DeterministicAssetVol& v1,
		const DeterministicAssetVol& v2)
	{
		double result = xv.volproduct(t,dt,v1)-xv.volproduct(t,dt,v2);
		return result;
	}

	double DeterministicVolMediator::volproduct_ConstVol(double t,double dt,const Array<double,1>& lvl,const DeterministicAssetVol& v2)
	{
		return blitz::sum(lvl*v2.integral(t,dt));
	}

	double DeterministicVolMediator::volproduct_ConstVol(double t, double dt, const Array<double, 1>& lvl, \
		const DeterministicAssetVol& v2, Array<double, 1>& temp)
	{
		v2.integral(t, dt, temp);
		return blitz::sum(lvl*temp);
	}

	double DeterministicVolMediator::volproduct_ExponentialVol(double t,double dt,const Array<double,1>& lvl,const Array<double,1>& decay,const ExponentialVol& v1,const DeterministicAssetVol& v2)
	{
		switch (v2.type()) {
		case FLAT: // reverse calculation
			return v2.volproduct(t,dt,v1);
		case EXPONENTIAL:
			return ((ExponentialVol&)v2).volproduct_ExponentialVol(t,dt,lvl,decay);
		default:
			throw std::logic_error("Unable to calculate vol product in DeterministicVolMediator::volproduct_ExponentialVol");
		}
	}

	// ConstVol refers to the bond
	double DeterministicVolMediator::bondvolproduct_ConstVol(double t,double dt,double bondmat,const Array<double,1>& lvl,const DeterministicAssetVol& xv)
	{
		int i,j;
		double result = 0.0;
		Array<double,1> segments = xv.segments(t,dt);
		switch (xv.type()) {
		case FLAT:
			for (i=0; i<segments.extent(firstDim)-1; i++) {
				double          dti = segments(i+1) - segments(i);
				Array<double,1> sgm(xv.integral(segments(i),dti)/dti);
				double          erg = -blitz::sum(sgm*lvl);
				result += erg * (bondmat*dti-0.5*(segments(i+1)*segments(i+1)-segments(i)*segments(i)));
			}
			return result;
		case EXPONENTIAL:
			for (i=0; i<segments.extent(firstDim)-1; i++) {
				double          dti = segments(i+1) - segments(i);
				const ExponentialVol& v = dynamic_cast<const ExponentialVol&>(xv);
				Array<double,1> sgm(lvl.copy());
				for (j=0; j<sgm.extent(firstDim); j++) {
					sgm(j) *= v.volatility_level(j,segments(i),segments(i+1)) / v.mean_reversion(j);
					sgm(j) *= std::exp(v.mean_reversion(j)*segments(i));
					sgm(j) *= std::exp(v.mean_reversion(j)*dti)*(segments(i+1)-bondmat-1.0/v.mean_reversion(j)) - (segments(i)-bondmat-1.0/v.mean_reversion(j));
				}
				result += blitz::sum(sgm);
			}
			return result;
		default:
			throw std::logic_error("Unable to calculate vol product in DeterministicVolMediator::bondvolproduct_ConstVol");
		}
	}

	double DeterministicVolMediator::bondbondvolproduct_ConstVol(double t,double dt,double T1,double T2,const Array<double,1>& lvl,const ConstVol& v1,const DeterministicAssetVol& xv)
	{
		int i;
		double result = 0.0;
		Array<double,1> segments = xv.segments(t,dt);
		switch (xv.type()) {
		case FLAT:
			for (i=0; i<segments.extent(firstDim)-1; i++) {
				double          dti = segments(i+1) - segments(i);
				Array<double,1> sgm(xv.integral(segments(i),dti)/dti);
				double          erg = blitz::sum(sgm*lvl);
				result += erg * (T1*T2*dti - (T1+T2)*0.5*(segments(i+1)*segments(i+1)-segments(i)*segments(i)) + (segments(i+1)*segments(i+1)*segments(i+1)-segments(i)*segments(i)*segments(i))/3.0);
			}
			return result;
		case EXPONENTIAL: // reverse calculation
			return xv.bondbondvolproduct(t,dt,T2,T1,v1);
		default:
			throw std::logic_error("Unable to calculate vol product in DeterministicVolMediator::bondbondvolproduct_ConstVol");
		}
	}

	double DeterministicVolMediator::bondvolproduct_ExponentialVol(double t,double dt,double bondmat,const Array<double,1>& lvl,const Array<double,1>& decay,const ExponentialVol& this_vol,const DeterministicAssetVol& xv)
	{
		int i,j;
		double result = 0.0;
		Array<double,1> segments = xv.segments(t,dt);
		switch (xv.type()) {
		case FLAT:
			for (i=0; i<segments.extent(firstDim)-1; i++) {
				double          dti = segments(i+1) - segments(i);
				Array<double,1> sgm(xv.integral(segments(i),dti)/dti);
				for (j=0; j<lvl.extent(firstDim); j++) {
					double a = decay(j);
					result += sgm(j)*lvl(j)/a * (exp(-a*(bondmat-segments(i)))/a*(exp(a*dti)-1.0)-dti);
				}
			}
			return result;
		case EXPONENTIAL: // implemented 15 Feb 2011
			return (this_vol.bondvolproduct_ExponentialVol(t,dt,bondmat,(ExponentialVol&)xv));
		default:
			throw std::logic_error("Unable to calculate vol product in DeterministicVolMediator::bondvolproduct_ExponentialVol");
		}
	}

	double DeterministicVolMediator::bondbondvolproduct_ExponentialVol(double t,double dt,double T2,double T1,const Array<double,1>& lvl,const Array<double,1>& decay,const ExponentialVol& this_vol,const DeterministicAssetVol& xv)
		// note the way the arguments are permutated
	{
		int i,j;
		double result = 0.0;
		Array<double,1> segments = xv.segments(t,dt);
		switch (xv.type()) {
		case FLAT:
			for (i=0; i<segments.extent(firstDim)-1; i++) {
				double          dti = segments(i+1) - segments(i);
				double          tci = 0.5*(segments(i+1)*segments(i+1) - segments(i)*segments(i));
				Array<double,1> sgm(xv.integral(segments(i),dti)/dti);
				for (j=0; j<lvl.extent(firstDim); j++) {
					double a   = decay(j);
					double tmp = T1 * (exp(-a*(T2-segments(i)))*(exp(a*dti)-1.0)/a - dti);
					tmp -= exp(-a*(T2-segments(i))) * (segments(i+1)*exp(a*dti) - segments(i) - (exp(a*dti)-1.0)/a) / a;
					result += sgm(j)*lvl(j)/a * (tmp+tci);
				}
			}
			return -result;
		case EXPONENTIAL:
			return ((ExponentialVol&)xv).bondbondvolproduct_ExponentialVol(t,dt,T2,T1,this_vol);
		default:
			throw std::logic_error("Unable to calculate vol product in DeterministicVolMediator::bondbondvolproduct_ExponentialVol");
		}
	}

	Array<double,1> DeterministicVolMediator::z_bondintegral_ExponentialVol(double t,double dt,double T,const Array<double,1>& lvl,const Array<double,1>& decay,const ExponentialVol& v1,const DeterministicAssetVol& xv)
	{
		switch (xv.type()) {
		case EXPONENTIAL:
			return v1.z_bondintegral_ExponentialVol(t,dt,T,((ExponentialVol&)xv));
		case FLAT: // not implemented yet
		default:
			throw std::logic_error("Unable to calculate vol product in DeterministicVolMediator::bondvolproduct_ExponentialVol");
		}
	}

	Array<double,1> DeterministicVolMediator::z_volintegral_ExponentialVol(double t,double dt,double T,const Array<double,1>& lvl,const Array<double,1>& decay,const ExponentialVol& v1,const DeterministicAssetVol& fxvol)
	{
		int j;
		double tdt = t + dt;
		Array<double,1> sgm(fxvol.factors());
		if (!fxvol.get_volatility_level(t,tdt,sgm)) throw std::logic_error("Unable to calculate DeterministicVolMediator::z_volintegral_ExponentialVol");
		Array<double,1> result(sgm*lvl/decay);
		switch (fxvol.type()) {
		case FLAT:
			for (j=0; j<result.extent(firstDim); j++) result(j) *= std::exp(decay(j)*tdt)-std::exp(decay(j)*t);
			return result;
		default:
			throw std::logic_error("Unable to calculate DeterministicVolMediator::z_volintegral_ExponentialVol");
		}
	}

	double DeterministicVolMediator::FwdFXexponential_ExponentialVol(double t,double dt,const Array<double,1>& dWj,const Array<double,1>& dZj0,const Array<double,1>& dZjk,const Array<double,1>& nu,const Array<double,1>& a,\
		const ExponentialVol* this_vol,const std::shared_ptr<DeterministicAssetVol>& xv, const std::shared_ptr<DeterministicAssetVol>& fxvol)
	{
		int j;
		double tdt = t + dt;
		Array<double,1> sgm(fxvol->factors());
		if ((fxvol->type()!=FLAT)||(xv->type()!=EXPONENTIAL)||(!fxvol->get_volatility_level(t,tdt,sgm))) 
		{
			throw std::logic_error("Unable to calculate DeterministicVolMediator::FwdFXexponential_ExponentialVol");
		}
		const ExponentialVol& fvol = *(std::dynamic_pointer_cast<ExponentialVol>(xv));
		double result = 0.0;
		for (j=0; j<dWj.extent(firstDim); j++) {
			double nu_tilde = fvol.volatility_level(j,t,tdt);
			double a_tilde  = fvol.mean_reversion(j);
			result += sgm(j) * dWj(j);
			result += (std::exp(-a_tilde*tdt)*dZjk(j) - nu_tilde*dWj(j))/a_tilde;
			result -= (std::exp(-a(j)*tdt)*dZj0(j) - nu(j)*dWj(j))/a(j);
		}
		// this part is type()-independent
		result -= 0.5*fxvol->volproduct(t,dt,*fxvol);
		result -= fvol.bondvolproduct(t,dt,tdt,*fxvol);
		result += this_vol->bondvolproduct(t,dt,tdt,*fxvol);
		result -= 0.5*fvol.bondbondvolproduct(t,dt,tdt,tdt,fvol);
		result += fvol.bondbondvolproduct(t,dt,tdt,tdt,*this_vol);
		result -= 0.5*this_vol->bondbondvolproduct(t,dt,tdt,tdt,*this_vol);
		return std::exp(result);
	}

	double DeterministicVolMediator::covar_ExponentialVol(int j,double t,double dt,double lvl,double decay,const ExponentialVol& this_vol,const DeterministicAssetVol& xv)
	{
		double tdt = t + dt;
		if (xv.type()!=EXPONENTIAL) throw std::logic_error("Unable to calculate DeterministicVolMediator::FwdFXexponential_ExponentialVol");
		ExponentialVol& fvol = ((ExponentialVol&)xv);
		double nu_tilde = fvol.volatility_level(j,t,tdt);
		double a_tilde  = fvol.mean_reversion(j);
		return nu_tilde*lvl/(a_tilde+decay) * (std::exp((a_tilde+decay)*tdt)-std::exp((a_tilde+decay)*t));
	}
}
