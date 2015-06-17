/*
Copyright (C) Nathan Muruganantha 2015
Initial version: Copyright 2008, 2015 by Erik Schlögl
*/

#include <complex>
#include <boost/bind.hpp>
#include "HestonAsset.hpp"
#include "Constants.hpp"

using std::complex;

namespace derivative
{
	HestonAsset::HestonAsset(CIRprocess& xvol_process, double ini, double xrho, double xlambda, int xn)
		: vol_process(xvol_process), xzero(ini), rho(xrho), lambda(xlambda), n(xn), gausslaguerre(xn)
	{
		kappa = vol_process.get_kappa();
		theta = vol_process.get_theta();
		sigma = vol_process.get_sigma_level();
	}

	HestonAsset_as_BlackScholesAsset::HestonAsset_as_BlackScholesAsset(const HestonAsset& heston_asset)
		: BlackScholesAsset(NULL, heston_asset.initial_value()), vol_lvl(2)
	{
		double rho = heston_asset.get_rho();
		double sgm = heston_asset.get_initial_volatility();
		vol_lvl(0) = rho*sgm;
		vol_lvl(1) = std::sqrt(1.0 - rho*rho)*sgm;
		const_vol.reset(new ConstVol(vol_lvl));
		set_volatility(const_vol);
	}

	HestonAsset_as_BlackScholesAsset::~HestonAsset_as_BlackScholesAsset()
	{
	}

	double HestonAsset::option(double mat, double K, double r, int sign) const
	{
		double discount = std::exp(-mat*r);
		double fwd = xzero / discount;
		double x = std::log(fwd / K);
		boost::function<double(double t)> f0 = boost::bind(&HestonAsset::P_Gatheral, this, _1, 0, mat, x, r);
		boost::function<double(double t)> f1 = boost::bind(&HestonAsset::P_Gatheral, this, _1, 1, mat, x, r);
		double P1 = gausslaguerre.integrate(f1) / PI + 0.5;
		double P0 = gausslaguerre.integrate(f0) / PI + 0.5;
		double result = discount*(fwd*P1 - K*P0);
		if (sign == -1) result -= xzero - K*discount;
		return result;
	}

	double HestonAsset::P_Gatheral(double phi, int j, double mat, double x, double r) const
	{
		complex<double> iphi(0.0, phi);
		complex<double> alpha(0.0, j*phi - 0.5*phi);
		alpha -= 0.5*phi*phi;
		complex<double> beta = kappa - j*rho*sigma - iphi*rho*sigma;
		double gamma = 0.5*sigma*sigma;
		complex<double> d = std::sqrt(beta*beta - 4.0*alpha*gamma);
		complex<double> rplus = (beta + d) / (sigma*sigma);
		complex<double> rminus = (beta - d) / (sigma*sigma);
		complex<double> g = rminus / rplus;
		complex<double> D = rminus * (1.0 - std::exp(-d*mat)) / (1.0 - g*std::exp(-d*mat));
		complex<double> C = kappa * (rminus*mat - std::log((1.0 - g*std::exp(-d*mat)) / (1.0 - g)) / gamma);
		complex<double> f = C*theta + D*vol_process.get_initial();
		complex<double> result = std::exp(iphi*x + f) / iphi;
		return result.real();
	}
}

