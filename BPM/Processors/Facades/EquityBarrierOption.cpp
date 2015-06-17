/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#include <functional>
#include <random/normal.h>
#include <boost/math/distributions/normal.hpp>

#include "EquityBarrierOption.hpp"
#include "DException.hpp"
#include "SystemUtil.hpp"
#include "EntityMgrUtil.hpp"
#include "MsgRegister.hpp"
#include "GroupRegister.hpp"
#include "EquityBarrierOptMessage.hpp"

#include "IStock.hpp"
#include "IStockValue.hpp"
#include "BlackScholesAssetAdapter.hpp"
#include "PrimaryAssetUtil.hpp"
#include "IRCurve.hpp"

#include "ConstVol.hpp"
#include "PiecewiseVol.hpp"
#include "EquityVolatilitySurface.hpp"
#include "EquityGARCH.hpp"

#include "EquityAssetPricer.hpp"
#include "EquityOption.hpp"

using namespace derivative;
using namespace derivative::SystemUtil;

using blitz::Array;
using blitz::Range;
using blitz::firstIndex;
using blitz::secondIndex;

namespace derivative
{
	GROUP_REGISTER(EquityBarrierOption);
	ALIAS_REGISTER(EquityBarrierOption, IMessageSink);
	MSG_REGISTER(EquityBarrierOptMessage, EquityBarrierOption);

	EquityBarrierOption::EquityBarrierOption(const Exemplar &ex)
		:EquityOption(),
		m_name(TYPEID)
	{}

	EquityBarrierOption::EquityBarrierOption(const Name& nm)
		: EquityOption(),
		m_name(nm)
	{}

	std::shared_ptr<IMake> EquityBarrierOption::Make(const Name &nm)
	{
		/// Construct EquityBarrierOption from given name and register with EntityManager
		std::shared_ptr<EquityBarrierOption> optionProc = std::make_shared<EquityBarrierOption>(nm);

		/// now register the object
		EntityMgrUtil::registerObject(nm, optionProc);

		/// return constructed object if no exception is thrown
		return optionProc;
	}

	std::shared_ptr<IMake> EquityBarrierOption::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("not implemented");
	}

	void EquityBarrierOption::Dispatch(std::shared_ptr<IMessage>& msg)
	{
		std::shared_ptr<EquityBarrierOptMessage> optMsg = std::dynamic_pointer_cast<EquityBarrierOptMessage>(msg);

		/// transfer request inputs to member variables 
		m_optType = (optMsg->GetRequest().option == EquityBarrierOptMessage::CALL) ? 1 : -1;
		m_barrierType = optMsg->GetRequest().barrierType;
		m_maturity = optMsg->GetRequest().maturity;
		m_strike = optMsg->GetRequest().strike;
		m_barrier = optMsg->GetRequest().barrier;
		/// get stock value.
		m_stockVal = PrimaryUtil::getStockValue(optMsg->GetRequest().underlying);

		ProcessVol(optMsg);
		ProcessRate(optMsg);

		/// now construct the BlackScholesAdapter from the stock value.
		m_stock = std::make_shared<BlackScholesAssetAdapter>(m_stockVal, m_vol);

		/// get the pricing method and run 
		EquityBarrierOptMessage::BarrierResponse res;
		dd::date today = dd::day_clock::local_day();
		auto t = double((m_maturity - dd::day_clock::local_day()).days()) / 365;
		res.optPrice = m_stock->option(t, m_strike, m_termRate, m_optType);
		if (optMsg->GetRequest().method == EquityBarrierOptMessage::MONTE_CARLO)
		{
			res.barierOptPrice = BarrierOptionPricer::ValueWithMC(m_stock, m_term, m_barrierType, m_maturity, \
				m_strike, m_barrier, static_cast<BarrierOptionPricer::VanillaOptionType>(m_optType));
		}
		else
		{
			throw std::invalid_argument("Invalid pricing method");
		}

		/// set the futures info
		res.underlyingTradeDate = m_stockVal->GetTradeDate();
		res.underlyingTradePrice = m_stockVal->GetTradePrice();

		/// set the message;
		optMsg->SetResponse(res);
	}
}
