/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include "EquityOptionSpreadMessage.hpp"
#include "EquityOptionSpreadJSONVisitor.hpp"

namespace derivative
{
	json::value EquityOptionSpreadMessage::AsJSON()
	{
		EquityOptionSpreadJSONVisitor visitor;
		json::value jsonMsg;
		visitor.Visit(shared_from_this(), jsonMsg);
		return jsonMsg;
	}
} 
/* namespace derivative */