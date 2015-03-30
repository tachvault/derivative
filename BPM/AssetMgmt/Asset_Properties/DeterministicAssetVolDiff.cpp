/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#include "DeterministicAssetVolDiff.hpp"
#include "DeterministicVolMediator.hpp"
#include "QFArrayUtil.hpp"

namespace derivative
{
	std::shared_ptr<DeterministicAssetVol> DeterministicAssetVolDiff::component_vol(int i) const
	{
		std::shared_ptr<DeterministicAssetVol> result = \
			std::make_shared<DeterministicAssetVolDiff>(v1.component_vol(i),v2.component_vol(i));
		return result;
	}

	/// clone this object
    std::shared_ptr<DeterministicAssetVol> DeterministicAssetVolDiff::Clone()
	{
		std::shared_ptr<DeterministicAssetVol> obj =  std::make_shared<DeterministicAssetVolDiff>(v1, v2);
		return obj;
	}

	Array<double,1> DeterministicAssetVolDiff::segments(double t,double dt) const
	{
		Array<double,1> seg_v1 = v1.segments(t,dt);
		Array<double,1> seg_v2 = v2.segments(t,dt);
		Array<double,1> result = unique_merge(seg_v1,seg_v2);
		return result;
	}

	bool DeterministicAssetVolDiff::get_volatility_level(double t,double T,Array<double,1>& vol_lvl) const
	{
		Array<double,1> tmp(vol_lvl.extent(firstDim));
		bool result = v1.get_volatility_level(t,T,vol_lvl);
		result = result && v2.get_volatility_level(t,T,tmp);
		vol_lvl -= tmp;
		return result;
	}

	double DeterministicAssetVolDiff::volproduct(double t,double dt,const DeterministicAssetVol& xv) const
	{
		return DeterministicVolMediator::volproduct_DeterministicAssetVolDiff(t,dt,xv,v1,v2);
	}

	Array<double,1> DeterministicAssetVolDiff::integral(double t,double dt) const
	{
		Array<double,1> result(v1.integral(t,dt)-v2.integral(t,dt));
		return result;
	}

	int DeterministicAssetVolDiff::factors() const
	{
		return v1.factors();
	}

	double DeterministicAssetVolDiff::FwdBondVol(double t,double T1,double T2) const
	{
		throw std::logic_error("DeterministicAssetVolDiff::FwdBondVol not implemented");
	}

	/// Integral over the scalar product between the bond volatility given by (*this) and a deterministic asset volatility.
	double DeterministicAssetVolDiff::bondvolproduct(double t,double dt,double bondmat,const DeterministicAssetVol& xv) const
	{
		return v1.bondvolproduct(t,dt,bondmat,xv)-v2.bondvolproduct(t,dt,bondmat,xv);
	}

	/** Integral over the scalar product between the bond volatility given by (*this) and a
	bond volatility given by another DeterministicAssetVol object. */
	double DeterministicAssetVolDiff::bondbondvolproduct(double t,double dt,double T1,double T2,const DeterministicAssetVol& xv) const
	{
		return v1.bondbondvolproduct(t,dt,T1,T2,xv)-v2.bondbondvolproduct(t,dt,T1,T2,xv);
	}

	/// Function needed for the exponential-affine representation of zero coupon bond prices.
	double DeterministicAssetVolDiff::A(double t,double T) const
	{
		throw std::logic_error("DeterministicAssetVolDiff::A() not implemented");
	}

	/// Function needed for the exponential-affine representation of zero coupon bond prices.
	Array<double,1> DeterministicAssetVolDiff::B(double t,double T) const
	{
		throw std::logic_error("DeterministicAssetVolDiff::B() not implemented");
	}

	/// For Monte Carlo simulation: variance of increment from t to dt of state variable i.
	double DeterministicAssetVolDiff::var(int i,double t,double dt) const
	{
		throw std::logic_error("DeterministicAssetVolDiff::var() not implemented");
		return 0.0;
	}

	/// For Monte Carlo simulation: covariance of increment from t to dt of state variable i with increment of Brownian motion \f$ \Delta W^{(i)} \f$.
	double DeterministicAssetVolDiff::covar_dW(int i,double t,double dt) const
	{
		throw std::logic_error("DeterministicAssetVolDiff::covar_dW() not implemented");
		return 0.0;
	}

	Array<double,1> DeterministicAssetVolDiff::bondvolintegral(double t,double dt,double T) const
	{
		Array<double,1> result(1);
		throw std::logic_error("DeterministicAssetVolDiff::bondvolintegral() not implemented");
		return result;
	}

	Array<double,1> DeterministicAssetVolDiff::StateVariableMean(double t,double dt,double T) const
	{
		Array<double,1> result(1);
		throw std::logic_error("DeterministicAssetVolDiff::StateVariableMean() not implemented");
		return result;
	}	
}