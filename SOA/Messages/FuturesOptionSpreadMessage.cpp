/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include "FuturesOptionSpreadMessage.hpp"
#include "FuturesOptionSpreadJSONVisitor.hpp"

namespace derivative
{
	json::value FuturesOptionSpreadMessage::AsJSON()
	{
		FuturesOptionSpreadJSONVisitor visitor;
		json::value jsonMsg;
		visitor.Visit(shared_from_this(), jsonMsg);
		return jsonMsg;
	}
} 
/* namespace derivative */