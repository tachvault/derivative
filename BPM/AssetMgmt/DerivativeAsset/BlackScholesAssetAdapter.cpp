/*
Copyright (C) Nathan Muruganantha 2013 - 2014
*/

#include "BlackScholesAssetAdapter.hpp"
#include "IAssetValue.hpp"
#include "CSV2Array.hpp"
#include "CSVReader.hpp"
#include "PrimaryAssetUtil.hpp"
#include "IStockValue.hpp"
#include "IAssetValue.hpp"
#include "ConstVol.hpp"
#include "IAsset.hpp"

namespace derivative
{
	BlackScholesAssetAdapter::BlackScholesAssetAdapter(const std::shared_ptr<IAssetValue>& val)
		:m_asset(val)
	{
		std::shared_ptr<DeterministicAssetVol>  xv = std::make_shared<ConstVol>(val->GetAsset()->GetImpliedVol());
		m_backScholesAsset = std::unique_ptr<BlackScholesAsset>(new BlackScholesAsset(xv, \
			val->GetTradePrice(), m_asset->GetDivYield()));
	}

	BlackScholesAssetAdapter::BlackScholesAssetAdapter(const std::shared_ptr<IAssetValue>& val, \
		std::shared_ptr<DeterministicAssetVol> xv)
		:m_asset(val)
	{
		m_backScholesAsset = std::unique_ptr<BlackScholesAsset>(new BlackScholesAsset(xv, \
			val->GetTradePrice(), m_asset->GetDivYield()));
	}
	
	/// constructor for constructing asset value, deterministic
	/// volatility and dividend yield
	BlackScholesAssetAdapter::BlackScholesAssetAdapter(const std::shared_ptr<IAssetValue> &val,
			std::shared_ptr<DeterministicAssetVol> xv, ///< Volatility function.
			std::shared_ptr<TermStructure> div
			)
	{
		m_backScholesAsset = std::unique_ptr<BlackScholesAsset>(new BlackScholesAsset(xv, val->GetTradePrice(), div));
	}

	BlackScholesAssetAdapter::BlackScholesAssetAdapter(const BlackScholesAssetAdapter& xasset) 		 
	{
		*m_backScholesAsset = *xasset.m_backScholesAsset;
		*m_asset = *xasset.m_asset;
	};

	BlackScholesAssetAdapter& BlackScholesAssetAdapter::operator=(const BlackScholesAssetAdapter& xasset)
	{
		if (&xasset == this)
		{
			return *this;
		}

		*m_backScholesAsset = *xasset.m_backScholesAsset;
		*m_asset = *xasset.m_asset;

		return *this;
	}

	BlackScholesAssetAdapter::BlackScholesAssetAdapter(const char* path)
	{
		int i = 0;
		std::shared_ptr<DeterministicAssetVol> v;                    
		double                 xzero;                ///< Initial ("time zero") value.
		std::shared_ptr<TermStructure>  dividend;  ///< Term structure of dividend yields.

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

		/// Assign the asset value and create BlackScholesAsset.
		m_asset = PrimaryUtil::getStockValue(path);
		dynamic_pointer_cast<IStockValue>(m_asset)->SetTradePrice(xzero);
		m_backScholesAsset = std::unique_ptr<BlackScholesAsset>(new BlackScholesAsset(v, xzero, dividend));
	}
}
