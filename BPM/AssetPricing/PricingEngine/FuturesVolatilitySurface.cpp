/*
Modified and ported: 2013, Nathan Muruganantha. All rights reserved.
*/

#include <future>
#include "GroupRegister.hpp"
#include "EntityManager.hpp"
#include "DException.hpp"
#include "FuturesVolatilitySurface.hpp"
#include "PrimaryAssetUtil.hpp"
#include "IFuturesValue.hpp"
#include "IFutures.hpp"
#include "BlackScholesAssetAdapter.hpp"
#include "PiecewiseVol.hpp"
#include "IDailyFuturesOptionValue.hpp"
#include "Exchange.hpp"

namespace derivative
{
	FuturesVolatilitySurface::FuturesVolatilitySurface(const Exemplar &ex)
		:m_name(TYPEID)
	{
	}

	FuturesVolatilitySurface::FuturesVolatilitySurface(const string& symbol, const std::shared_ptr<IAssetValue>& asset, const dd::date& processDate, const dd::date& deliveryDate)
		: VolatilitySurface(AssetClassTypeEnum::FUTURES, symbol, asset, processDate),
		m_name(ConstructName(symbol, processDate, deliveryDate)),
		m_deliveryDate(deliveryDate)
	{}

	std::shared_ptr<DeterministicAssetVol> FuturesVolatilitySurface::GetConstVol(const dd::date& mat, double strike) const
	{
		if (m_options.empty()) throw std::domain_error("No historical option data found");

		dd::date today(dd::day_clock::local_day());
		int maturity = (mat - today).days();
		double r = PrimaryUtil::FindInterestRate(m_cntry.GetCode(), (double)maturity / 365, IRCurve::LIBOR);

		double domestic_discount = exp(-r*((double)maturity / 365));
		double foreign_discount = exp(-r*((double)maturity / 365));

		return VolatilitySurface::GetConstVol(mat, strike, domestic_discount, foreign_discount);
	}

	void FuturesVolatilitySurface::LoadOptions()
	{
		/// get the asset from symbol
		std::shared_ptr<IFuturesValue> asset = PrimaryUtil::getFuturesValue(m_symbol, m_processedDate, m_deliveryDate);

		/// get the m_options for the given underlying
		/// get the m_options for all maturities.. pass matyrity as 0;
		dd::date today;
		PrimaryUtil::FindOptionValues<IDailyFuturesOptionValue, IDailyOptionValue>(asset->GetFutures()->GetSymbol(), today, m_deliveryDate, m_options);

		if (m_options.empty())
		{
			LOG(ERROR) << "No option data found " << endl;
			throw std::domain_error("No option data");
		}

		/// create m_vol map with empty values (volatilities)
		for (auto opt : m_options)
		{
			if (m_vol.find(opt->GetStrikePrice()) == m_vol.end())
			{
				m_vol.insert(std::make_pair(opt->GetStrikePrice(), nullptr));
			}
		}

		/// sort the options by maturity date so that they can easily used in Graham Charlier.
		sort(m_options.begin(), m_options.end(),
			[](const std::shared_ptr<IDailyOptionValue> & a, const std::shared_ptr<IDailyOptionValue> & b)
		{
			return a->GetMaturityDate() < b->GetMaturityDate();
		});
	}
	
	std::shared_ptr<FuturesVolatilitySurface> BuildFuturesVolSurface(const std::string& symbol, const dd::date& edate, const dd::date& ddate)
	{
		/// register the exemplar here until the bug on visual C++ is fixed.
		GroupRegister FuturesSurfaceGrp(FuturesVolatilitySurface::TYPEID, std::make_shared<FuturesVolatilitySurface>(Exemplar()));

		Name curName = FuturesVolatilitySurface::ConstructName(symbol, edate, ddate);
		/// check if this object is already registered with EntityManager

		std::shared_ptr<IObject> obj;
		EntityManager& entMgr = EntityManager::getInstance();
		/// check if the object is already registered with EntityManager
		try
		{
			obj = entMgr.findObject(curName);
		}
		catch (RegistryException& e)
		{
			LOG(INFO) << e.what();
		}
		/// if the object is found then register return
		if (obj)
		{
			std::shared_ptr<FuturesVolatilitySurface> surface = dynamic_pointer_cast<FuturesVolatilitySurface>(obj);
			return surface;
		}

		/// if not in registry, then construct the volatility surface
		std::shared_ptr<IAssetValue> asset = PrimaryUtil::getFuturesValue(symbol, edate, ddate);
		std::shared_ptr<FuturesVolatilitySurface>  surface = std::make_shared<FuturesVolatilitySurface>(symbol, asset, edate, ddate);

		try
		{
			/// Build the Volatility Surface
			surface->LoadOptions();

			/// if sucessfully loaded all options (without any exception)
			/// register with EntityManager
			entMgr.registerObject(surface->GetName(), surface);
		}
		catch (RegistryException& e)
		{
			LOG(ERROR) << "Throw exception.. The object not in registry and unable to register or build" << endl;
			LOG(ERROR) << e.what();
			throw e;
		}
		return surface;
	}
}