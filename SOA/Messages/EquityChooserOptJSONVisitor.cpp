/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#include "EquityChooserOptJSONVisitor.hpp"
#include "EquityChooserOptMessage.hpp"
#include "QFUtil.hpp"

namespace derivative
{
	void EquityChooserOptJSONVisitor::Visit(const std::shared_ptr<IMessage>& message, json::value& out)
	{
		json::value reqObj;
		json::value resObj;
		json::value greekObj;

		std::shared_ptr<EquityChooserOptMessage> msg;
		try
		{
			/// if we are here then the message should be of type EquityChooserOptMessage.
			msg = dynamic_pointer_cast<EquityChooserOptMessage>(message);
		}
		catch(std::bad_cast& e)
		{
			LOG(ERROR) << "This should not happen to..  " << message << endl;
			assert(false);
		}

		/// adding request parameters
		reqObj[L"equity symbol"] = json::value::string(utility::conversions::to_string_t(msg->GetRequest().underlying));
		reqObj[L"option"] = (msg->GetRequest().option == EquityChooserOptMessage::CALL) ? json::value::string(U("call")) : json::value::string(U("put"));
		reqObj[L"maturityDate"] = json::value::string(utility::conversions::to_string_t(dd::to_simple_string(msg->GetRequest().maturity)));

		/// adding greeks and response parameters
		resObj[L"underlying trade date"] = json::value::string(utility::conversions::to_string_t(dd::to_simple_string(msg->GetResponse().underlyingTradeDate)));
		resObj[L"last underlying price"] = json::value::number(msg->GetResponse().underlyingTradePrice);
		resObj[L"Vanilla option price"] = json::value::string(utility::conversions::to_string_t((to_money<double>(msg->GetResponse().optPrice))));
		resObj[L"option price"] = json::value::string(utility::conversions::to_string_t((to_money<double>(msg->GetResponse().chooserOptPrice))));
		resObj[L"Volatility"] = json::value::number(msg->GetResponse().vol);
		out[L"request"] = reqObj;
		out[L"response"] = resObj;

		/// adding message status
		out[L"outcome"] = json::value::number(msg->GetSystemResponse().outcome);
		out[L"Message"] = json::value::string(utility::conversions::to_string_t(msg->GetSystemResponse().outText));
	}
} /* namespace derivative */