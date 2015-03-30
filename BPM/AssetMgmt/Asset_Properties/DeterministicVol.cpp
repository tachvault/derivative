/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#include "DeterministicVol.hpp"
#include "DeterministicVolMediator.hpp"
#include "QFArrayUtil.hpp"

namespace derivative
{
	int DeterministicAssetVol::type() const
	{
		return DeterministicVolMediator::FLAT;
	}

	Array<double,1> DeterministicAssetVol::segments(double t,double dt) const
	{
		Array<double,1> result(2);
		result(0) = t;
		result(1) = t + dt;
		return result;
	}

	bool DeterministicAssetVol::get_volatility_level(double t,double T,Array<double,1>& vol_lvl) const
	{
		return false;
	}

	double DeterministicAssetVol::z_integral(int i,double t,double dt) const
	{
		throw std::logic_error("z_integral not implemented");
		return 0.0;
	}

	Array<double,1> DeterministicAssetVol::z_bondintegral(double t,double dt,double T,const DeterministicAssetVol& xmodel) const
	{
		throw std::logic_error("z_bondintegral not implemented");
		Array<double,1> result(1);
		return result;
	}

	Array<double,1> DeterministicAssetVol::z_volintegral(double t,double dt,double T,const DeterministicAssetVol& fxvol) const
	{
		throw std::logic_error("z_volintegral not implemented");
		Array<double,1> result(1);
		return result;
	}

	double DeterministicAssetVol::bondvolexponential(double t,double dt,double T,const Array<double,1>& dZ,const Array<double,1>& dW) const
	{
		throw std::logic_error("bondvolexponential not implemented");
		return 0.0;
	}

	double DeterministicAssetVol::FwdFXexponential(double t,double dt,const Array<double,1>& dWj,const Array<double,1>& dZj0,const Array<double,1>& dZjk,\
		const std::shared_ptr<DeterministicAssetVol>& xv, const std::shared_ptr<DeterministicAssetVol>& fxvol) const
	{
		throw std::logic_error("FwdFXexponential not implemented");
		return 0.0;
	}

	double DeterministicAssetVol::covar(int j,double t,double dt,const DeterministicAssetVol& xv) const
	{
		throw std::logic_error("covar() not implemented");
		return 0.0;
	}
}

