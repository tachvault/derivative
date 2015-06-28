/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include "ExchangeRateBarrierOptMessage.hpp"
#include "ExchangeRateBarrierOptJSONVisitor.hpp"

namespace derivative
{	
	json::value ExchangeRateBarrierOptMessage::AsJSON()
	{
		ExchangeRateBarrierOptJSONVisitor visitor;
		json::value jsonMsg;
		visitor.Visit(shared_from_this(), jsonMsg);
		return jsonMsg;
	}

	ExchangeRateBarrierOptMessage::BarrierOptionTypeEnum ExchangeRateBarrierOptMessage::ParseBarrierType(const std::map<string_t, string_t>& query_strings)
	{
		if (query_strings.find(U("_barrierType")) != query_strings.end())
		{
			if (query_strings.at(U("_barrierType")).compare(U("knock_down_out")) == 0)
			{
				return ExchangeRateBarrierOptMessage::KDO;
			}
			else if (query_strings.at(U("_barrierType")).compare(U("knock_down_in")) == 0)
			{
				return ExchangeRateBarrierOptMessage::KDI;
			}
			else if (query_strings.at(U("_barrierType")).compare(U("knock_up_out")) == 0)
			{
				return ExchangeRateBarrierOptMessage::KUO;
			}
			else if (query_strings.at(U("_barrierType")).compare(U("knock_up_in")) == 0)
			{
				return ExchangeRateBarrierOptMessage::KUI;
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