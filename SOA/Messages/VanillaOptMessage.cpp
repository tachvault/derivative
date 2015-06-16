/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include "VanillaOptMessage.hpp"

namespace derivative
{
	std::atomic<long> VanillaOptMessage::g_msgSeq = 0;

	void VanillaOptMessage::ParseSymbol(VanillaOptMessage::Request &req, \
		const std::map<string_t, string_t>& query_strings)
	{
		if (query_strings.find(U("_symbol")) != query_strings.end())
		{
			req.underlying = conversions::to_utf8string(query_strings.at(U("_symbol")));
		}
		else
		{
			throw std::invalid_argument("No underlying symbol");
		}
	}

	void VanillaOptMessage::ParseMaturity(VanillaOptMessage::Request &req, \
		const std::map<string_t, string_t>& query_strings)
	{
		if (query_strings.find(U("_maturity")) != query_strings.end())
		{
			auto mat = conversions::to_utf8string(query_strings.at(U("_maturity")));
			req.maturity = dd::from_string(mat);
		}
		else
		{
			throw std::invalid_argument("No maturity date");
		}
	}

	void VanillaOptMessage::ParseStrike(VanillaOptMessage::Request &req, \
		const std::map<string_t, string_t>& query_strings)
	{
		if (query_strings.find(U("_strike")) != query_strings.end())
		{
			auto strike = conversions::to_utf8string(query_strings.at(U("_strike")));
			req.strike = stod(strike);
		}
		else
		{
			throw std::invalid_argument("No strike value");
		}
	}

	void VanillaOptMessage::ParseVol(VanillaOptMessage::Request &req, \
		const std::map<string_t, string_t>& query_strings)
	{
		if (query_strings.find(U("_vol")) != query_strings.end())
		{
			auto vol = conversions::to_utf8string(query_strings.at(U("_vol")));
			req.vol = stod(vol);
		}
	}

	VanillaOptMessage::OptionTypeEnum VanillaOptMessage::ParseOptionType(const std::map<string_t, string_t>& query_strings)
	{
		if (query_strings.find(U("_option")) != query_strings.end())
		{
			if (query_strings.at(U("_option")).compare(U("call")) == 0)
			{
				return VanillaOptMessage::CALL;
			}
			else if (query_strings.at(U("_option")).compare(U("put")) == 0)
			{
				return VanillaOptMessage::PUT;
			}
			else
			{
				throw std::invalid_argument("Invalid option type. Should Call/Put");
			}
		}
		else
		{
			return VanillaOptMessage::TYPE_UNKNOWN;
		}
	}

	VanillaOptMessage::OptionStyleEnum  VanillaOptMessage::ParseOptionStyle(const std::map<string_t, string_t>& query_strings)
	{
		if (query_strings.find(U("_style")) != query_strings.end())
		{
			if (query_strings.at(U("_style")).compare(U("european")) == 0)
			{
				return VanillaOptMessage::EUROPEAN;
			}
			else if (query_strings.at(U("_style")).compare(U("american")) == 0)
			{
				return VanillaOptMessage::AMERICAN;
			}
			else
			{
				throw std::invalid_argument("Invalid Style parameter");
			}
		}
		else
		{
			return VanillaOptMessage::STYLE_UNKNOWN;
		}
	}

	VanillaOptMessage::PricingMethodEnum  VanillaOptMessage::ParsePricingMethod(const std::map<string_t, string_t>& query_strings)
	{
		if (query_strings.find(U("_method")) != query_strings.end())
		{
			if (query_strings.at(U("_method")).compare(U("closed")) == 0)
			{
				return VanillaOptMessage::CLOSED;
			}
			else if (query_strings.at(U("_method")).compare(U("lattice")) == 0)
			{
				return VanillaOptMessage::LATTICE;
			}
			else if (query_strings.at(U("_method")).compare(U("montecarlo")) == 0)
			{
				return VanillaOptMessage::MONTE_CARLO;
			}
			else
			{
				throw std::invalid_argument("Invalid pricing method parameter");
			}
		}
		else
		{
			return VanillaOptMessage::METHOD_UNKNOWN;
		}
	}

	VanillaOptMessage::RateTypeEnum  VanillaOptMessage::ParseRateType(const std::map<string_t, string_t>& query_strings)
	{
		if (query_strings.find(U("_ratetype")) != query_strings.end())
		{
			if (query_strings.at(U("_ratetype")).compare(U("Yield")) == 0)
			{
				return VanillaOptMessage::YIELD;
			}
			else if (query_strings.at(U("_ratetype")).compare(U("LIBOR")) == 0)
			{
				return VanillaOptMessage::LIBOR;
			}
			else
			{
				throw std::invalid_argument("Invalid rate type parameter");
			}
		}
		else
		{
			return VanillaOptMessage::LIBOR;
		}
	}

	VanillaOptMessage::VolatilityTypeEnum  VanillaOptMessage::ParseVolType(const std::map<string_t, string_t>& query_strings)
	{
		if (query_strings.find(U("_voltype")) != query_strings.end())
		{
			if (query_strings.at(U("_voltype")).compare(U("IV")) == 0)
			{
				return VanillaOptMessage::IV;
			}
			else if (query_strings.at(U("_voltype")).compare(U("HV")) == 0)
			{
				return VanillaOptMessage::HV;
			}
			else
			{
				throw std::invalid_argument("Invalid Volatility type parameter");
			}
		}
	}
}
/* namespace derivative */