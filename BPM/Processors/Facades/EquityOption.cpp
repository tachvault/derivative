/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#include "EquityOption.hpp"
#include "DException.hpp"

#include "IStock.hpp"
#include "IStockValue.hpp"
#include "BlackScholesAssetAdapter.hpp"
#include "PrimaryAssetUtil.hpp"
#include "IRCurve.hpp"

#include "ConstVol.hpp"
#include "PiecewiseVol.hpp"
#include "EquityVolatilitySurface.hpp"
#include "EquityGARCH.hpp"
#include "SystemUtil.hpp"

#include "VanillaOptMessage.hpp"
#include "EquityAssetPricer.hpp"

using namespace derivative;
using namespace derivative::SystemUtil;

using blitz::Array;
using blitz::Range;
using blitz::firstIndex;
using blitz::secondIndex;

namespace derivative
{
	EquityOption::EquityOption()
		:m_stockVal(nullptr),
		m_stock(nullptr),
		m_vol(nullptr),
		m_term(nullptr)
	{}

	void EquityOption::Activate(const std::deque<boost::any>& agrs)
	{}

	void EquityOption::Passivate()
	{
		m_stockVal = nullptr;
		m_stock = nullptr;
		m_vol = nullptr;
		m_term = nullptr;
	}

	void EquityOption::ProcessVol(const std::shared_ptr<VanillaOptMessage>& optMsg)
	{
		dd::date today = dd::day_clock::local_day();
		/// get the volatility. If greater than zero then use this const vol
		if (optMsg->GetRequest().vol > 0)
		{
			m_vol = std::make_shared<ConstVol>(optMsg->GetRequest().vol);
		}
		else
		{
			if (optMsg->GetRequest().volType == VanillaOptMessage::IV)
			{
				std::shared_ptr<EquityVolatilitySurface> volSurface = BuildEquityVolSurface(optMsg->GetRequest().underlying, today);

				try
				{
					// first try Vol surface
					m_vol = volSurface->GetVolatility(m_maturity, m_strike);
				}
				catch (std::domain_error& e)
				{
					/// means for the maturity not enough data in historic vol
					/// we use GramCharlier to construct constant vol for the given maturity and strike
					m_vol = volSurface->GetConstVol(m_maturity, m_strike);
				}
			}
			else
			{
				std::shared_ptr<EquityGARCH> garch = BuildEquityGARCH(optMsg->GetRequest().underlying, today);
				try
				{
					m_vol = garch->GetVolatility();
				}
				catch (std::domain_error& e)
				{
					cout << "Error " << e.what() << endl;
					throw e;
				}
			}
		}
	}

	void EquityOption::ProcessRate(const std::shared_ptr<VanillaOptMessage>& optMsg)
	{
		dd::date today = dd::day_clock::local_day();
		/// get the interest rate
		auto t = double((m_maturity - dd::day_clock::local_day()).days()) / 365;
		if (optMsg->GetRequest().rateType == VanillaOptMessage::LIBOR)
		{
			/// Get domestic interest rate of the stock
			std::shared_ptr<IRCurve> irCurve = BuildIRCurve(IRCurve::LIBOR, m_stockVal->GetStock()->GetCountry().GetCode(), today);
			m_term = irCurve->GetTermStructure();
			m_termRate = m_term->simple_rate(0, t);
		}
		else
		{
			/// Get domestic interest rate of the stock
			std::shared_ptr<IRCurve> irCurve = BuildIRCurve(IRCurve::YIELD, m_stockVal->GetStock()->GetCountry().GetCode(), today);
			m_term = irCurve->GetTermStructure();
			m_termRate = m_term->simple_rate(0, t);
		}
	}
}