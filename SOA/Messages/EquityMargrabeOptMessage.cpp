/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include "EquityMargrabeOptMessage.hpp"
#include "EquityMargrabeOptJSONVisitor.hpp"

namespace derivative
{
	json::value EquityMargrabeOptMessage::AsJSON()
	{
		EquityMargrabeOptJSONVisitor visitor;
		json::value jsonMsg;
		visitor.Visit(shared_from_this(), jsonMsg);
		return jsonMsg;
	}

	void EquityMargrabeOptMessage::ParseStrike(VanillaOptMessage::Request &req, const std::map<string_t, string_t>& query_strings)
	{
		if (query_strings.find(U("_strike")) != query_strings.end())
		{
			auto strike = conversions::to_utf8string(query_strings.at(U("_strike")));
			req.strike = stod(strike);
		}
		else
		{
			req.strike = 1.0;
		}
	}
	
	VanillaOptMessage::PricingMethodEnum  EquityMargrabeOptMessage::ParsePricingMethod(const std::map<string_t, string_t>& query_strings)
	{
		if (query_strings.find(U("_method")) != query_strings.end())
		{
			if (query_strings.at(U("_method")).compare(U("closed")) == 0)
			{
				return VanillaOptMessage::CLOSED;
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
} 
/* namespace derivative */