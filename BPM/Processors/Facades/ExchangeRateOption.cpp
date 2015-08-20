/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#include "ExchangeRateOption.hpp"
#include "DException.hpp"

#include "IExchangeRate.hpp"
#include "IExchangeRateValue.hpp"
#include "BlackScholesAssetAdapter.hpp"
#include "PrimaryAssetUtil.hpp"
#include "IRCurve.hpp"
#include "CountryHolder.hpp"

#include "ConstVol.hpp"
#include "PiecewiseVol.hpp"
#include "ExchangeRateGARCH.hpp"
#include "SystemUtil.hpp"

#include "VanillaOptMessage.hpp"
#include "ExchangeRateAssetPricer.hpp"

using namespace derivative;
using namespace derivative::SystemUtil;

using blitz::Array;
using blitz::Range;
using blitz::firstIndex;
using blitz::secondIndex;

namespace derivative
{
	ExchangeRateOption::ExchangeRateOption()
		:m_exchangeRateVal(nullptr),
		m_exchangeRate(nullptr),
		m_vol(nullptr),
		m_termLocal(nullptr),
	    m_termForeign(nullptr),
	    m_localRate(0),
	    m_foreignRate(0)
	{}

	void ExchangeRateOption::Activate(const std::deque<boost::any>& agrs)
	{}

	void ExchangeRateOption::Passivate()
	{
		m_exchangeRateVal = nullptr;
		m_exchangeRate = nullptr;
		m_termLocal = nullptr;
		m_termForeign = nullptr;
		m_localRate = 0;
		m_foreignRate = 0;
	}

	void ExchangeRateOption::ProcessVol(const std::shared_ptr<VanillaOptMessage>& optMsg)
	{
		dd::date today = dd::day_clock::local_day();
		/// get the volatility. If greater than zero then use this const vol
		if (optMsg->GetRequest().vol > 0)
		{
			m_vol = std::make_shared<ConstVol>(optMsg->GetRequest().vol);
		}
		else
		{
			/// get the historical volatility
			std::shared_ptr<ExchangeRateGARCH> garch = BuildExchangeRateGARCH(optMsg->GetRequest().domestic, optMsg->GetRequest().foreign, today);
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

	void ExchangeRateOption::ProcessRate(const std::shared_ptr<VanillaOptMessage>& optMsg)
	{
		/// get the interest rate for the delivery time (Not for the option maturity)
		if (optMsg->GetRequest().rateType == VanillaOptMessage::LIBOR)
		{
			GetRates(optMsg, IRCurve::LIBOR);
		}
		else
		{
			GetRates(optMsg, IRCurve::YIELD);
		}
	}


	void ExchangeRateOption::GetRates(const std::shared_ptr<VanillaOptMessage>& optMsg, IRCurve::DataSourceType src)
	{
		dd::date today = dd::day_clock::local_day();
		auto t = double((m_maturity - dd::day_clock::local_day()).days()) / 365;
		/// Get domestic interest rate of the ExchangeRate
		CountryHolder& cntryHolder = CountryHolder::getInstance();
		std::vector<Country> vec;
		cntryHolder.GetCountry(optMsg->GetRequest().domestic, vec);

		if (vec.empty()) throw std::invalid_argument("Invalid domestic country code");

		std::shared_ptr<IRCurve> irCurve = BuildIRCurve(src, vec.begin()->GetCode(), today);
		m_termLocal = irCurve->GetTermStructure();
		m_localRate = m_termLocal->simple_rate(0, t);

		/// Get foreign interest rate of the ExchangeRate
		cntryHolder.GetCountry(optMsg->GetRequest().foreign, vec);

		if (vec.empty()) throw std::invalid_argument("Invalid foreign country code");

		irCurve = BuildIRCurve(src, vec.begin()->GetCode(), today);
		m_termForeign = irCurve->GetTermStructure();
		m_foreignRate = m_termForeign->simple_rate(0, t);
	}

	void ExchangeRateOption::ValidateResponse(const std::shared_ptr<VanillaOptMessage>& optMsg)
	{
		const VanillaOptMessage::Response& res = optMsg->GetResponse();
		if (res.optPrice != res.optPrice)
		{
			throw std::exception("Unable to price this option");
		}
	}
}