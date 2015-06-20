/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#include <functional>
#include <random/normal.h>
#include <boost/math/distributions/normal.hpp>

#include "FuturesAverageOption.hpp"
#include "DException.hpp"
#include "SystemUtil.hpp"
#include "EntityMgrUtil.hpp"
#include "MsgRegister.hpp"
#include "GroupRegister.hpp"
#include "FuturesAverageOptMessage.hpp"

#include "IFutures.hpp"
#include "IFuturesValue.hpp"
#include "BlackScholesAssetAdapter.hpp"
#include "PrimaryAssetUtil.hpp"
#include "IRCurve.hpp"

#include "ConstVol.hpp"
#include "PiecewiseVol.hpp"
#include "FuturesVolatilitySurface.hpp"

#include "FuturesAssetPricer.hpp"

using namespace derivative;
using namespace derivative::SystemUtil;

using blitz::Array;
using blitz::Range;
using blitz::firstIndex;
using blitz::secondIndex;

namespace derivative
{
	GROUP_REGISTER(FuturesAverageOption);
	ALIAS_REGISTER(FuturesAverageOption, IMessageSink);
	MSG_REGISTER(FuturesAverageOptMessage, FuturesAverageOption);

	FuturesAverageOption::FuturesAverageOption(const Exemplar &ex)
		:FuturesOption(),
		m_name(TYPEID)
	{}

	FuturesAverageOption::FuturesAverageOption(const Name& nm)
		: FuturesOption(),
		m_name(nm)
	{}

	std::shared_ptr<IMake> FuturesAverageOption::Make(const Name &nm)
	{
		/// Construct FuturesAverageOption from given name and register with EntityManager
		std::shared_ptr<FuturesAverageOption> optionProc = std::make_shared<FuturesAverageOption>(nm);

		/// now register the object
		EntityMgrUtil::registerObject(nm, optionProc);

		/// return constructed object if no exception is thrown
		return optionProc;
	}

	std::shared_ptr<IMake> FuturesAverageOption::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("not implemented");
	}

	void FuturesAverageOption::Dispatch(std::shared_ptr<IMessage>& msg)
	{
		std::shared_ptr<FuturesAverageOptMessage> optMsg = std::dynamic_pointer_cast<FuturesAverageOptMessage>(msg);

		/// transfer request inputs to member variables 
		m_optType = (optMsg->GetRequest().option == FuturesAverageOptMessage::CALL) ? 1 : -1;
		m_averageType = optMsg->GetRequest().averageType;
		m_maturity = optMsg->GetRequest().maturity;
		m_delivery = optMsg->GetRequest().deliveryDate;
		m_strike = optMsg->GetRequest().strike;

		/// get futures value.
		dd::date today = dd::day_clock::local_day();
		m_futuresVal = PrimaryUtil::getFuturesValue(optMsg->GetRequest().underlying, today, m_delivery);

		ProcessVol(optMsg);
		ProcessRate(optMsg);

		/// get the interest rate
		auto t = double((m_maturity - dd::day_clock::local_day()).days()) / 365;

		/// now construct the BlackScholesAdapter from the futures value.
		m_futures = std::make_shared<BlackScholesAssetAdapter>(m_futuresVal, m_vol);
		/// for futures, the yield is the same as interest rate in black scholes world.
		m_futures->SetDivYield(m_termRate);

		/// get the pricing method and run 
		FuturesAverageOptMessage::AverageOptResponse res;
		if (optMsg->GetRequest().method == FuturesAverageOptMessage::MONTE_CARLO)
		{
			res.averageOptPrice = FuturesAverageOptionPricer::ValueWithMC(m_futures, \
				m_term, m_averageType, m_maturity, m_strike, static_cast<FuturesAverageOptionPricer::VanillaOptionType>(m_optType));
		}
		else
		{
			throw std::invalid_argument("Invalid pricing method");
		}

		/// set the futures info
		res.underlyingTradeDate = m_futuresVal->GetTradeDate();
		res.underlyingTradePrice = m_futuresVal->GetTradePrice();

		/// set the message;
		optMsg->SetResponse(res);
	}
}
