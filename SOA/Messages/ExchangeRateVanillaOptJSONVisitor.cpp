/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#include "ExchangeRateVanillaOptJSONVisitor.hpp"
#include "ExchangeRateVanillaOptMessage.hpp"
#include "QFUtil.hpp"

namespace derivative
{
	void ExchangeRateVanillaOptJSONVisitor::Visit(const std::shared_ptr<IMessage>& message, json::value& out)
	{
		json::value reqObj;
		json::value resObj;
		json::value greekObj;

		std::shared_ptr<ExchangeRateVanillaOptMessage> msg;
		try
		{
			/// if we are here then the message should be of type ExchangeRateVanillaOptMessage.
			msg = dynamic_pointer_cast<ExchangeRateVanillaOptMessage>(message);
		}
		catch (std::bad_cast& e)
		{
			LOG(ERROR) << "This should not happen to..  " << message << endl;
			assert(false);
		}

		/// adding request parameters
		reqObj[L"domestic"] = json::value::string(utility::conversions::to_string_t(msg->GetRequest().domestic));
		reqObj[L"foreign"] = json::value::string(utility::conversions::to_string_t(msg->GetRequest().foreign));
		reqObj[L"option"] = (msg->GetRequest().option == ExchangeRateVanillaOptMessage::CALL) ? json::value::string(U("call")) : json::value::string(U("put"));
		reqObj[L"style"] = (msg->GetRequest().style == ExchangeRateVanillaOptMessage::EUROPEAN) ? json::value::string(U("european")) : json::value::string(U("american"));
		reqObj[L"strike"] = json::value::number(msg->GetRequest().strike);
		reqObj[L"maturityDate"] = json::value::string(utility::conversions::to_string_t(dd::to_simple_string(msg->GetRequest().maturity)));

		/// adding greeks and response parameters
		resObj[L"underlying trade date"] = json::value::string(utility::conversions::to_string_t(dd::to_simple_string(msg->GetResponse().underlyingTradeDate)));
		resObj[L"last underlying price"] = json::value::number(msg->GetResponse().underlyingTradePrice);
		resObj[L"option price"] = json::value::string(utility::conversions::to_string_t((to_money<double>(msg->GetResponse().optPrice))));
		resObj[L"Volatility"] = json::value::number(msg->GetResponse().vol);
		greekObj[L"delta"] = json::value::number(msg->GetResponse().greeks.delta);
		greekObj[L"gamma"] = json::value::number(msg->GetResponse().greeks.gamma);
		greekObj[L"vega"] = json::value::number(msg->GetResponse().greeks.vega);
		//greekObj[L"theta"] = json::value::number(msg->GetResponse().greeks.theta);
		resObj[L"greeks (BS)"] = greekObj;
		out[L"request"] = reqObj;
		out[L"response"] = resObj;

		/// adding message status
		out[L"outcome"] = json::value::number(msg->GetSystemResponse().outcome);
		out[L"Message"] = json::value::string(utility::conversions::to_string_t(msg->GetSystemResponse().outText));
	}
} /* namespace derivative */