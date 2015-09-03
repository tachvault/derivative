/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include "ExchangeRateVanillaOptMessage.hpp"
#include "ExchangeRateVanillaOptJSONVisitor.hpp"

namespace derivative
{
	json::value ExchangeRateVanillaOptMessage::AsJSON()
	{
		ExchangeRateVanillaOptJSONVisitor visitor;
		json::value jsonMsg;
		visitor.Visit(shared_from_this(), jsonMsg);
		return jsonMsg;
	}
}
/* namespace derivative */