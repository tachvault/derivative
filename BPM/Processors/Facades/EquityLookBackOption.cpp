/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#include <functional>
#include <random/normal.h>
#include <boost/math/distributions/normal.hpp>

#include "EquityLookBackOption.hpp"
#include "DException.hpp"
#include "SystemUtil.hpp"
#include "EntityMgrUtil.hpp"
#include "MsgRegister.hpp"
#include "GroupRegister.hpp"
#include "EquityLookBackOptMessage.hpp"

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

using namespace derivative;
using namespace derivative::SystemUtil;

using blitz::Array;
using blitz::Range;
using blitz::firstIndex;
using blitz::secondIndex;

namespace derivative
{
	GROUP_REGISTER(EquityLookBackOption);
	ALIAS_REGISTER(EquityLookBackOption, IMessageSink);
	MSG_REGISTER(EquityLookBackOptMessage, EquityLookBackOption);

	EquityLookBackOption::EquityLookBackOption(const Exemplar &ex)
		:EquityOption(),
		m_name(TYPEID)
	{}

	EquityLookBackOption::EquityLookBackOption(const Name& nm)
		: EquityOption(),
		m_name(nm)
	{}

	std::shared_ptr<IMake> EquityLookBackOption::Make(const Name &nm)
	{
		/// Construct EquityLookBackOption from given name and register with EntityManager
		std::shared_ptr<EquityLookBackOption> optionProc = std::make_shared<EquityLookBackOption>(nm);

		/// now register the object
		EntityMgrUtil::registerObject(nm, optionProc);

		/// return constructed object if no exception is thrown
		return optionProc;
	}

	std::shared_ptr<IMake> EquityLookBackOption::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("not implemented");
	}
	
	void EquityLookBackOption::Dispatch(std::shared_ptr<IMessage>& msg)
	{
		std::shared_ptr<EquityLookBackOptMessage> optMsg = std::dynamic_pointer_cast<EquityLookBackOptMessage>(msg);

		/// transfer request inputs to member variables 
		m_optType = (optMsg->GetRequest().option == EquityLookBackOptMessage::CALL) ? 1 : -1;
		m_lookBackType = optMsg->GetRequest().lookBackType;
		m_maturity = optMsg->GetRequest().maturity;
		m_strike = optMsg->GetRequest().strike;
		/// get stock value.
		m_stockVal = PrimaryUtil::getStockValue(optMsg->GetRequest().underlying);

		ProcessVol(optMsg);
		ProcessRate(optMsg);
		
		/// now construct the BlackScholesAdapter from the stock value.
		m_stock = std::make_shared<BlackScholesAssetAdapter>(m_stockVal, m_vol);

		/// get the pricing method and run 
		EquityLookBackOptMessage::LookBackOptResponse res;
		auto t = double((m_maturity - dd::day_clock::local_day()).days()) / 365;
		res.optPrice = m_stock->option(t, m_strike, m_termRate, m_optType);
		if (optMsg->GetRequest().method == EquityLookBackOptMessage::MONTE_CARLO)
		{
			res.lookbackOptPrice = LookBackOptionPricer::ValueWithMC(m_stock, m_term, m_maturity, \
					m_strike, m_lookBackType, static_cast<LookBackOptionPricer::VanillaOptionType>(m_optType));			
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
