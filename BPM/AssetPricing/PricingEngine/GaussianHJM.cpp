/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#include "MSWarnings.hpp"
#include <boost/bind.hpp>
#include "GaussianHJM.hpp"
#include "GaussianQuadrature.hpp"
#include "Rootsearch.hpp"
#include "Constants.hpp"
#include "ConstVol.hpp"

namespace derivative
{

	/** Calculate a zero coupon bond price given the state variables.

	For a given realisation Z(t) of the <b>state variables</b> under the continuous rolling spot measure,
	calculate the zero coupon bond price at time t with time to maturity ttm. In the Gauss/Markov
	HJM model the zero coupon bond prices can be written as exponential-affine function of the
	state variables, i.e. as
	\f[ B(t,T) = \frac{B(0,T)}{B(0,t)}{\mathcal{A}}(t,T)\exp\{-{\mathcal{B}}(t,T)Z(t)\} \f]
	where the functions \f$ {\mathcal{A}}(t,T) \f$ and \f$ {\mathcal{B}}(t,T) \f$ depend on the 
	choice of volatility function and are therefore passed through to DeterministicVol::A()
	and DeterministicVol::B(), respectively.
	*/
	double GaussianHJM::bond(const Array<double,1> &W,double t,double ttm) const
	{
		return initialTS->operator()(t+ttm)/initialTS->operator()(t) * v->A(t,t+ttm) * exp(-blitz::sum(v->B(t,t+ttm)*W));
	}

	/** \brief Calculate the ("time zero") value of a zero coupon bond option.

	In the Gaussian HJM model, the "time zero" prices of a call and a put option on a zero coupon bond are
	given by the Black/Scholes-type formula
	\f[ C=B(0,T_2)N(h_1)-B(0,T_1)KN(h_2) \f] and
	\f[ P=B(0,T_1)KN(-h_2)-B(0,T_2)N(-h_1) \f]
	with
	\f[ h_{1,2}=\frac{\ln\frac{B(0,T_2)}{B(0,T_1)K}\pm\frac12\int_0^{T_1}(\sigma^*(u,T_2)-\sigma^*(u,T_1))^2du}{\sqrt{\int_0^{T_1}(\sigma^*(u,T_2)-\sigma^*(u,T_1))^2du}}\f]
	The value of the integral \f[ \int_0^{T_1}(\sigma^*(u,T_2)-\sigma^*(u,T_1))^2du \f]
	depends on the choice of \f$ \sigma(u,s) \f$ and is therefore passed through to
	DeterministicAssetVol::FwdBondVol(). \sa DeterministicAssetVol::FwdBondVol()
	*/
	double GaussianHJM::ZCBoption(double T1,  ///< Option expiry
		double T2,  ///< Maturity of the underlying bond
		double K,   ///< Exercise price
		int sign    ///< 1 if call option (default), -1 if put option
		) const
	{
		double vol = v->FwdBondVol(0.0,T1,T2);
		double B1  = initialTS->operator()(T1);
		double B2  = initialTS->operator()(T2);
		double h1  = (log(B2/(K*B1)) + 0.5*vol*vol) / vol;
		return sign * (B2*boost::math::cdf(N,sign*h1)-K*B1*boost::math::cdf(N,sign*(h1-vol)));
	}

	/** Calculate the ("time zero") value of a coupon bond option. One-factor case implemented according to equation (3.44)
	in Brigo/Mercurio (2001). Coupon is assumed to be actual payment per $1 notional. */
	double GaussianHJM::CBoption(const Array<double,1>& tenor,const Array<double,1>& coupon,double K,int sign) const
	{
		int i;
		double strike;
		if (factors()>2) throw std::logic_error("Coupon bond option formula not implemented for more than two factors");
		double result = 0.0;
		if (factors()==1) {
			r_star rstar(*this,tenor,coupon);
			Rootsearch<r_star,double,double> rs(rstar,K,0.05,0.02);
			double z = rs.solve();
			double discount = initialTS->operator()(tenor(0));
			for (i=1;i<tenor.extent(firstDim)-1;i++) {
				strike = blitz::sum(initialTS->operator()(tenor(i)) * v->A(tenor(0),tenor(i)) * exp(-v->B(tenor(0),tenor(i))*z))/discount;
				result += coupon(i-1) * ZCBoption(tenor(0),tenor(i),strike,sign); }
			strike = blitz::sum(initialTS->operator()(tenor(i)) * v->A(tenor(0),tenor(i)) * exp(-v->B(tenor(0),tenor(i))*z))/discount;
			result += (1.0+coupon(coupon.extent(firstDim)-1)) * ZCBoption(tenor(0),tenor(i),strike,sign); }
		else {  // two factors
			cb_option cbopt(*this,tenor,coupon,K,sign);
			GaussHermite gh(GH_n);
			std::function<double (double t)> f = std::bind(&GaussianHJM::cb_option::two_factor_CBoption_integrand,&cbopt,std::placeholders::_1);
			result = initialTS->operator()(tenor(0)) * gh.integrate(f); }
		return result;
	}

	GaussianHJM::cb_option::cb_option(const GaussianHJM& xmodel,const Array<double,1>& xtenor,const Array<double,1>& xcoupon,double strike,int xsign) 
		: model(xmodel),tenor(xtenor),coupon(xcoupon.copy()),tmpB(2),K(strike),sign(xsign)
	{ 
		Array<double,1> ez(2);
		ez = model.StateVariableMean(0.0,tenor(0),tenor(0));
		e0 = ez(0);
		e1 = ez(1);
		v0 = model.var(0,0.0,tenor(0)); 
		v1 = model.var(1,0.0,tenor(0)); 
		coupon(coupon.extent(firstDim)-1) += 1.0;   
	}

	double GaussianHJM::cb_option::fix_point_problem(double z2)
	{
		int i;
		double result = 0.0;
		for (i=0;i<coupon.extent(firstDim);i++) {
			tmpB = model.v->B(tenor(0),tenor(i+1));
			result += coupon(i) * model.v->A(tenor(0),tenor(i+1)) * model.initialTS->operator()(tenor(i+1))/model.initialTS->operator()(tenor(0)) * std::exp(-tmpB(0)*current_z1-tmpB(1)*z2); }
		return result;
	}

	double GaussianHJM::cb_option::two_factor_CBoption_integrand(double z) 
	{
		int i;
		double tmp;
		// z argument is assumed to be normalised
		current_z1 = std::sqrt(v0) * z + e0;
		std::function<double (double t)> f = boost::bind(std::mem_fun(&GaussianHJM::cb_option::fix_point_problem),this,_1);
		Rootsearch<std::function<double (double)>,double,double> rs(f,K,0.0,0.4);
		double zstar = rs.solve();
		double d = (zstar-e1)/std::sqrt(v1);
		double result = 0.0;
		for (i=0;i<coupon.extent(firstDim);i++) {
			tmpB = model.v->B(tenor(0),tenor(i+1));
			tmp  = coupon(i) * model.v->A(tenor(0),tenor(i+1)) * model.initialTS->operator()(tenor(i+1))/model.initialTS->operator()(tenor(0));
			double tmpexp = -current_z1*tmpB(0);
			tmpexp += -e1*tmpB(1) + 0.5*tmpB(1)*tmpB(1)*v1;
			tmp *= std::exp(tmpexp) * boost::math::cdf(model.N,sign*(d+tmpB(1)*std::sqrt(v1)));
			result += tmp; }
		result = sign*(result - K*boost::math::cdf(model.N,sign*d)); 
		result *= RSQRT2PI * std::exp(-0.5*z*z);
		return result;
	}

	/// Function for rootsearch required to implement coupon bond pricing in the single factor model.
	double GaussianHJM::r_star::operator()(double r) const
	{
		int i;
		double result = 0.0;
		for (i=1;i<tenor.extent(firstDim)-1;i++) result += coupon(i-1) * blitz::sum(model.initialTS->operator()(tenor(i)) * model.v->A(tenor(0),tenor(i)) * exp(-model.v->B(tenor(0),tenor(i))*r));
		result += (1.0+coupon(coupon.extent(firstDim)-1)) * (model.initialTS->operator()(tenor(tenor.extent(firstDim)-1)) * model.v->A(tenor(0),tenor(tenor.extent(firstDim)-1)) * exp(-blitz::sum(model.v->B(tenor(0),tenor(tenor.extent(firstDim)-1)))*r));
		return result / model.initialTS->operator()(tenor(0));
	}

	/// Calculate the ("time zero") value of swaption.
	double GaussianHJM::swaption(const Array<double,1>& tenor,  ///< Swap tenor.
		double K,                      ///< Strike (fixed side) level.
		int sign                       ///< 1 = receiver swaption, -1 = payer swaption.
		) const
	{
		int i;
		Array<double,1> coupon(tenor.extent(firstDim)-1);
		for (i=0;i<coupon.extent(firstDim);i++) coupon(i) = (tenor(i+1)-tenor(i)) * K;
		return CBoption(tenor,coupon,1.0,sign);
	}

	/// Calculate the ("time zero") value of a caplet.
	double GaussianHJM::caplet(double mat,double delta,double K) const
	{
		return (1.0+delta*K) * ZCBoption(mat,mat+delta,1.0/(1.0+delta*K),-1);
	}

	/** Calculate the ("time zero") value of an FX option.

	If the spot exchange rate follows Black/Scholes-type dynamics (e.g. constant volatility)
	and domestic and foreign interest rate dynamics are given by Gaussian HJM models, then the 
	"time zero" prices of a call and a put option on FX can be calculated by the Black/Scholes-type formula
	\f[ C=X(0)\tilde B(0,T_1)N(h_1)-B(0,T_1)KN(h_2) \f] and
	\f[ P=B(0,T_1)KN(-h_2)-X(0)\tilde B(0,T_1)N(-h_1) \f]
	with
	\f[ h_{1,2}=\frac{\ln\frac{X(0)\tilde B(0,T_1)}{B(0,T_1)K}\pm\frac12\int_0^{T_1}(\sigma_S+\tilde\sigma_B(u,T_1)-\sigma_B(u,T_1))^2du}{\sqrt{\int_0^{T_1}(\sigma_S+\tilde\sigma_B(u,T_1)-\sigma_B(u,T_1))^2du}}\f]
	\f$ \sigma_S+\tilde\sigma_B(u,T_1)-\sigma_B(u,T_1) \f$ is the volatility of the quotient
	\f[ \frac{X(t)\tilde B(t,T_1)}{B(t,T_1)} \f] and the value of the integral over the square
	of this volatility depends on the choice of volatility functions and is therefore calculated
	using member functions of the volatility objects.
	*/
	double GaussianHJM::FXoption(double X0,double T1,double K,const GaussianHJM& fmodel,const DeterministicAssetVol& fxvol,int sign) const
	{
		double B   = initialTS->operator()(T1);
		double fB  = fmodel.initialTS->operator()(T1);
		double var = fxvol.volproduct(0.0,T1,fxvol);
		var += v->bondbondvolproduct(0.0,T1,T1,T1,*v);
		var += fmodel.v->bondbondvolproduct(0.0,T1,T1,T1,*(fmodel.v));
		var -= 2.0 * v->bondvolproduct(0.0,T1,T1,fxvol);
		var += 2.0 * fmodel.v->bondvolproduct(0.0,T1,T1,fxvol);
		var -= 2.0 * v->bondbondvolproduct(0.0,T1,T1,T1,*(fmodel.v));
		double sd  = sqrt(var);
		double h1  = (log(X0*fB/(K*B)) + 0.5*var) / sd;
		return sign * (X0*fB*boost::math::cdf(N,sign*h1)-K*B*boost::math::cdf(N,sign*(h1-sd)));
	}

	/// Swap netting the difference between the foreign and domestic floating rates (reverse if sign = -1), paid on a domestic notional.
	double GaussianHJM::DiffSwap(const Array<double,1>& T,const GaussianHJM& fmodel,const DeterministicAssetVol& fxvol,int sign) const
	{
		int i;
		double result = 0.0;
		for (i=0;i<T.extent(firstDim)-1;i++) {
			double delta          = T(i+1)-T(i);
			double domestic_LIBOR = initialTS->simple_rate(T(i),delta);
			double discount       = initialTS->operator()(T(i+1));
			double fwdbondvol     = fmodel.v->FwdBondVol(0.0,T(i),T(i+1));
			double var            = v->bondbondvolproduct(0.0,T(i),T(i+1),T(i),*(fmodel.v));
			var -= v->bondbondvolproduct(0.0,T(i),T(i+1),T(i+1),*(fmodel.v));
			var += fmodel.v->bondbondvolproduct(0.0,T(i),T(i),T(i+1),*(fmodel.v));
			var -= fmodel.v->bondbondvolproduct(0.0,T(i),T(i),T(i),*(fmodel.v));
			var += fmodel.v->bondvolproduct(0.0,T(i),T(i+1),fxvol);
			var -= fmodel.v->bondvolproduct(0.0,T(i),T(i),fxvol);
			// Calculate the expectation of the reciprocal of the foreign zero coupon bond under the appropriate domestic forward measure.
			double expectation = fmodel.initialTS->operator()(T(i))/fmodel.initialTS->operator()(T(i+1));
			expectation *= exp(fwdbondvol*fwdbondvol+var);
			result += discount * (expectation - (1.0+delta*domestic_LIBOR)); }
		return result;
	}

	/// Foreign caplet with payoff converted to domestic currency at a guaranteed exchange rate \e X0.
	double GaussianHJM::QuantoCaplet_old(double X0,double T,double delta,double lvl,const GaussianHJM& fmodel,const DeterministicAssetVol& fxvol) const
	{
		double discount        = initialTS->operator()(T+delta);
		double fwdbondvol      = fmodel.v->FwdBondVol(0.0,T,T+delta);
		double exphalffwdvolsq = exp(0.5*fwdbondvol*fwdbondvol);
		double Ctilde          = (fmodel.initialTS->operator()(T+delta)/fmodel.initialTS->operator()(T)) / exphalffwdvolsq;
		double var             = v->bondbondvolproduct(0.0,T,T+delta,T,*(fmodel.v));
		var -= v->bondbondvolproduct(0.0,T,T+delta,T+delta,*(fmodel.v));
		var += fmodel.v->bondbondvolproduct(0.0,T,T,T+delta,*(fmodel.v));
		var -= fmodel.v->bondbondvolproduct(0.0,T,T,T,*(fmodel.v));
		var += fmodel.v->bondvolproduct(0.0,T,T+delta,fxvol);
		var -= fmodel.v->bondvolproduct(0.0,T,T,fxvol);
		double h2 = (log((1.0+delta*lvl)*Ctilde) - var) / fwdbondvol;
		double h1 = h2 - fwdbondvol;
		double expectation = exp(var)*exphalffwdvolsq/Ctilde * boost::math::cdf(N,-h1);
		return discount*X0 * (expectation - (1.0+delta*lvl)*boost::math::cdf(N,-h2));
	}

	/// Foreign caplet with payoff converted to domestic currency at a guaranteed exchange rate \e X0.
	double GaussianHJM::QuantoCaplet(double X0,double T,double delta,double lvl,const GaussianHJM& fmodel,const DeterministicAssetVol& fxvol) const
	{
		double discount = initialTS->operator()(T+delta);
		double var = fmodel.v->bondbondvolproduct(0.0,T,T,T,*(fmodel.v));
		var -= 2.0*fmodel.v->bondbondvolproduct(0.0,T,T,T+delta,*(fmodel.v));
		var += fmodel.v->bondbondvolproduct(0.0,T,T+delta,T+delta,*(fmodel.v));
		double sgm = std::sqrt(var/T);
		double quantoexp = -fmodel.v->bondbondvolproduct(0.0,T,T,T+delta,*(fmodel.v));
		quantoexp += fmodel.v->bondbondvolproduct(0.0,T,T,T+delta,*(v));
		quantoexp -= fmodel.v->bondvolproduct(0.0,T,T,fxvol);
		quantoexp += fmodel.v->bondbondvolproduct(0.0,T,T+delta,T+delta,*(fmodel.v));
		quantoexp -= fmodel.v->bondbondvolproduct(0.0,T,T+delta,T+delta,*(v));
		quantoexp += fmodel.v->bondvolproduct(0.0,T,T+delta,fxvol);
		quantoexp = std::exp(quantoexp);
		std::shared_ptr<DeterministicAssetVol> tmpvol = std::make_shared<ConstVol>(sgm);
		BlackScholesAsset tmpS(tmpvol,(fmodel.initialTS->operator()(T)/fmodel.initialTS->operator()(T+delta)));
		double K = (1.0+delta*lvl)/quantoexp;
		return discount*X0*quantoexp*tmpS.option(T,K,0.0);
	}

	/** Calculate the ("time zero") value of an equity option. Implemented by creating an "artificial"
	BlackScholesAsset with constant volatility sgm and continuously compounded interest rate r. */
	double GaussianHJM::option(const BlackScholesAsset& S,double mat,double K,int sign) const
	{
		const DeterministicAssetVol& svol = S.volatility_function();
		double r   = -std::log(initialTS->operator()(mat))/mat;
		double var = svol.volproduct(0.0,mat,svol);
		var -= 2.0 * v->bondvolproduct(0.0,mat,mat,svol);
		var += v->bondbondvolproduct(0.0,mat,mat,mat,*v);
		double sgm = std::sqrt(var/mat);
		std::shared_ptr<DeterministicAssetVol> tmpvol = std::make_shared<ConstVol>(sgm);
		BlackScholesAsset tmpS(std::move(tmpvol),S.initial_value(),S.dividend_yield(0.0,mat));
		return tmpS.option(mat,K,r,sign);
	}

	/** Expected value of the state variable increments between t and t+dt under the time T forward measure
	associated with *xmodel, where the volatility of the spot exchange rate (of units of the *xmodel currency
	per unit of *this currency) is fxvol. */
	Array<double,1> GaussianHJM::StateVariableMean(double t,double dt,double T,const GaussianHJM* xmodel,const DeterministicAssetVol& fxvol) const
	{
		Array<double,1> result(v->z_bondintegral(t,dt,T,*(xmodel->v)) - v->z_volintegral(t,dt,T,fxvol));
		return result;          
	}	
}

