/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#include "FuturesVanillaOptJSONVisitor.hpp"
#include "FuturesVanillaOptMessage.hpp"

namespace derivative
{
	void FuturesVanillaOptJSONVisitor::Visit(const std::shared_ptr<IMessage>& message, json::value& out)
	{
		json::value reqObj;
		json::value resObj;
		json::value greekObj;

		std::shared_ptr<FuturesVanillaOptMessage> msg;
		try
		{
			/// if we are here then the message should be of type FuturesVanillaOptMessage.
			msg = dynamic_pointer_cast<FuturesVanillaOptMessage>(message);
		}
		catch(std::bad_cast& e)
		{
			LOG(ERROR) << "This should not happen to..  " << message << endl;
			assert(false);
		}

		/// adding request parameters
		reqObj[L"futures symbol"] = json::value::string(utility::conversions::to_string_t(msg->GetRequest().underlying));
		reqObj[L"option"] = (msg->GetRequest().option == FuturesVanillaOptMessage::CALL) ? json::value::string(U("call")) : json::value::string(U("put"));
		reqObj[L"style"] = (msg->GetRequest().style == FuturesVanillaOptMessage::EUROPEAN) ? json::value::string(U("european")) : json::value::string(U("american"));
		reqObj[L"strike"] = json::value::number(msg->GetRequest().strike);
		reqObj[L"maturityDate"] = json::value::string(utility::conversions::to_string_t(dd::to_simple_string(msg->GetRequest().maturity)));
		reqObj[L"deliveryDate"] = json::value::string(utility::conversions::to_string_t(dd::to_simple_string(msg->GetRequest().deliveryDate)));

		/// adding greeks and response parameters
		resObj[L"trade date"] = json::value::string(utility::conversions::to_string_t(dd::to_simple_string(msg->GetResponse().underlyingTradeDate)));
		resObj[L"last price"] = json::value::number(msg->GetResponse().underlyingTradePrice);
		resObj[L"option price"] = json::value::number(msg->GetResponse().optPrice);
		greekObj[L"delta"] = json::value::number(msg->GetResponse().greeks.delta);
		greekObj[L"gamma"] = json::value::number(msg->GetResponse().greeks.gamma);
		greekObj[L"vega"] = json::value::number(msg->GetResponse().greeks.vega);
	//	greekObj[L"theta"] = json::value::number(msg->GetResponse().greeks.theta);
		resObj[L"greeks (BS)"] = greekObj;
		out[L"request"] = reqObj;
		out[L"response"] = resObj;

		/// adding message status
		out[L"outcome"] = json::value::number(msg->GetSystemResponse().outcome);
		out[L"Message"] = json::value::string(utility::conversions::to_string_t(msg->GetSystemResponse().outText));
	}
} /* namespace derivative */