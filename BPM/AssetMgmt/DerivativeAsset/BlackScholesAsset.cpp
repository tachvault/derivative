/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#include <stdexcept>
#include <functional>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <fstream>

#include "BlackScholesAsset.hpp"
#include "Rootsearch.hpp"
#include "DeterministicAssetVolDiff.hpp"
#include "CSV2Array.hpp"
#include "CSVReader.hpp"


namespace derivative
{
	BlackScholesAsset::BlackScholesAsset(std::shared_ptr<DeterministicAssetVol> xv, double ini,double div) 
		: v(xv),xzero(ini) 
	{
		dividend = std::make_shared<FlatTermStructure>(div,0.0,100.0);
	};		

	/// Constructor.
	BlackScholesAsset::BlackScholesAsset(std::shared_ptr<DeterministicAssetVol> xv, double ini,	\
		std::shared_ptr<TermStructure> div)
		: v(xv),xzero(ini), dividend(div)
	{};

	BlackScholesAsset::BlackScholesAsset(const BlackScholesAsset& xasset) 
		: xzero(xasset.xzero) 
	{
		v = xasset.v->Clone();
		dividend = xasset.dividend->pointer_to_copy();
	};

	BlackScholesAsset& BlackScholesAsset::operator=(const BlackScholesAsset& xasset)
	{
		/// prevent self assignment
		if (&xasset == this)
		{
			return *this;
		}

		/// clone the members and move the cloned members.
		/// Note: we don't move xaaset members.
		v= xasset.v->Clone();
		dividend = xasset.dividend->pointer_to_copy();

		return *this;
	}

	/// Price a European call or put option (call is default).
	double BlackScholesAsset::option(double mat,                ///< Time to maturity of the option.
		double K,                  ///< Exercise price.
		double r,                  ///< Deterministic interest rate.
		int sign,                  ///< Call = 1 (default), put = -1.
		double today
		) const
	{
		double vol = v->volproduct(today,mat,*v);

		return genericBlackScholes(xzero*(*dividend)(today+mat)/(*dividend)(today),K*exp(-mat*r),vol,sign);
	}

	/// Delta of a European call or put option (call is default).
	double BlackScholesAsset::delta(double mat,                ///< Maturity of the option.
		double K,                  ///< Exercise price.
		double r,                  ///< Deterministic interest rate.
		int sign                   ///< Call = 1 (default), put = -1.
		) const
	{
		double vol = v->volproduct(0.0,mat,*v);
		double sd  = std::sqrt(vol);
		double h1  = (std::log((xzero*(*dividend)(mat)/(*dividend)(0.0))/(K*exp(-mat*r))) + 0.5*vol) / sd;
		return sign*(*dividend)(mat)*boost::math::cdf(N,sign*h1);
	}

	/// Gamma of a European call or put option (call is default).
	double BlackScholesAsset::gamma(double mat,                ///< Maturity of the option.
		double K,                  ///< Exercise price.
		double r,                  ///< Deterministic interest rate.
		int sign                   ///< Call = 1 (default), put = -1.
		) const
	{
		double vol = v->volproduct(0.0,mat,*v);
		double sd  = std::sqrt(vol);
		double h1  = (std::log((xzero*(*dividend)(mat)/(*dividend)(0.0))/(K*exp(-mat*r))) + 0.5*vol) / sd;
		return (*dividend)(mat)*boost::math::pdf(N,h1)/(xzero*sd);
	}

	/// Vega of a European call or put option (call is default).
	double BlackScholesAsset::vega(double mat,                ///< Maturity of the option.
		double K,                  ///< Exercise price.
		double r,                  ///< Deterministic interest rate.
		int sign                   ///< Call = 1 (default), put = -1.
		) const
	{
		double vol = v->volproduct(0.0, mat, *v);
		double sd = std::sqrt(vol);
		double h1 = (std::log((xzero*(*dividend)(mat) / (*dividend)(0.0)) / (K*exp(-mat*r))) + 0.5*vol) / sd;
		return (*dividend)(mat)*boost::math::pdf(N, h1)*std::sqrt(mat);
	}

	double BlackScholesAsset::Margrabe(const BlackScholesAsset& S,double mat,double K, int sign) const
	{
		DeterministicAssetVolDiff voldiff(*v,*(S.v));
		double vol = voldiff.volproduct(0.0,mat,voldiff);      
		return genericBlackScholes(xzero*(*dividend)(mat)/(*dividend)(0.0),K*S.xzero*exp(-mat*S.dividend_yield(0.0,mat)),vol,sign);
	}

	double BlackScholesAsset::DoleansExp(double t,double T,const Array<double,1>& dW) const
	{
		Array<double,1> vol_lvl(dW.extent(firstDim));
		if (!v->get_volatility_level(t,T,vol_lvl)) throw std::logic_error("Volatility not constant in BlackScholesAsset::DoleansExp");
		return std::exp(blitz::sum(dW*vol_lvl)-0.5*v->volproduct(t,T-t,*v));
	}

	/// Calculate the implied volatility for a given price.
	double BlackScholesAsset::implied_volatility(double price,double mat,double K,double r,int sign) const
	{
		boost::function<double (double)> objective_function = boost::bind(&GenericBlackScholes::operator(),&genericBlackScholes,(double)(xzero*(*dividend)(mat)/(*dividend)(0.0)),(double)(K*exp(-mat*r)),_1,sign);
		Rootsearch<boost::function<double (double)>,double,double> rs(objective_function,price,0.3,0.2,1e-12);
		double vol = rs.solve();

		/// proceed only if the resultant value is valid
		if (vol > DBL_MAX || vol < -DBL_MAX) throw std::out_of_range("implied volatility is out of range");
		return sqrt(vol/mat);
	}

	BlackScholesAsset::BlackScholesAsset(const char* path)
	{
		int i;
		std::ifstream is_inputs(path);
		if (!is_inputs.is_open()) throw std::logic_error("Failed to open input file in BlackScholesAsset constructor");
		blitz::Array<std::string,2> inputs_matrix(CSV2Array<std::string>(is_inputs,make_string_strip_spaces));
		std::map<std::string,std::string> inputs_map;
		for (i=0;i<inputs_matrix.rows();i++) inputs_map[inputs_matrix(i,0)] = inputs_matrix(i,1);
		if (inputs_map.count("CSV file for volatility")) v = CSVreadvolatility(inputs_map["CSV file for volatility"].data());
		else throw std::logic_error("Missing volatility specification");
		std::string str("Initial value");
		if (inputs_map.count(str)) xzero = std::atof(inputs_map[str].data());
		else throw std::logic_error("Missing initial value for BlackScholesAsset"); 
		if (inputs_map.count("CSV file for dividend yield term structure")) 
			dividend = CSVreadtermstructure(inputs_map["CSV file for dividend yield term structure"].data());
		else dividend = std::make_shared<FlatTermStructure>(0.0,0.0,100.0);
	}
}
