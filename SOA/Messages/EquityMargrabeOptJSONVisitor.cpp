/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#include "EquityMargrabeOptJSONVisitor.hpp"
#include "EquityMargrabeOptMessage.hpp"

namespace derivative
{
	void EquityMargrabeOptJSONVisitor::Visit(const std::shared_ptr<IMessage>& message, json::value& out)
	{
		json::value reqObj;
		json::value resObj;
		json::value greekObj;

		std::shared_ptr<EquityMargrabeOptMessage> msg;
		try
		{
			/// if we are here then the message should be of type EquityMargrabeOptMessage.
			msg = dynamic_pointer_cast<EquityMargrabeOptMessage>(message);
		}
		catch(std::bad_cast& e)
		{
			LOG(ERROR) << "This should not happen to..  " << message << endl;
			assert(false);
		}

		/// adding request parameters
		reqObj[L"equity symbol"] = json::value::string(utility::conversions::to_string_t(msg->GetRequest().underlying));
		reqObj[L"numeraire equity symbol"] = json::value::string(utility::conversions::to_string_t(msg->GetRequest().numeraire));
		reqObj[L"option"] = (msg->GetRequest().option == EquityMargrabeOptMessage::CALL) ? json::value::string(U("call")) : json::value::string(U("put"));
		reqObj[L"maturityDate"] = json::value::string(utility::conversions::to_string_t(dd::to_simple_string(msg->GetRequest().maturity)));

		/// adding greeks and response parameters
		resObj[L"underlying trade date"] = json::value::string(utility::conversions::to_string_t(dd::to_simple_string(msg->GetResponse().underlyingTradeDate)));
		resObj[L"last underlying price"] = json::value::number(msg->GetResponse().underlyingTradePrice);
		resObj[L"option price"] = json::value::number(msg->GetResponse().optPrice);
		out[L"request"] = reqObj;
		out[L"response"] = resObj;

		/// adding message status
		out[L"outcome"] = json::value::number(msg->GetSystemResponse().outcome);
		out[L"Message"] = json::value::string(utility::conversions::to_string_t(msg->GetSystemResponse().outText));
	}
} /* namespace derivative */