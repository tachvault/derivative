/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include "FuturesVanillaOptMessage.hpp"
#include "FuturesVanillaOptJSONVisitor.hpp"

namespace derivative
{
	json::value FuturesVanillaOptMessage::AsJSON()
	{
		FuturesVanillaOptJSONVisitor visitor;
		json::value jsonMsg;
		visitor.Visit(shared_from_this(), jsonMsg);
		return jsonMsg;
	}

	void FuturesVanillaOptMessage::ParseDeliveryDate(FuturesVanillaOptMessage::FuturesRequest &req, const std::map<string_t, string_t>& query_strings)
	{
		if (query_strings.find(U("_delivery")) != query_strings.end())
		{
			auto del = conversions::to_utf8string(query_strings.at(U("_delivery")));
			req.deliveryDate = dd::from_string(del);
		}
		else
		{
			throw std::invalid_argument("No delivery date");
		}
	}
}
/* namespace derivative */