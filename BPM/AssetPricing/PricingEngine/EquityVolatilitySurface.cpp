/*
Modified and ported: 2013, Nathan Muruganantha. All rights reserved.
*/

#include <future>
#include "GroupRegister.hpp"
#include "EntityManager.hpp"
#include "DException.hpp"
#include "EquityVolatilitySurface.hpp"
#include "PrimaryAssetUtil.hpp"
#include "IStockValue.hpp"
#include "IStock.hpp"
#include "BlackScholesAssetAdapter.hpp"
#include "PiecewiseVol.hpp"
#include "IDailyEquityOptionValue.hpp"
#include "GramCharlierAssetAdapter.hpp"
#include "Country.hpp"
#include "Exchange.hpp"
#include "ConstVol.hpp"

namespace derivative
{
	EquityVolatilitySurface::EquityVolatilitySurface(const Exemplar &ex)
		:m_name(TYPEID)
	{
	}

	EquityVolatilitySurface::EquityVolatilitySurface(const string& symbol, const std::shared_ptr<IAssetValue>& asset, const dd::date& processDate)
		: VolatilitySurface(AssetClassTypeEnum::EQUITY, symbol, asset, processDate),
		m_name(ConstructName(symbol, processDate))
	{			
	}

	std::shared_ptr<DeterministicAssetVol> EquityVolatilitySurface::GetConstVol(const dd::date& mat, double strike) const
	{
		if (m_options.empty()) throw std::domain_error("No historical option data found");

		dd::date today(dd::day_clock::local_day());
		int maturity = (mat - today).days();
		double r = PrimaryUtil::FindInterestRate(m_cntry.GetCode(), (double)maturity / 365, IRCurve::LIBOR);

		double domestic_discount = exp(-r*((double)maturity / 365));
		double foreign_discount = 1.0;

		return VolatilitySurface::GetConstVol(mat, strike, domestic_discount, foreign_discount);
	}

	void EquityVolatilitySurface::LoadOptions()
	{		
		/// get the m_options for the given underlying
		/// get the m_options for all maturities.. pass matyrity as 0;
		dd::date today;
		int maturity = 0;
		PrimaryUtil::FindOptionValues<IDailyEquityOptionValue, IDailyOptionValue>(m_symbol, today, maturity, m_options);

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

	std::shared_ptr<EquityVolatilitySurface> BuildEquityVolSurface(const std::string& symbol, const dd::date& edate)
	{
		/// register the exemplar here until the bug on visual C++ is fixed.
		GroupRegister SurfaceGrp(EquityVolatilitySurface::TYPEID, std::make_shared<EquityVolatilitySurface>(Exemplar()));

		Name curName = EquityVolatilitySurface::ConstructName(symbol, edate);
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
			std::shared_ptr<EquityVolatilitySurface> surface = dynamic_pointer_cast<EquityVolatilitySurface>(obj);
			return surface;
		}

		/// if not in registry, then construct the volatility surface
		std::shared_ptr<IStockValue> assetVal = PrimaryUtil::getStockValue(symbol);
		std::shared_ptr<EquityVolatilitySurface>  surface = std::make_shared<EquityVolatilitySurface>(symbol, assetVal,edate);

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