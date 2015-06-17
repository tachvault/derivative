/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#include "EquityOptionSpreadJSONVisitor.hpp"
#include "EquityOptionSpreadMessage.hpp"

namespace derivative
{
	void EquityOptionSpreadJSONVisitor::Visit(const std::shared_ptr<IMessage>& message, json::value& out)
	{
		std::shared_ptr<EquityOptionSpreadMessage> msg;
		try
		{
			/// if we are here then the message should be of type EquityOptionSpreadMessage.
			msg = dynamic_pointer_cast<EquityOptionSpreadMessage>(message);
		}
		catch (std::bad_cast& e)
		{
			LOG(ERROR) << "This should not happen to..  " << message << endl;
			assert(false);
		}

		/// adding request parameters
		auto req = msg->GetRequest();
		json::value reqObj;
		json::value resObj;
		json::value greekObj;

		reqObj[L"equity symbol"] = json::value::string(utility::conversions::to_string_t(req.underlying));
		reqObj[L"style"] = (req.style == EquityOptionSpreadMessage::EUROPEAN) ? json::value::string(U("european")) : json::value::string(U("american"));
		if (fabs(req.vol - 0.0) > std::numeric_limits<int>::epsilon())
		{
			reqObj[L"volatility"] = json::value::string(utility::conversions::to_string_t(to_string(req.vol)));
		}
		if (req.equityPos.validate())
		{
			reqObj[L"naked position"] = (req.equityPos.pos == EquityOptionSpreadMessage::LONG) ? json::value::string(U("long")) : json::value::string(U("short"));
			reqObj[L"naked position units"] = req.equityPos.units;
		}
		int i = 0;
		json::value reqLegArray = json::value::array();
		for (auto &leg : req.legs)
		{
			json::value legObj = json::value::object();
			legObj[L"leg"] = json::value::number(i+1);
			legObj[L"option"] = (leg.option == EquityOptionSpreadMessage::CALL) ? json::value::string(U("call")) : json::value::string(U("put"));
			legObj[L"naked position"] = (leg.pos == EquityOptionSpreadMessage::LONG) ? json::value::string(U("long")) : json::value::string(U("short"));
			legObj[L"strike"] = json::value::number(leg.strike);
			legObj[L"maturityDate"] = json::value::string(utility::conversions::to_string_t(dd::to_simple_string(leg.maturity)));
			legObj[L"units"] = leg.units;
			reqLegArray[i] = legObj;
			++i;
		}
		reqObj[L"Legs"] = reqLegArray;

		/// adding greeks and response parameters
		auto res = msg->GetResponse();
		resObj[L"trade date"] = json::value::string(utility::conversions::to_string_t(dd::to_simple_string(res.underlyingTradeDate)));
		resObj[L"last price"] = json::value::number(res.underlyingTradePrice);
		resObj[L"spread price"] = json::value::number(res.spreadPrice);
		i = 0;
		json::value resLegArray = json::value::array();
		for (auto &leg : res.legs)
		{
			json::value legGreekObj;
			json::value legObj = json::value::object();
			legObj[L"leg"] = json::value::number(i + 1);
			legObj[L"option price"] = json::value::number(leg.optPrice);
			legGreekObj[L"delta"] = json::value::number(leg.greeks.delta);
			legGreekObj[L"gamma"] = json::value::number(leg.greeks.gamma);
			legGreekObj[L"vega"] = json::value::number(leg.greeks.vega);
			legObj[L"Greeks"] = legGreekObj;
			resLegArray[i] = legObj;
			++i;
		}
		resObj[L"Legs"] = resLegArray;
		greekObj[L"delta"] = json::value::number(res.greeks.delta);
		greekObj[L"gamma"] = json::value::number(res.greeks.gamma);
		greekObj[L"vega"] = json::value::number(res.greeks.vega);
		resObj[L"greeks"] = greekObj;
		out[L"request"] = reqObj;
		out[L"response"] = resObj;

		/// adding message status
		out[L"outcome"] = json::value::number(msg->GetSystemResponse().outcome);
		out[L"Message"] = json::value::string(utility::conversions::to_string_t(msg->GetSystemResponse().outText));
	}
} /* namespace derivative */