/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#include <algorithm>
#include <fstream>
#include <memory>
#include "GaussianEconomy.hpp"
#include "QFArrayUtil.hpp"
#include "CSV2Array.hpp"
#include "CSVReader.hpp"

namespace derivative
{
	GaussianEconomy::GaussianEconomy(const std::vector<std::shared_ptr<BlackScholesAssetAdapter> >& xunderlying,
		std::shared_ptr<DeterministicAssetVol> xv,
		std::shared_ptr<TermStructure> xinitialTS)
		: underlying(xunderlying),v(xv),initialTS(xinitialTS),hjm(new GaussianHJM(xv,xinitialTS))
	{
		int i;
		for (i=0;i<xv->factors();i++) component_vol.push_back(xv->component_vol(i));
	}

	GaussianEconomy::GaussianEconomy(const char* path)
	{
		int i;
		std::ifstream is_inputs(path);
		if (!is_inputs.is_open()) throw std::logic_error("Failed to open input file in GaussianEconomy constructor");
		blitz::Array<std::string,2> inputs_matrix(CSV2Array<std::string>(is_inputs,make_string_strip_spaces));
		std::map<std::string,std::string> inputs_map;
		for (i=0;i<inputs_matrix.rows();i++) inputs_map[inputs_matrix(i,0)] = inputs_matrix(i,1);
		int number_of_underlyings = -1;
		if (inputs_map.count("Number of Black/Scholes assets")) number_of_underlyings = std::atoi(inputs_map["Number of Black/Scholes assets"].data());
		if (number_of_underlyings<0) throw std::logic_error("Misspecified number of Black/Scholes assets");		
		for (i=0;i<number_of_underlyings;i++)
		{
			std::string str("CSV file for Black/Scholes asset ");
			str += (char)(48+i);
			if (inputs_map.count(str)) 
			{
				cout << inputs_map[str].data() << endl;
				std::shared_ptr<BlackScholesAssetAdapter> asset = std::make_shared<BlackScholesAssetAdapter>(inputs_map[str].data());
				underlying.push_back(asset); 
			}
			else throw std::logic_error("Missing Black/Scholes asset specification"); 
		}
		if (inputs_map.count("CSV file for interest rate volatility"))
		{
			v = CSVreadvolatility(inputs_map["CSV file for interest rate volatility"].data());
		}
		else 
		{
			throw std::logic_error("Missing interest rate volatility specification");
		}
		for (i=0;i<v->factors();i++)
		{
			component_vol.push_back(v->component_vol(i));
		}
		if (inputs_map.count("CSV file for initial interest rate term structure")) 
		{
			initialTS = CSVreadtermstructure(inputs_map["CSV file for initial interest rate term structure"].data());
		}
		else
		{
			throw std::logic_error("Missing initial interest rate term structure");
		}
		hjm.reset(new GaussianHJM(v,initialTS));
	}
}
