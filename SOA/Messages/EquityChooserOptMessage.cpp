/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include "EquityChooserOptMessage.hpp"
#include "EquityChooserOptJSONVisitor.hpp"

namespace derivative
{
	json::value EquityChooserOptMessage::AsJSON()
	{
		EquityChooserOptJSONVisitor visitor;
		json::value jsonMsg;
		visitor.Visit(shared_from_this(), jsonMsg);
		return jsonMsg;
	}
} 
/* namespace derivative */