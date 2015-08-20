/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#include <functional>
#include <random/normal.h>
#include <boost/math/distributions/normal.hpp>

#include "EquityVanillaOption.hpp"
#include "DException.hpp"
#include "SystemUtil.hpp"
#include "EntityMgrUtil.hpp"
#include "MsgRegister.hpp"
#include "GroupRegister.hpp"
#include "EquityVanillaOptMessage.hpp"

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
	GROUP_REGISTER(EquityVanillaOption);
	ALIAS_REGISTER(EquityVanillaOption, IMessageSink);
	MSG_REGISTER(EquityVanillaOptMessage, EquityVanillaOption);

	EquityVanillaOption::EquityVanillaOption(const Exemplar &ex)
		:EquityOption(),
		m_name(TYPEID)
	{}

	EquityVanillaOption::EquityVanillaOption(const Name& nm)
		: EquityOption(),
		m_name(nm)
	{}

	std::shared_ptr<IMake> EquityVanillaOption::Make(const Name &nm)
	{
		/// Construct EquityVanillaOption from given name and register with EntityManager
		std::shared_ptr<EquityVanillaOption> optionProc = std::make_shared<EquityVanillaOption>(nm);

		/// now register the object
		EntityMgrUtil::registerObject(nm, optionProc);

		/// return constructed object if no exception is thrown
		return optionProc;
	}

	std::shared_ptr<IMake> EquityVanillaOption::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("not implemented");
	}
	
	void EquityVanillaOption::Dispatch(std::shared_ptr<IMessage>& msg)
	{
		std::shared_ptr<EquityVanillaOptMessage> optMsg = std::dynamic_pointer_cast<EquityVanillaOptMessage>(msg);

		/// transfer request inputs to member variables 
		m_optType = (optMsg->GetRequest().option == EquityVanillaOptMessage::CALL) ? 1 : -1;
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
		
		/// now construct the BlackScholesAdapter from the stock value.
		m_stock = std::make_shared<BlackScholesAssetAdapter>(m_stockVal, m_vol);

		/// get the pricing method and run 
		EquityVanillaOptMessage::Response res;
		auto t = double((m_maturity - dd::day_clock::local_day()).days()) / 365;
		if (optMsg->GetRequest().method == EquityVanillaOptMessage::CLOSED)
		{
			res.optPrice = m_stock->option(t, m_strike, m_termRate, m_optType);
		}
		else if (optMsg->GetRequest().method == EquityVanillaOptMessage::LATTICE)
		{
			if (optMsg->GetRequest().style == EquityVanillaOptMessage::EUROPEAN)
			{
				res.optPrice = VanillaOptionPricer::ValueEuropeanWithBinomial(m_stock, m_termRate, m_maturity, \
					m_strike, static_cast<VanillaOptionPricer::VanillaOptionType>(m_optType));
			}
			else
			{
				res.optPrice = VanillaOptionPricer::ValueAmericanWithBinomial(m_stock, m_termRate, m_maturity, \
					m_strike, static_cast<VanillaOptionPricer::VanillaOptionType>(m_optType));
			}
		}
		else if (optMsg->GetRequest().method == EquityVanillaOptMessage::MONTE_CARLO)
		{
			if (optMsg->GetRequest().style == EquityVanillaOptMessage::EUROPEAN)
			{
				res.optPrice = VanillaOptionPricer::ValueEuropeanWithMC(m_stock, m_term, m_maturity, \
					m_strike, static_cast<VanillaOptionPricer::VanillaOptionType>(m_optType));
			}
			else
			{
				res.optPrice = VanillaOptionPricer::ValueAmericanWithMC(m_stock, m_term, m_maturity, \
					m_strike, static_cast<VanillaOptionPricer::VanillaOptionType>(m_optType));
			}
		}
		else
		{
			throw std::invalid_argument("Invalid pricing method");
		}

		/// set the futures info
		res.underlyingTradeDate = m_stockVal->GetTradeDate();
		res.underlyingTradePrice = m_stockVal->GetTradePrice();
		/// now get the greeks
		double mat = (double((m_maturity - dd::day_clock::local_day()).days())) / 365;
		res.greeks.delta = m_stock->delta(mat, m_strike, m_termRate, m_optType);
		res.greeks.gamma = m_stock->gamma(mat, m_strike, m_termRate, m_optType);
		res.greeks.vega = m_stock->vega(mat, m_strike, m_termRate, m_optType);
		res.greeks.theta = m_stock->theta(mat, m_strike, m_termRate, m_optType);
		res.greeks.vega = m_stock->vega(mat, m_strike, m_termRate, m_optType);

		/// set the message;
		optMsg->SetResponse(res);
		ValidateResponse(optMsg);
	}
}
