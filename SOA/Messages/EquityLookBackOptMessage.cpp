/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include "EquityLookBackOptMessage.hpp"
#include "EquityLookBackOptJSONVisitor.hpp"

namespace derivative
{
	json::value EquityLookBackOptMessage::AsJSON()
	{
		EquityLookBackOptJSONVisitor visitor;
		json::value jsonMsg;
		visitor.Visit(shared_from_this(), jsonMsg);
		return jsonMsg;
	}

	EquityLookBackOptMessage::LookBackTypeEnum EquityLookBackOptMessage::ParseLookBackType(const std::map<string_t, string_t>& query_strings)
	{
		if (query_strings.find(U("_lookbackType")) != query_strings.end())
		{
			if (query_strings.at(U("_lookbackType")).compare(U("fixed_strike")) == 0)
			{
				return EquityLookBackOptMessage::FIXED_STRIKE;
			}
			else if (query_strings.at(U("_lookbackType")).compare(U("floating_strike")) == 0)
			{
				return EquityLookBackOptMessage::FLOATING_STRIKE;
			}
			else
			{
				throw std::invalid_argument("Invalid LookBack type parameter");
			}
		}
		else
		{
			throw std::invalid_argument("Invalid LookBack type parameter");
		}
	}

} 
/* namespace derivative */