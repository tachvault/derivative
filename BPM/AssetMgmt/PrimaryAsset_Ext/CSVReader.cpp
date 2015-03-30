/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#include "CSV2Array.hpp"
#include "ExponentialVol.hpp"
#include "PiecewiseVol.hpp"
#include "ConstVol.hpp"
#include "FlatTermStructure.hpp"
#include "CSVReader.hpp"

namespace derivative
{
	std::shared_ptr<TermStructure> CSVreadtermstructure(const char* path)
	{
		int i;
		std::ifstream is_inputs(path);
		if (!is_inputs.is_open()) throw std::logic_error("Failed to open input file in CSVreadtermstructure()");
		blitz::Array<std::string,2> inputs_matrix(CSV2Array<std::string>(is_inputs,make_string_strip_spaces));
		std::map<std::string,std::string> inputs_map;
		for (i=0;i<inputs_matrix.rows();i++) inputs_map[inputs_matrix(i,0)] = inputs_matrix(i,1);
		if (!inputs_map.count("Term structure type")) throw std::logic_error("Missing term structure type");
		std::shared_ptr<TermStructure> result;
		if (inputs_map["Term structure type"]=="flat") 
		{
			double lvl;
			std::string str("Interest rate level");
			if (inputs_map.count(str)) 
			{
				double start = 0.0;
				double end   = 40.0;
				lvl = std::atof(inputs_map[str].data());
				str = "Start time";
				if (inputs_map.count(str)) start = std::atof(inputs_map[str].data());
				str = "End time";
				if (inputs_map.count(str)) end = std::atof(inputs_map[str].data());
				result = std::make_shared<FlatTermStructure>(lvl,start,end);
				return result; 
			}
			else throw std::logic_error("Missing interest rate level specification"); 
		}
		else
		{
			throw std::logic_error("Only Flat Term structure is supported"); 
		}
	}

	std::shared_ptr<DeterministicAssetVol> CSVreadvolatility(const char* path)
	{
		int i;
		std::ifstream is_inputs(path);
		std::shared_ptr<DeterministicAssetVol> vol;
		if (!is_inputs.is_open()) throw std::logic_error("Failed to open input file in CSVreadvolatility()");
		blitz::Array<std::string,2> inputs_matrix(CSV2Array<std::string>(is_inputs,make_string_strip_spaces));
		std::map<std::string,std::string> inputs_map;
		for (i=0;i<inputs_matrix.rows();i++) inputs_map[inputs_matrix(i,0)] = inputs_matrix(i,1);
		int voldim = 0;
		if (inputs_map.count("Volatility dimension")) voldim = std::atoi(inputs_map["Volatility dimension"].data());
		if (voldim<1) throw std::logic_error("Misspecified volatility dimension");
		if (!inputs_map.count("Volatility function type")) throw std::logic_error("Missing volatility function type");
		Array<double,1> vollvl(voldim);
		if (inputs_map["Volatility function type"]=="constant")
		{
			for (i=0;i<voldim;i++) 
			{
				std::string str("Volatility level ");
				str += (char)(48+i);
				if (inputs_map.count(str)) vollvl(i) = std::atof(inputs_map[str].data());
				else throw std::logic_error("Missing volatility level specification"); 
			}
			vol = std::make_shared<ConstVol>(vollvl);
			return vol; 
		}
		if (inputs_map["Volatility function type"]=="exponential") 
		{
			Array<double,1> mr(voldim);
			for (i=0;i<voldim;i++) 
			{
				std::string str("Volatility level ");
				str += (char)(48+i);
				if (inputs_map.count(str)) vollvl(i) = std::atof(inputs_map[str].data());
				else throw std::logic_error("Missing volatility level specification"); 
				std::string str1("Mean reversion speed ");
				str1 += (char)(48+i);
				if (inputs_map.count(str1)) mr(i) = std::atof(inputs_map[str1].data());
				else throw std::logic_error("Missing mean reversion speed specification"); 
			}
			vol = std::make_shared<ExponentialVol>(vollvl,mr);
			return vol; 
		}
		throw std::logic_error("Unsupported volatility function type");
	}
}