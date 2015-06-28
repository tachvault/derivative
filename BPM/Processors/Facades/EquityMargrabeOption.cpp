/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#include <functional>
#include <thread>
#include <random/normal.h>
#include <boost/math/distributions/normal.hpp>

#include "EquityMargrabeOption.hpp"
#include "DException.hpp"
#include "SystemUtil.hpp"
#include "EntityMgrUtil.hpp"
#include "MsgRegister.hpp"
#include "GroupRegister.hpp"
#include "EquityMargrabeOptMessage.hpp"

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
	GROUP_REGISTER(EquityMargrabeOption);
	ALIAS_REGISTER(EquityMargrabeOption, IMessageSink);
	MSG_REGISTER(EquityMargrabeOptMessage, EquityMargrabeOption);

	EquityMargrabeOption::EquityMargrabeOption(const Exemplar &ex)
		:EquityOption(),
		m_name(TYPEID)
	{}

	EquityMargrabeOption::EquityMargrabeOption(const Name& nm)
		: EquityOption(),
		m_name(nm)
	{}

	std::shared_ptr<IMake> EquityMargrabeOption::Make(const Name &nm)
	{
		/// Construct EquityMargrabeOption from given name and register with EntityManager
		std::shared_ptr<EquityMargrabeOption> optionProc = std::make_shared<EquityMargrabeOption>(nm);

		/// now register the object
		EntityMgrUtil::registerObject(nm, optionProc);

		/// return constructed object if no exception is thrown
		return optionProc;
	}
	
	void EquityMargrabeOption::ProcessVol(const std::shared_ptr<VanillaOptMessage>& msg)
	{
		std::shared_ptr<EquityMargrabeOptMessage> optMsg = std::dynamic_pointer_cast<EquityMargrabeOptMessage>(msg);

		dd::date today = dd::day_clock::local_day();
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

			std::shared_ptr<EquityVolatilitySurface> volNumeraireSurface = BuildEquityVolSurface(optMsg->GetRequest().numeraire, today);
			try
			{
				// first try Vol surface
				m_numeraireVol = volNumeraireSurface->GetVolatility(m_maturity, m_strike);
			}
			catch (std::domain_error& e)
			{
				/// means for the maturity not enough data in historic vol
				/// we use GramCharlier to construct constant vol for the given maturity and strike
				m_numeraireVol = volNumeraireSurface->GetConstVol(m_maturity, m_strike);
			}
		}
		else
		{
			std::shared_ptr<EquityGARCH> garch = BuildEquityGARCH(optMsg->GetRequest().numeraire, today);
			try
			{
				m_numeraireVol = garch->GetVolatility();
			}
			catch (std::domain_error& e)
			{
				cout << "Error " << e.what() << endl;
				throw e;
			}
		}
	}

	std::shared_ptr<IMake> EquityMargrabeOption::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("not implemented");
	}

	void EquityMargrabeOption::Dispatch(std::shared_ptr<IMessage>& msg)
	{
		std::shared_ptr<EquityMargrabeOptMessage> optMsg = std::dynamic_pointer_cast<EquityMargrabeOptMessage>(msg);

		/// transfer request inputs to member variables 
		m_optType = (optMsg->GetRequest().option == EquityMargrabeOptMessage::CALL) ? 1 : -1;
		m_symbol = optMsg->GetRequest().underlying;
		m_numeraireSymbol = optMsg->GetRequest().numeraire;
		m_maturity = optMsg->GetRequest().maturity;
		m_strike = optMsg->GetRequest().strike;
		/// get stock value.
		m_stockVal = PrimaryUtil::getStockValue(optMsg->GetRequest().underlying);
		
		m_numeraireStockVal = PrimaryUtil::getStockValue(optMsg->GetRequest().numeraire);
		
		ProcessVol(optMsg);
		ProcessRate(optMsg);

		/// now construct the BlackScholesAdapter from the stock value.
		m_stock = std::make_shared<BlackScholesAssetAdapter>(m_stockVal, m_vol);
		m_numeraireStock = std::make_shared<BlackScholesAssetAdapter>(m_numeraireStockVal, m_numeraireVol);
		
		/// get the pricing method and run 
		EquityMargrabeOptMessage::Response res;
		dd::date today = dd::day_clock::local_day();
		auto t = double((m_maturity - dd::day_clock::local_day()).days()) / 365;
		if (optMsg->GetRequest().method == VanillaOptMessage::CLOSED)
		{
			res.optPrice = MargrabeOptionPricer::ValueEuropeanClosedForm(m_stock, m_numeraireStock, m_term, m_maturity, \
				m_strike, static_cast<VanillaOptionPricer::VanillaOptionType>(m_optType));
		}
		else if (optMsg->GetRequest().method == VanillaOptMessage::MONTE_CARLO)
		{
			if (optMsg->GetRequest().style == VanillaOptMessage::EUROPEAN)
			{
				res.optPrice = MargrabeOptionPricer::ValueEuropeanWithMC(m_stock, m_numeraireStock, m_term, m_maturity, \
					m_strike, static_cast<VanillaOptionPricer::VanillaOptionType>(m_optType));
			}
			else
			{
				res.optPrice = MargrabeOptionPricer::ValueAmericanWithMC(m_stock, m_numeraireStock, m_term, m_maturity, \
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

		/// set the message;
		optMsg->SetResponse(res);
	}
}
