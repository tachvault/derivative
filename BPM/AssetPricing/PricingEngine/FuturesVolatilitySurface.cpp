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
#include "VolatilitySmile.hpp"
#include "IDailyFuturesOptionValue.hpp"
#include "Exchange.hpp"

namespace derivative
{
	FuturesVolatilitySurface::FuturesVolatilitySurface(const Exemplar &ex)
		:m_name(TYPEID)
	{
	}

	FuturesVolatilitySurface::FuturesVolatilitySurface(const string& symbol, const dd::date& processDate, const dd::date& deliveryDate)
		: m_name(ConstructName(symbol, processDate, deliveryDate)),
		m_symbol(symbol),
		m_processedDate(processDate),
		m_deliveryDate(deliveryDate)
	{}

	std::shared_ptr<DeterministicAssetVol> FuturesVolatilitySurface::GetVolByStrike(double strike)
	{
		if (m_vol.empty())
		{
			throw std::domain_error("No option data found");
		}

		if (m_vol.find(strike) != m_vol.end())
		{
			if (m_vol.find(strike)->second == nullptr)
			{
				Build(strike);
			}
			return m_vol.find(strike)->second;
		}

		/// return first if strike is less than min strike
		if (m_vol.begin()->first > strike)
		{
			if (m_vol.begin()->second == nullptr)
			{
				Build(strike);
			}
			return m_vol.begin()->second;
		}

		auto it = m_vol.begin();
		for (; it != m_vol.end(); ++it)
		{
			if (it->first > strike)
			{
				break;
			}
		}

		/// if strike is greater than max strike then throw exception
		if (it == m_vol.end())
		{
			LOG(WARNING) << "Cannot extrapolate but return the vol for largest strike price" << endl;
			if (m_vol.rbegin()->second == nullptr)
			{
				Build(m_vol.rbegin()->first);
			}
			return m_vol.rbegin()->second;
		}

		/// otherwise interpolate for each element from the closest
		auto rit = --it;
		if (strike - rit->first < it->first - strike)
		{
			/// interpolate by taking the rit's timeline as the master 
			//std::shared_ptr<DeterministicAssetVol> vol = rit->second->Clone();
			//vol->interpolate(it->second->Clone(), double(it->first - strike) / (strike - rit->first));
			//return vol;
			if (rit->second == nullptr)
			{
				Build(rit->first);
			}
			return rit->second;
		}
		else
		{
			/// interpolate by it's timeline as the master
			//std::shared_ptr<DeterministicAssetVol> vol = it->second->Clone();
			//vol->interpolate(rit->second->Clone(), double(strike - rit->first) / (it->first - strike));
			//return vol;
			if (it->second == nullptr)
			{
				Build(it->first);
			}
			return it->second;
		}
	}

	/// NATHAN -> need to be tested.
	std::unique_ptr<VolatilitySmile> FuturesVolatilitySurface::GetVolByMaturity(double mat) const
	{
		std::map<int, double> volSmile;
		Array<int, 1> K(m_vol.size());
		Array<double, 1> vols(m_vol.size());

		int i = 0;
		for (auto &vol : m_vol)
		{
			Array<double, 1> vol_lvl(1);
			bool status = vol.second->get_volatility_level(0, mat, vol_lvl);
			K(i) = vol.first;
			vols(i) = vol_lvl(0);
		}

		std::unique_ptr<VolatilitySmile> smile(new VolatilitySmile(K, vols));
		return smile;
	}

	void FuturesVolatilitySurface::LoadOptions()
	{
		/// get the asset from symbol
		std::shared_ptr<IFuturesValue> asset = PrimaryUtil::getFuturesValue(m_symbol, m_processedDate, m_deliveryDate);

		/// get the m_options for the given underlying
		/// get the m_options for all maturities.. pass matyrity as 0;
		dd::date today;
		PrimaryUtil::FindOptionValues<IDailyFuturesOptionValue>(asset->GetFutures()->GetSymbol(), today, m_deliveryDate, m_options);

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
	}


	void FuturesVolatilitySurface::Build(double strike)
	{
		/// get the asset from symbol
		std::shared_ptr<IFuturesValue> asset = PrimaryUtil::getFuturesValue(m_symbol, m_processedDate, m_deliveryDate);

		/// construct black scholes asset
		/// NATHAN: FIXME
		std::shared_ptr<DeterministicAssetVol>  v; // = asset->GetVolatility()->Clone();

		/// now construct the BlackScholesAdapter from the stock value.
		std::shared_ptr<BlackScholesAssetAdapter> bsasset = \
			std::make_shared<BlackScholesAssetAdapter>(asset, v);

		/// count the number of options for the given strike
		/// it is required to initialize Array
		int count = 0;
		for (auto& option : m_options)
		{
			if (option->GetStrikePrice() == strike)
			{
				++count;
			}
		}

		/// for each strike, build the DeterministicVol
		/// insert into surface map.
		auto exchange = asset->GetFutures()->GetExchange();
		auto cntry = exchange.GetCountry().GetCode();
		std::shared_ptr<IRCurve> irCurve = BuildIRCurve(IRCurve::LIBOR, cntry, m_processedDate);
		auto term = irCurve->GetTermStructure();
		Array<double, 1> timeline(count + 1);
		Array<double, 2> vols(count, 1);
		timeline(0) = 0.0;
		int i = 0;
		std::vector<std::future<double>> futures;
		futures.reserve(count);
		for (auto it = m_options.begin(); it < m_options.end(); ++it)
		{
			if ((*it)->GetStrikePrice() == strike)
			{
				auto numDays = ((*it)->GetDeliveryDate() - (*it)->GetTradeDate()).days();
				timeline(i + 1) = double(numDays) / 365;
				double r = PrimaryUtil::getDFToCompoundRate((*term)(timeline(i + 1)), timeline(i + 1));
				auto sign = ((*it)->GetOptionType() == IDailyOptionValue::VANILLA_CALL) ? 1 : -1;
		     	futures.push_back(std::move(std::async(&BlackScholesAssetAdapter::CalculateImpliedVolatility, bsasset, (*it)->GetTradePrice(), timeline(i + 1), (*it)->GetStrikePrice(), r, sign)));
				++i;
			}
		}
		i = 0;
		for (auto &val : futures)
		{
			vols(i, 0) = val.get();
			++i;
		}

		/// now construct DeterministicAsset.
		if (timeline.size() > 0)
		{
			std::shared_ptr<DeterministicAssetVol> assetVol = std::make_shared<PiecewiseConstVol>(timeline, vols);
			m_vol.at(strike) = assetVol;
			LOG(INFO) << " K = " << strike << ", Timeline " << timeline << ", vol " << vols << endl;
			cout << " K = " << strike << ", Timeline " << timeline << ", vol " << vols << endl;
		}
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
		std::shared_ptr<FuturesVolatilitySurface>  surface = std::make_shared<FuturesVolatilitySurface>(symbol, edate, ddate);

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