/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#include <functional>
#include <random/normal.h>
#include <boost/math/distributions/normal.hpp>

#include "EquityAverageOption.hpp"
#include "DException.hpp"
#include "SystemUtil.hpp"
#include "EntityMgrUtil.hpp"
#include "MsgRegister.hpp"
#include "GroupRegister.hpp"
#include "EquityAverageOptMessage.hpp"

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
	GROUP_REGISTER(EquityAverageOption);
	ALIAS_REGISTER(EquityAverageOption, IMessageSink);
	MSG_REGISTER(EquityAverageOptMessage, EquityAverageOption);

	EquityAverageOption::EquityAverageOption(const Exemplar &ex)
		:EquityOption(),
		m_name(TYPEID)
	{}

	EquityAverageOption::EquityAverageOption(const Name& nm)
		: EquityOption(),
		m_name(nm)
	{}

	std::shared_ptr<IMake> EquityAverageOption::Make(const Name &nm)
	{
		/// Construct EquityAverageOption from given name and register with EntityManager
		std::shared_ptr<EquityAverageOption> optionProc = std::make_shared<EquityAverageOption>(nm);

		/// now register the object
		EntityMgrUtil::registerObject(nm, optionProc);

		/// return constructed object if no exception is thrown
		return optionProc;
	}

	std::shared_ptr<IMake> EquityAverageOption::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("not implemented");
	}

	void EquityAverageOption::Dispatch(std::shared_ptr<IMessage>& msg)
	{
		std::shared_ptr<EquityAverageOptMessage> optMsg = std::dynamic_pointer_cast<EquityAverageOptMessage>(msg);

		/// transfer request inputs to member variables 
		m_optType = (optMsg->GetRequest().option == EquityAverageOptMessage::CALL) ? 1 : -1;
		m_averageType = optMsg->GetRequest().averageType;
		m_maturity = optMsg->GetRequest().maturity;
		/// get stock value.
		m_stockVal = PrimaryUtil::getStockValue(optMsg->GetRequest().underlying);

		/// get strike price
		if (optMsg->GetRequest().strike == std::numeric_limits<double>::max())
		{
			optMsg->GetRequest().strike = m_stockVal->GetTradePrice();
		}
		m_strike = optMsg->GetRequest().strike;

		ProcessVol(optMsg);
		ProcessRate(optMsg);

		/// get the interest rate
		dd::date today = dd::day_clock::local_day();
		auto t = double((m_maturity - dd::day_clock::local_day()).days()) / 365;

		/// now construct the BlackScholesAdapter from the stock value.
		m_stock = std::make_shared<BlackScholesAssetAdapter>(m_stockVal, m_vol);

		/// get the pricing method and run 
		EquityAverageOptMessage::AverageOptResponse res;
		res.optPrice = m_stock->option(t, m_strike, m_termRate, m_optType);
		if (optMsg->GetRequest().method == EquityAverageOptMessage::MONTE_CARLO)
		{
			res.averageOptPrice = AverageOptionPricer::ValueWithMC(m_stock, \
				m_term, m_averageType, m_maturity, m_strike, static_cast<AverageOptionPricer::VanillaOptionType>(m_optType));
		}
		else
		{
			throw std::invalid_argument("Invalid pricing method");
		}

		/// set the futures info
		double mat = (double((m_maturity - dd::day_clock::local_day()).days())) / 365;
		res.vol = m_stock->GetVolatility(0, mat);
		res.underlyingTradeDate = m_stockVal->GetTradeDate();
		res.underlyingTradePrice = m_stockVal->GetTradePrice();

		/// set the message;
		optMsg->SetResponse(res);
		ValidateResponse(optMsg);
	}
}
