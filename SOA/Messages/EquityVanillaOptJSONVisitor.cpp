/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#include "EquityVanillaOptJSONVisitor.hpp"
#include "EquityVanillaOptMessage.hpp"

namespace derivative
{
	void EquityVanillaOptJSONVisitor::Visit(const std::shared_ptr<IMessage>& message, json::value& out)
	{
		json::value reqObj;
		json::value resObj;
		json::value greekObj;

		std::shared_ptr<EquityVanillaOptMessage> msg;
		try
		{
			/// if we are here then the message should be of type EquityVanillaOptMessage.
			msg = dynamic_pointer_cast<EquityVanillaOptMessage>(message);
		}
		catch(std::bad_cast& e)
		{
			LOG(ERROR) << "This should not happen to..  " << message << endl;
			assert(false);
		}

		/// adding request parameters
		reqObj[L"equity symbol"] = json::value::string(utility::conversions::to_string_t(msg->GetRequest().underlying));
		reqObj[L"option"] = (msg->GetRequest().option == EquityVanillaOptMessage::CALL) ? json::value::string(U("call")) : json::value::string(U("put"));
		reqObj[L"style"] = (msg->GetRequest().style == EquityVanillaOptMessage::EUROPEAN) ? json::value::string(U("european")) : json::value::string(U("american"));
		reqObj[L"strike"] = json::value::number(msg->GetRequest().strike);
		reqObj[L"maturityDate"] = json::value::string(utility::conversions::to_string_t(dd::to_simple_string(msg->GetRequest().maturity)));

		/// adding greeks and response parameters
		resObj[L"trade date"] = json::value::string(utility::conversions::to_string_t(dd::to_simple_string(msg->GetResponse().underlyingTradeDate)));
		resObj[L"last price"] = json::value::number(msg->GetResponse().underlyingTradePrice);
		resObj[L"option price"] = json::value::number(msg->GetResponse().optPrice);
		greekObj[L"delta"] = json::value::number(msg->GetResponse().greeks.delta);
		greekObj[L"gamma"] = json::value::number(msg->GetResponse().greeks.gamma);
		greekObj[L"vega"] = json::value::number(msg->GetResponse().greeks.vega);
		//greekObj[L"theta"] = json::value::number(msg->GetResponse().greeks.theta);
		resObj[L"greeks"] = greekObj;
		out[L"request"] = reqObj;
		out[L"response"] = resObj;

		/// adding message status
		out[L"outcome"] = json::value::number(msg->GetSystemResponse().outcome);
		out[L"Message"] = json::value::string(utility::conversions::to_string_t(msg->GetSystemResponse().outText));
	}
} /* namespace derivative */