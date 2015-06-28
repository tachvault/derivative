/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include "EquityVanillaOptMessage.hpp"
#include "EquityVanillaOptJSONVisitor.hpp"

namespace derivative
{
	json::value EquityVanillaOptMessage::AsJSON()
	{
		EquityVanillaOptJSONVisitor visitor;
		json::value jsonMsg;
		visitor.Visit(shared_from_this(), jsonMsg);
		return jsonMsg;
	}
} 
/* namespace derivative */