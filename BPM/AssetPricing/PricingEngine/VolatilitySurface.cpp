/*
Modified and ported: 2013, Nathan Muruganantha. All rights reserved.
*/

#include <future>
#include "GroupRegister.hpp"
#include "EntityManager.hpp"
#include "DException.hpp"
#include "VolatilitySurface.hpp"
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
	VolatilitySurface::VolatilitySurface(AssetClassTypeEnum assetCls, const string& symbol, const std::shared_ptr<IAssetValue>& asset, const dd::date& processDate)
		:m_assetClass(assetCls),
		m_symbol(symbol),
		m_asset(asset),
		m_processedDate(processDate)
	{
		/// get country
		m_cntry = dynamic_pointer_cast<IPrimitiveSecurity>(m_asset->GetAsset())->GetExchange().GetCountry();

		/// construct black scholes asset
		std::shared_ptr<DeterministicAssetVol>  v;
		m_bsasset = std::make_shared<BlackScholesAssetAdapter>(m_asset, v);
	}

	std::shared_ptr<DeterministicAssetVol> VolatilitySurface::GetVolatility(const dd::date& mat, double strike, int rateType, bool exactMatch)
	{
		if (m_vol.empty())
		{
			throw std::domain_error("No historic option market data found");
		}

		int numDays = (mat - m_processedDate).days();
		double maturity = (double)(long long(double(numDays) / 365 * std::pow(10, 15))) / std::pow(10, 15);
		if (m_vol.find(strike) != m_vol.end())
		{
			if (m_vol.find(strike)->second == nullptr)
			{
				Build(strike, rateType);
			}
			if (m_vol.find(strike)->second->timeframe() < maturity)
			{
				throw std::domain_error("not enough option data with given maturity");
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
				Build(m_vol.begin()->first, rateType);
			}
			if (m_vol.begin()->second->timeframe() < maturity)
			{
				throw std::domain_error("not enough option data with given maturity");
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
			auto rit = m_vol.rbegin();
			if (rit->second == nullptr)
			{
				Build(m_vol.rbegin()->first, rateType);
			}
			if (m_vol.rbegin()->second->timeframe() < maturity)
			{
				throw std::domain_error("not enough option data with given maturity");
			}
			return m_vol.rbegin()->second;
		}

		/// otherwise interpolate for each element from the closest
		auto rit = it;
		--rit;
		/// interpolate by taking the rit's timeline as the master
		if (rit->second == nullptr)	Build(rit->first, rateType);
		if (it->second == nullptr)	Build(it->first, rateType);

		auto x = rit->second->timeframe();
		auto y = it->second->timeframe();
		if ((rit->second->timeframe() < maturity) || (it->second->timeframe() < maturity))
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

	std::shared_ptr<DeterministicAssetVol> VolatilitySurface::GetConstVol(const dd::date& mat, double strike, double domestic_discount, double foreign_discount, int rateType) const
	{
		if (m_options.empty()) throw std::domain_error("No historical option data found");

		/// Get subset of options applicable for this maturity
		std::vector<std::shared_ptr<IDailyOptionValue> > options;
		dd::date prev, next, close;
		auto it = m_options.begin();
		prev = (*it)->GetMaturityDate();
		if (prev > mat)
		{
			double vol = GetVolByGramCharlier(prev, strike, domestic_discount, foreign_discount, rateType);
			std::shared_ptr<DeterministicAssetVol> v = std::make_shared<ConstVol>(vol);
			return v;
		}

		for (; it < m_options.end(); ++it)
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
			double vol = GetVolByGramCharlier(mat, strike, domestic_discount, foreign_discount, rateType);
			std::shared_ptr<DeterministicAssetVol> v = std::make_shared<ConstVol>(vol);
			return v;
		}
		else if (next.is_not_a_date())
		{
			double vol = GetVolByGramCharlier(prev, strike, domestic_discount, foreign_discount, rateType);
			std::shared_ptr<DeterministicAssetVol> v = std::make_shared<ConstVol>(vol);
			return v;
		}
		else
		{
			double prev_vol = GetVolByGramCharlier(prev, strike, domestic_discount, foreign_discount, rateType);
			double next_vol = GetVolByGramCharlier(next, strike, domestic_discount, foreign_discount, rateType);
			auto vol = (double)(next - mat).days() / (next - prev).days()*prev_vol + (double)(mat - prev).days() / (next - prev).days()*next_vol;
			std::shared_ptr<DeterministicAssetVol> v = std::make_shared<ConstVol>(vol);
			return v;
		}
	}

	double VolatilitySurface::GetVolByGramCharlier(const dd::date& mat, double strike, double domestic_discount, double foreign_discount, int rateType) const
	{
		int numDays = (mat - m_processedDate).days();
		double maturity = (double)(long long(double(numDays) / 365 * std::pow(10, 15))) / std::pow(10, 15);
		double r = PrimaryUtil::FindInterestRate(m_cntry.GetCode(), maturity, static_cast<IRCurve::DataSourceType>(rateType));

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
		std::shared_ptr<GramCharlierAssetAdapter> gcasset = GramCharlierAssetAdapter::Create(gc, m_asset, numDays, options);

		gcasset->calibrate(domestic_discount, foreign_discount, highest_moment);
		/// get the call option price using Gram Charlier 
		auto price = gcasset->call(strike, domestic_discount, foreign_discount);

		/// now use this option price with BlackScholes to get vol
		return m_bsasset->CalculateImpliedVolatility(price, maturity, strike, r, 1);
	}

	void VolatilitySurface::Build(double strike, int rateType)
	{

		/// count the number of options for the given strike
		/// it is required to initialize Array
		int count = 0;
		int lastMat = 0;
		for (auto& option : m_options)
		{
			if (option->GetStrikePrice() == strike)
			{
				auto numDays = (option->GetMaturityDate() - m_processedDate).days();
				if (lastMat != numDays)
				{
					lastMat = numDays;
					++count;
				}
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
		lastMat = 0;
		for (auto it = m_options.begin(); it < m_options.end(); ++it)
		{
			if ((*it)->GetStrikePrice() == strike)
			{
				auto numDays = ((*it)->GetMaturityDate() - m_processedDate).days();
				if (lastMat == numDays) continue; else lastMat = numDays;
				timeline(i + 1) = (double)(long long(double(numDays) / 365 * std::pow(10, 15))) / std::pow(10, 15);
				double r = PrimaryUtil::FindInterestRate(m_cntry.GetCode(), timeline(i + 1), static_cast<IRCurve::DataSourceType>(rateType));
				auto sign = ((*it)->GetOptionType() == IDailyOptionValue::VANILLA_CALL) ? 1 : -1;
				futures.push_back(std::move(std::async(&BlackScholesAssetAdapter::CalculateImpliedVolatility, m_bsasset, (*it)->GetTradePrice(), timeline(i + 1), (*it)->GetStrikePrice(), r, sign)));
				++i;
			}
		}
		i = 0;
		try
		{
			for (auto &val : futures)
			{
				vols(i, 0) = val.get();
				++i;
			}
		}
		catch (std::exception& e)
		{
			LOG(ERROR) << "Error in volatility surface building " << e.what() << endl;
			throw e;
		}

		/// now construct DeterministicAsset.
		if (timeline.size() > 0)
		{
			std::shared_ptr<DeterministicAssetVol> assetVol = std::make_shared<PiecewiseConstVol>(timeline, vols);
			m_vol.at(strike) = assetVol;
			LOG(INFO) << " K = " << strike << ", Timeline " << timeline << ", vol " << vols << endl;
		}
	}
}