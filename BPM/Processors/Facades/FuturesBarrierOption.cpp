/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#include <functional>
#include <random/normal.h>
#include <boost/math/distributions/normal.hpp>

#include "FuturesBarrierOption.hpp"
#include "DException.hpp"
#include "SystemUtil.hpp"
#include "EntityMgrUtil.hpp"
#include "MsgRegister.hpp"
#include "GroupRegister.hpp"
#include "FuturesBarrierOptMessage.hpp"

#include "IFutures.hpp"
#include "IFuturesValue.hpp"
#include "BlackScholesAssetAdapter.hpp"
#include "PrimaryAssetUtil.hpp"
#include "IRCurve.hpp"

#include "ConstVol.hpp"
#include "PiecewiseVol.hpp"
#include "FuturesVolatilitySurface.hpp"

#include "FuturesAssetPricer.hpp"
#include "FuturesOption.hpp"

using namespace derivative;
using namespace derivative::SystemUtil;

using blitz::Array;
using blitz::Range;
using blitz::firstIndex;
using blitz::secondIndex;

namespace derivative
{
	GROUP_REGISTER(FuturesBarrierOption);
	ALIAS_REGISTER(FuturesBarrierOption, IMessageSink);
	MSG_REGISTER(FuturesBarrierOptMessage, FuturesBarrierOption);

	FuturesBarrierOption::FuturesBarrierOption(const Exemplar &ex)
		:FuturesOption(),
		m_name(TYPEID)
	{}

	FuturesBarrierOption::FuturesBarrierOption(const Name& nm)
		: FuturesOption(),
		m_name(nm)
	{}

	std::shared_ptr<IMake> FuturesBarrierOption::Make(const Name &nm)
	{
		/// Construct FuturesBarrierOption from given name and register with EntityManager
		std::shared_ptr<FuturesBarrierOption> optionProc = std::make_shared<FuturesBarrierOption>(nm);

		/// now register the object
		EntityMgrUtil::registerObject(nm, optionProc);

		/// return constructed object if no exception is thrown
		return optionProc;
	}

	std::shared_ptr<IMake> FuturesBarrierOption::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("not implemented");
	}

	void FuturesBarrierOption::Dispatch(std::shared_ptr<IMessage>& msg)
	{
		std::shared_ptr<FuturesBarrierOptMessage> optMsg = std::dynamic_pointer_cast<FuturesBarrierOptMessage>(msg);
		dd::date today = dd::day_clock::local_day();

		/// transfer request inputs to member variables 
		m_optType = (optMsg->GetRequest().option == FuturesBarrierOptMessage::CALL) ? 1 : -1;
		m_barrierType = optMsg->GetRequest().barrierType;
		m_maturity = optMsg->GetRequest().maturity;
		m_strike = optMsg->GetRequest().strike;
		m_barrier = optMsg->GetRequest().barrier;
		m_delivery = optMsg->GetRequest().deliveryDate;

		/// get futures value.
		m_futuresVal = PrimaryUtil::getFuturesValue(optMsg->GetRequest().underlying, today, m_delivery);

		ProcessVol(optMsg);
		ProcessRate(optMsg);

		/// now construct the BlackScholesAdapter from the futures value.
		m_futures = std::make_shared<BlackScholesAssetAdapter>(m_futuresVal, m_vol);
		/// for futures, the yield is the same as interest rate in black scholes world.
		m_futures->SetDivYield(m_termRate);

		/// get the pricing method and run 
		FuturesBarrierOptMessage::BarrierResponse res;
		auto t = double((m_maturity - dd::day_clock::local_day()).days()) / 365;
		res.optPrice = m_futures->option(t, m_strike, m_termRate, m_optType);
		if (optMsg->GetRequest().method == FuturesBarrierOptMessage::MONTE_CARLO)
		{
			res.barierOptPrice = FuturesBarrierOptionPricer::ValueWithMC(m_futures, m_term, m_barrierType, m_maturity, \
				m_strike, m_barrier, static_cast<FuturesBarrierOptionPricer::VanillaOptionType>(m_optType));
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
