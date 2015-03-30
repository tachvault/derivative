/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/


#include <stdexcept>
#include <functional>
#include "GramCharlierAsset.hpp"

#undef min
#undef max

using namespace std::placeholders;

namespace derivative
{
	GramCharlierAsset::GramCharlierAsset(const GramCharlierAsset& xasset) 
		:gc( xasset.gc),sgm( xasset.sgm),xzero( xasset.xzero),T( xasset.T),prices_( xasset.prices_) 
	{
	};

	GramCharlierAsset& GramCharlierAsset::operator=(const GramCharlierAsset& xasset)
	{
		/// prevent self assignment
		if (&xasset == this)
		{
			return *this;
		}

		gc = xasset.gc;
		sgm = xasset.sgm;
		xzero = xasset.xzero;
		T = xasset.T;
		prices_ = xasset.prices_;

		return *this;
	}
	
	double GramCharlierAsset::genericBlackScholes(double S1,double S2,double vol,int sign) const
	{
		double sd  = std::sqrt(vol);
		double h1  = (std::log(S1/S2) + 0.5*vol) / sd;
		return sign * (S1*boost::math::cdf(N,sign*h1)-S2*boost::math::cdf(N,sign*(h1-sd)));
	} 

	/// Price a European call option.
	double GramCharlierAsset::call(double K,double domestic_discount,double foreign_discount) const
	{
		int i,j;
		const Array<double,1>& c = gc.coefficients();
		double sgmpowsum = 0.0;
		double sgmpow    = 1.0;
		for (i=0;i<=gc.highest_moment();i++) 
		{
			sgmpowsum += c(i) * sgmpow;
			sgmpow    *= sgm; 
		}
		double fwd = xzero * foreign_discount/domestic_discount;
		double d   = std::log(fwd/(sgmpowsum*K))/sgm + 0.5*sgm;
		Array<double,1> He(std::max(2,gc.highest_moment()-1));
		double x = -d + sgm;
		He(0) = 1;
		He(1) = x;
		for (i=2;i<gc.highest_moment()-1;i++) He(i) = x*He(i-1) - (i-1)*He(i-2);
		double sumterm = 0.0; 
		for (j=2;j<=gc.highest_moment();j++) 
		{
			sgmpow = sgm;
			for (i=1;i<j;i++)
			{
				sumterm += c(j) * sgmpow * He(j-1-i);
				sgmpow  *= sgm;
			}
		}
		double density = RSQRT2PI * std::exp(-d*d/2.0);
		double result = foreign_discount * xzero * boost::math::cdf(N,d);
		result -= domestic_discount * K * boost::math::cdf(N,d-sgm);
		result += xzero/sgmpowsum * density * sumterm; 
		return result;
	}

	/// Best fit calibration to a given set of Black/Scholes implied volatilities.
	double GramCharlierAsset::calibrate(std::shared_ptr<const Array<double,1> > xstrikes, std::shared_ptr<const Array<double,1> > xvols, \
		                                double domestic_discount,double foreign_discount,int highest_moment)
	{
		int i;
		strikes_ = xstrikes;
		int len = strikes_->extent(firstDim);
		if (len>prices_.extent(firstDim)) prices_.resize(len);
		// Forward price of underlying
		double fwd = xzero * foreign_discount/domestic_discount; 
		// Convert implied volatilities to call option prices
		for (i=0;i<len;i++) prices_(i) = domestic_discount * genericBlackScholes(fwd,(*strikes_)(i),(*xvols)(i)*(*xvols)(i)*T,1);
		domestic_ = domestic_discount;
		foreign_  = foreign_discount;
		// Pass objective function to Gram/Charlier calibration routine
		std::function<double (double)> objective_function = std::bind(std::mem_fun(&GramCharlierAsset::calibration_objective_function),this,_1);
		return gc.fit_coefficients(objective_function,(*xvols)(0),highest_moment);
	}

	/** Objective function for best fit calibration to a given set of Black/Scholes implied volatilities. 
	Note that this function updates the volatility parameter sgm of GramCharlierAsset. */
	double GramCharlierAsset::calibration_objective_function(double xsgm)
	{
		int i;
		double result = 0.0;
		sgm = xsgm;
		for (i=0;i<strikes_->extent(firstDim);i++)
		{
			double reldiff = (prices_(i)-call((*strikes_)(i),domestic_,foreign_))/prices_(i);
			result += reldiff*reldiff; 
		}
		return result * 10000.0;
	}
}



