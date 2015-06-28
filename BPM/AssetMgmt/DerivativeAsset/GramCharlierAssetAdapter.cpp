/*
Copyright (C) Nathan Muruganantha 2013 - 2014
*/

#include <stdexcept>
#include <functional>
#include <future>

#include "GroupRegister.hpp"
#include "EntityMgrUtil.hpp"
#include "EntityManager.hpp"

#include "GramCharlierAssetAdapter.hpp"
#include "IDailyEquityOptionValue.hpp"
#include "IStockValue.hpp"
#include "ConstVol.hpp"
#include "BlackScholesAssetAdapter.hpp"
#include "IIRValue.hpp"
#include "Maturity.hpp"
#include "PrimaryAssetUtil.hpp"
#include "IPrimitiveSecurity.hpp"
#include "Exchange.hpp"

#undef min
#undef max

using namespace std::placeholders;

namespace derivative
{
	GramCharlierAssetAdapter::GramCharlierAssetAdapter(const Exemplar &ex)
		:m_name(TYPEID),
		m_options(std::vector<std::shared_ptr<IDailyOptionValue> >())
	{
		LOG(INFO) << "Exemplar Constructor is called" << endl;
	}

	/// Constructor.
	GramCharlierAssetAdapter::GramCharlierAssetAdapter(GramCharlier& xgc, const std::shared_ptr<IAssetValue>& asset, \
		int daysForMaturity, std::vector<std::shared_ptr<IDailyOptionValue> >& options)
		:m_name(TYPEID, 1),
		m_asset(asset),
		m_tenor((double)daysForMaturity / 365),
		m_options(std::move(options)),
		m_r(0.0)
	{
		/// Get domestic interest rate for the asset exchange
		auto cntry = dynamic_pointer_cast<IPrimitiveSecurity>(m_asset->GetAsset())->GetExchange().GetCountry().GetCode();
		dd::date today(dd::day_clock::local_day());
		std::shared_ptr<IRCurve> irCurve = BuildIRCurve(IRCurve::LIBOR, cntry, today);
		auto term = irCurve->GetTermStructure();
		m_r = PrimaryUtil::getDFToCompoundRate((*term)(m_tenor), m_tenor);

		/// construct GramCharlier asset
		std::shared_ptr<DeterministicAssetVol>  xv = std::make_shared<ConstVol>(m_asset->GetAsset()->GetImpliedVol());
		auto vol_level = xv->volproduct(0, m_tenor, *xv);
		m_gramCharlierAsset = std::unique_ptr<GramCharlierAsset>(new GramCharlierAsset(xgc, vol_level, \
			asset->GetTradePrice(), m_tenor));

		/// construct the BlackScholesAdapter from the asset value.
		m_bsAsset = std::make_shared<BlackScholesAssetAdapter>(m_asset, xv);
	};

	std::shared_ptr<IMake> GramCharlierAssetAdapter::Make(const Name &nm)
	{
		throw std::logic_error("Invalid factory method call");
	}

	std::shared_ptr<IMake> GramCharlierAssetAdapter::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("Invalid factory method call");
	}

	/// Best fit calibration to a given set of Black/Scholes implied volatilities.
	double GramCharlierAssetAdapter::calibrate(double domestic_discount, double foreign_discount, int highest_moment)
	{
		/// for each option, calculate the implied volatility
		/// add the implied volatility and strike price into two seperate vectors
		std::shared_ptr<Array<double, 1> > strikes = std::make_shared<Array<double, 1> >(m_options.size());
		std::shared_ptr<Array<double, 1> > vols = std::make_shared<Array<double, 1> >(m_options.size());
		int i = 0;
		std::vector<std::future<double>> futures;
		futures.reserve(m_options.size());
		auto sign = 1;
		for (std::shared_ptr<IDailyOptionValue> option : m_options)
		{
			try
			{
				/// calculate the implied volatility
				futures.push_back(std::move(std::async(&BlackScholesAssetAdapter::CalculateImpliedVolatility, m_bsAsset, option->GetTradePrice(), m_tenor, option->GetStrikePrice(), m_r, sign)));
			}
			catch (std::out_of_range& e)
			{
				LOG(INFO) << "Out of rage vol for " << option->GetStrikePrice() << endl;
				throw e;
			}
			catch (std::runtime_error& e)
			{
				LOG(INFO) << "Run time error occurred for vol with strike price  " << option->GetStrikePrice() << endl;
				throw e;
			}
			++i;
		}
		i = 0;
		try
		{
			for (auto &val : futures)
			{
				(*strikes)(i) = m_options[i]->GetStrikePrice();
				(*vols)(i) = val.get();
				++i;
			}
		}
		catch (std::exception& e)
		{
			LOG(ERROR) << "Error occurred while calculating implied vol " << e.what() << endl;
			throw e;
		}

		/// Now call the Caliberate function to caliberate the contained GramCharlier asset
		/// with those implied volatilities and strike prices.
		return m_gramCharlierAsset->calibrate(strikes, vols, domestic_discount, foreign_discount, highest_moment);
	}

	std::shared_ptr<GramCharlierAssetAdapter> GramCharlierAssetAdapter::Create(GramCharlier& xgc, \
		const std::shared_ptr<IAssetValue>& asset, int daysForMaturity, std::vector<std::shared_ptr<IDailyOptionValue> >& options)
	{
		/// Construct GramCharlierAssetAdapter from given name and register with EntityManager
		std::shared_ptr<GramCharlierAssetAdapter> value = make_shared<GramCharlierAssetAdapter>(xgc, asset, daysForMaturity, options);

		/// return constructed object if no exception is thrown
		return value;
	}
}



