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
} 
/* namespace derivative */