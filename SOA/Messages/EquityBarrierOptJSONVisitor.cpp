/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#include "EquityBarrierOptJSONVisitor.hpp"
#include "EquityBarrierOptMessage.hpp"

namespace derivative
{
	void EquityBarrierOptJSONVisitor::Visit(const std::shared_ptr<IMessage>& message, json::value& out)
	{
		json::value reqObj;
		json::value resObj;
		json::value greekObj;

		std::shared_ptr<EquityBarrierOptMessage> msg;
		try
		{
			/// if we are here then the message should be of type EquityBarrierOptMessage.
			msg = dynamic_pointer_cast<EquityBarrierOptMessage>(message);
		}
		catch(std::bad_cast& e)
		{
			LOG(ERROR) << "This should not happen to..  " << message << endl;
			assert(false);
		}

		/// adding request parameters
		reqObj[L"equity symbol"] = json::value::string(utility::conversions::to_string_t(msg->GetRequest().underlying));
		reqObj[L"option"] = (msg->GetRequest().option == EquityBarrierOptMessage::CALL) ? json::value::string(U("call")) : json::value::string(U("put"));
		if (msg->GetRequest().barrierType == EquityBarrierOptMessage::KDI)
		{
			reqObj[L"Barrier option type"] = json::value::string(U("Knock down-and-in"));
		}
		else if (msg->GetRequest().barrierType == EquityBarrierOptMessage::KDO)
		{
			reqObj[L"Barrier option type"] = json::value::string(U("Knock down-and-out"));
		}
		else if (msg->GetRequest().barrierType == EquityBarrierOptMessage::KUI)
		{
			reqObj[L"Barrier option type"] = json::value::string(U("Knock up-and-in"));
		}
		else if (msg->GetRequest().barrierType == EquityBarrierOptMessage::KUO)
		{
			reqObj[L"Barrier option type"] = json::value::string(U("Knock up-and-out"));
		}
		reqObj[L"strike"] = json::value::number(msg->GetRequest().strike);
		reqObj[L"barrier"] = json::value::number(msg->GetRequest().barrier);
		reqObj[L"maturityDate"] = json::value::string(utility::conversions::to_string_t(dd::to_simple_string(msg->GetRequest().maturity)));

		/// adding greeks and response parameters
		resObj[L"trade date"] = json::value::string(utility::conversions::to_string_t(dd::to_simple_string(msg->GetResponse().underlyingTradeDate)));
		resObj[L"last price"] = json::value::number(msg->GetResponse().underlyingTradePrice);
		resObj[L"vanilla option price"] = json::value::number(msg->GetResponse().optPrice);
		resObj[L"barrier price"] = json::value::number(msg->GetResponse().barierOptPrice);
		out[L"request"] = reqObj;
		out[L"response"] = resObj;

		/// adding message status
		out[L"outcome"] = json::value::number(msg->GetSystemResponse().outcome);
		out[L"Message"] = json::value::string(utility::conversions::to_string_t(msg->GetSystemResponse().outText));
	}
} /* namespace derivative */