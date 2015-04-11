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

	EquityVolatilitySurface::EquityVolatilitySurface(const string& symbol, const dd::date& processDate)
		: m_name(ConstructName(symbol, processDate)),
		m_symbol(symbol),
		m_processedDate(processDate)
	{
		/// get the asset and construct Black Scholes asset
		m_asset = PrimaryUtil::getStockValue(m_symbol);

		/// get country
		m_cntry = dynamic_pointer_cast<IPrimitiveSecurity>(m_asset->GetStock())->GetExchange().GetCountry();
				
		/// construct black scholes asset
		std::shared_ptr<DeterministicAssetVol>  v;
		m_bsasset = std::make_shared<BlackScholesAssetAdapter>(m_asset, v);		
	}

	std::shared_ptr<DeterministicAssetVol> EquityVolatilitySurface::GetVolatility(const dd::date& mat, double strike, bool exactMatch)
	{
		if (m_vol.empty())
		{
			throw std::domain_error("No historic option market data found");
		}

		if (m_vol.find(strike) != m_vol.end())
		{
			if (m_vol.find(strike)->second == nullptr)
			{
				Build(strike);
			}
			return m_vol.find(strike)->second;
		}

		/// if options not found for given strike then throw
		if (exactMatch) throw std::domain_error("No applicable option data found");
		
		/// otherwise we will start approximate
		/// return first if strike is less than min strike
		if (m_vol.begin()->first > strike)
		{
			if (m_vol.begin()->second == nullptr)
			{
				Build(m_vol.begin()->first);
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

		/// if strike is greater then max strike  return vol for max strike
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
		auto rit = it;
		--rit;
		/// interpolate by taking the rit's timeline as the master
		if (rit->second == nullptr)	Build(rit->first);
		if (it->second == nullptr)	Build(it->first);

		/// if either of them does not have go up to maturity then throw exception
		dd::date today(dd::day_clock::local_day());
		int maturity = (mat - today).days();
		auto x = rit->second->timeframe();
		auto y = it->second->timeframe();
		if ((rit->second->timeframe() < ((double)maturity / 365)) || (it->second->timeframe() < ((double)maturity / 365)))
		{
			throw std::domain_error("not enough option data with given maturity");
		}

		std::shared_ptr<DeterministicAssetVol> vol = rit->second->Clone();
		std::shared_ptr<DeterministicAssetVol> neibor = it->second->Clone();
		try
		{
			vol->interpolate(neibor, double(strike - rit->first) / (it->first - rit->first));
			return vol;
		}
		catch (std::exception& e) /// out_of_range or logic_error
		{
			if (strike - rit->first < it->first - strike)
			{
				return rit->second;
			}
			else
			{
				return it->second;
			}
		}
	}

	std::shared_ptr<DeterministicAssetVol> EquityVolatilitySurface::GetConstVol(const dd::date& mat, double strike) const
	{
		if (m_options.empty()) throw std::domain_error("No historical option data found");

		/// Get subset of options applicable for this maturity
		std::vector<std::shared_ptr<IDailyOptionValue> > options;
		dd::date prev, next, close;
		auto it = m_options.begin();
		prev = (*it)->GetMaturityDate();
		if (prev > mat)
		{
			double vol = GetVolByGramCharlier(prev, strike);
			std::shared_ptr<DeterministicAssetVol> v = std::make_shared<ConstVol>(vol);
			return v;
		}

		for(;it < m_options.end(); ++it)
		{
			if ((*it)->GetMaturityDate() > mat)
			{
				next = (*it)->GetMaturityDate();
				break;
			}
			else if ((*it)->GetMaturityDate() == mat)
			{
				next = (*it)->GetMaturityDate();
				prev = (*it)->GetMaturityDate(); 
				break;

			}
			prev = (*it)->GetMaturityDate();
		}

		if (prev == next)
		{
			double vol = GetVolByGramCharlier(mat, strike);
			std::shared_ptr<DeterministicAssetVol> v = std::make_shared<ConstVol>(vol);
			return v;
		}
		else if (next.is_not_a_date())
		{
			double vol = GetVolByGramCharlier(prev, strike);
			std::shared_ptr<DeterministicAssetVol> v = std::make_shared<ConstVol>(vol);
			return v;
		}
		else
		{
			double prev_vol = GetVolByGramCharlier(prev, strike);
			double next_vol = GetVolByGramCharlier(next, strike);
			auto vol = (double)(next - mat).days() / (next - prev).days()*prev_vol + (double)(mat - prev).days() / (next - prev).days()*next_vol;
			std::shared_ptr<DeterministicAssetVol> v = std::make_shared<ConstVol>(vol);
			return v;
		}
	}

	double EquityVolatilitySurface::GetVolByGramCharlier(const dd::date& mat, double strike) const
	{
		dd::date today(dd::day_clock::local_day());
		int maturity = (mat - today).days();
		double r = PrimaryUtil::FindInterestRate(m_cntry.GetCode(), (double)maturity / 365, IRCurve::LIBOR);

		/// now find all the options with date close
		std::vector<std::shared_ptr<IDailyOptionValue> > options;
		for (auto& opt : m_options)
		{
			if (opt->GetMaturityDate() == mat)
			{
				options.push_back(opt);
			}
		}

		/// Create GramCharlierAsset for the given maturity
		int highest_moment = 4;
		Array<double, 1> coeff(highest_moment + 1);
		coeff = 0.0;
		coeff(0) = 1.0;
		GramCharlier gc(coeff);
		std::shared_ptr<GramCharlierAssetAdapter> gcasset = GramCharlierAssetAdapter::Create(gc, m_asset, maturity, options);

		double domestic_discount = exp(-r*((double)maturity/365));
		double foreign_discount = 1.0;
		gcasset->calibrate(domestic_discount, foreign_discount, highest_moment);
		/// get the call option price using Gram Charlier 
		auto price = gcasset->call(strike, domestic_discount, foreign_discount);

		/// now use this option price with BlackScholes to get vol
		return m_bsasset->CalculateImpliedVolatility(price, double(maturity) / 365, strike, r, 1);
	}

	void EquityVolatilitySurface::LoadOptions()
	{		
		/// get the m_options for the given underlying
		/// get the m_options for all maturities.. pass matyrity as 0;
		dd::date today;
		int maturity = 0;
		PrimaryUtil::FindOptionValues<IDailyEquityOptionValue>(m_asset->GetStock()->GetSymbol(), today, maturity, m_options);

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
			[](const std::shared_ptr<IDailyEquityOptionValue> & a, const std::shared_ptr<IDailyEquityOptionValue> & b)
		{
			return a->GetMaturityDate() < b->GetMaturityDate();
		});
	}


	void EquityVolatilitySurface::Build(double strike)
	{
		
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
				auto numDays = ((*it)->GetMaturityDate() - (*it)->GetTradeDate()).days();
				timeline(i + 1) = double(numDays) / 365;
				double r = PrimaryUtil::FindInterestRate(m_cntry.GetCode(), timeline(i + 1), IRCurve::LIBOR);
				auto sign = ((*it)->GetOptionType() == IDailyOptionValue::VANILLA_CALL) ? 1 : -1;
				futures.push_back(std::move(std::async(&BlackScholesAssetAdapter::CalculateImpliedVolatility, m_bsasset, (*it)->GetTradePrice(), timeline(i + 1), (*it)->GetStrikePrice(), r, sign)));
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
		}
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
		std::shared_ptr<EquityVolatilitySurface>  surface = std::make_shared<EquityVolatilitySurface>(symbol, edate);

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