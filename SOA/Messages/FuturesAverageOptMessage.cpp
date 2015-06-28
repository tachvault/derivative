/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include "FuturesAverageOptMessage.hpp"
#include "FuturesAverageOptJSONVisitor.hpp"

namespace derivative
{
	json::value FuturesAverageOptMessage::AsJSON()
	{
		FuturesAverageOptJSONVisitor visitor;
		json::value jsonMsg;
		visitor.Visit(shared_from_this(), jsonMsg);
		return jsonMsg;
	}
	
	FuturesAverageOptMessage::AverageTypeEnum FuturesAverageOptMessage::ParseAverageType(const std::map<string_t, string_t>& query_strings)
	{
		if (query_strings.find(U("_averageType")) != query_strings.end())
		{
			if (query_strings.at(U("_averageType")).compare(U("fixed_strike")) == 0)
			{
				return FuturesAverageOptMessage::FIXED_STRIKE;
			}
			else if (query_strings.at(U("_averageType")).compare(U("floating_strike")) == 0)
			{
				return FuturesAverageOptMessage::FLOATING_STRIKE;
			}
			else
			{
				throw std::invalid_argument("Invalid Average type parameter");
			}
		}
		else
		{
			throw std::invalid_argument("Invalid Average type parameter");
		}
	}
} 
/* namespace derivative */