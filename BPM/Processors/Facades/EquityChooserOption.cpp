/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#include <functional>
#include <random/normal.h>
#include <boost/math/distributions/normal.hpp>

#include "EquityChooserOption.hpp"
#include "DException.hpp"
#include "SystemUtil.hpp"
#include "EntityMgrUtil.hpp"
#include "MsgRegister.hpp"
#include "GroupRegister.hpp"
#include "EquityChooserOptMessage.hpp"

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
	GROUP_REGISTER(EquityChooserOption);
	ALIAS_REGISTER(EquityChooserOption, IMessageSink);
	MSG_REGISTER(EquityChooserOptMessage, EquityChooserOption);

	EquityChooserOption::EquityChooserOption(const Exemplar &ex)
		:EquityOption(),
		m_name(TYPEID)
	{}

	EquityChooserOption::EquityChooserOption(const Name& nm)
		: EquityOption(),
		m_name(nm)
	{}

	std::shared_ptr<IMake> EquityChooserOption::Make(const Name &nm)
	{
		/// Construct EquityChooserOption from given name and register with EntityManager
		std::shared_ptr<EquityChooserOption> optionProc = std::make_shared<EquityChooserOption>(nm);

		/// now register the object
		EntityMgrUtil::registerObject(nm, optionProc);

		/// return constructed object if no exception is thrown
		return optionProc;
	}

	std::shared_ptr<IMake> EquityChooserOption::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("not implemented");
	}
	
	void EquityChooserOption::Dispatch(std::shared_ptr<IMessage>& msg)
	{
		std::shared_ptr<EquityChooserOptMessage> optMsg = std::dynamic_pointer_cast<EquityChooserOptMessage>(msg);

		/// transfer request inputs to member variables 
		m_optType = (optMsg->GetRequest().option == EquityChooserOptMessage::CALL) ? 1 : -1;
		m_maturity = optMsg->GetRequest().maturity;
		m_strike = optMsg->GetRequest().strike;
		/// get stock value.
		m_stockVal = PrimaryUtil::getStockValue(optMsg->GetRequest().underlying);

		ProcessVol(optMsg);
		ProcessRate(optMsg);
		
		/// now construct the BlackScholesAdapter from the stock value.
		m_stock = std::make_shared<BlackScholesAssetAdapter>(m_stockVal, m_vol);

		/// get the pricing method and run 
		EquityChooserOptMessage::ChooserOptResponse res;
		auto t = double((m_maturity - dd::day_clock::local_day()).days()) / 365;
		res.optPrice = m_stock->option(t, m_strike, m_termRate, m_optType);
		if (optMsg->GetRequest().method == EquityChooserOptMessage::MONTE_CARLO)
		{
			res.chooserOptPrice = ChooserOptionPricer::ValueWithMC(m_stock, m_term, m_maturity, m_strike);			
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
