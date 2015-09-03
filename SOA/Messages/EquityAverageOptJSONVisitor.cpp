/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#include "EquityAverageOptJSONVisitor.hpp"
#include "EquityAverageOptMessage.hpp"
#include "QFUtil.hpp"

namespace derivative
{
	void EquityAverageOptJSONVisitor::Visit(const std::shared_ptr<IMessage>& message, json::value& out)
	{
		json::value reqObj;
		json::value resObj;
		json::value greekObj;

		std::shared_ptr<EquityAverageOptMessage> msg;
		try
		{
			/// if we are here then the message should be of type EquityAverageOptMessage.
			msg = dynamic_pointer_cast<EquityAverageOptMessage>(message);
		}
		catch (std::bad_cast& e)
		{
			LOG(ERROR) << "This should not happen to..  " << message << endl;
			assert(false);
		}

		/// adding request parameters
		reqObj[L"equity symbol"] = json::value::string(utility::conversions::to_string_t(msg->GetRequest().underlying));
		reqObj[L"option"] = (msg->GetRequest().option == EquityAverageOptMessage::CALL) ? json::value::string(U("call")) : json::value::string(U("put"));
		if (msg->GetRequest().averageType == EquityAverageOptMessage::FIXED_STRIKE)
		{
			reqObj[L"Strike type"] = (msg->GetRequest().averageType == EquityAverageOptMessage::FIXED_STRIKE) ? json::value::string(U("Fixed strike")) : json::value::string(U("Floating strike"));
		}
		if (msg->GetRequest().averageType == EquityAverageOptMessage::FIXED_STRIKE) reqObj[L"strike"] = json::value::number(msg->GetRequest().strike);
		reqObj[L"maturityDate"] = json::value::string(utility::conversions::to_string_t(dd::to_simple_string(msg->GetRequest().maturity)));

		/// adding greeks and response parameters
		resObj[L"underlying trade date"] = json::value::string(utility::conversions::to_string_t(dd::to_simple_string(msg->GetResponse().underlyingTradeDate)));
		resObj[L"last underlying price"] = json::value::number(msg->GetResponse().underlyingTradePrice);
		resObj[L"average option price"] = json::value::string(utility::conversions::to_string_t((to_money<double>(msg->GetResponse().averageOptPrice))));
		if (msg->GetRequest().averageType == EquityAverageOptMessage::FIXED_STRIKE)
		{
			resObj[L"vanilla option price"] = json::value::string(utility::conversions::to_string_t((to_money<double>(msg->GetResponse().optPrice))));
		}
		resObj[L"Volatility"] = json::value::number(msg->GetResponse().vol);
		out[L"request"] = reqObj;
		out[L"response"] = resObj;

		/// adding message status
		out[L"outcome"] = json::value::number(msg->GetSystemResponse().outcome);
		out[L"Message"] = json::value::string(utility::conversions::to_string_t(msg->GetSystemResponse().outText));
	}
} /* namespace derivative */