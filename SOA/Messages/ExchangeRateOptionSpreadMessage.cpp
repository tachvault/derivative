/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include "ExchangeRateOptionSpreadMessage.hpp"
#include "ExchangeRateOptionSpreadJSONVisitor.hpp"

namespace derivative
{
	json::value ExchangeRateOptionSpreadMessage::AsJSON()
	{
		ExchangeRateOptionSpreadJSONVisitor visitor;
		json::value jsonMsg;
		visitor.Visit(shared_from_this(), jsonMsg);
		return jsonMsg;
	}
}
/* namespace derivative */