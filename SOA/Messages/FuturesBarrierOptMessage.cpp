/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include "FuturesBarrierOptMessage.hpp"
#include "FuturesBarrierOptJSONVisitor.hpp"

namespace derivative
{	
	json::value FuturesBarrierOptMessage::AsJSON()
	{
		FuturesBarrierOptJSONVisitor visitor;
		json::value jsonMsg;
		visitor.Visit(shared_from_this(), jsonMsg);
		return jsonMsg;
	}

	FuturesBarrierOptMessage::BarrierOptionTypeEnum FuturesBarrierOptMessage::ParseBarrierType(const std::map<string_t, string_t>& query_strings)
	{
		if (query_strings.find(U("_barrierType")) != query_strings.end())
		{
			if (query_strings.at(U("_barrierType")).compare(U("knock_down_out")) == 0)
			{
				return FuturesBarrierOptMessage::KDO;
			}
			else if (query_strings.at(U("_barrierType")).compare(U("knock_down_in")) == 0)
			{
				return FuturesBarrierOptMessage::KDI;
			}
			else if (query_strings.at(U("_barrierType")).compare(U("knock_up_out")) == 0)
			{
				return FuturesBarrierOptMessage::KUO;
			}
			else if (query_strings.at(U("_barrierType")).compare(U("knock_up_in")) == 0)
			{
				return FuturesBarrierOptMessage::KUI;
			}
			else
			{
				throw std::invalid_argument("Invalid Barrier type parameter");
			}
		}
		else
		{
			throw std::invalid_argument("Invalid Barrier type parameter");
		}		
	}
} 
/* namespace derivative */