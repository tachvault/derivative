/*
Copyright (C) Nathan Muruganantha 2013 - 2014
*/

#include <stdexcept>
#include <functional>

#include "GroupRegister.hpp"
#include "EntityMgrUtil.hpp"

#include "GramCharlierAssetAdapter.hpp"
#include "IDailyEquityOptionValue.hpp"
#include "IStockValue.hpp"
#include "ConstVol.hpp"
#include "BlackScholesAssetAdapter.hpp"
#include "IIRValue.hpp"
#include "Maturity.hpp"
#include "PrimaryAssetUtil.hpp"

#undef min
#undef max

using namespace std::placeholders;

namespace derivative
{	
	GramCharlierAssetAdapter::GramCharlierAssetAdapter(const Exemplar &ex)
		:m_name(TYPEID)
	{
		LOG(INFO) << "Exemplar Constructor is called" << endl;
	}

	/// Constructor.
	GramCharlierAssetAdapter::GramCharlierAssetAdapter(GramCharlier& xgc,const std::shared_ptr<IAssetValue>& asset, int maturity)
		:m_name(TYPEID, std::hash<std::string>()(asset->GetAsset()->GetSymbol() + to_string(maturity))),
		m_asset(asset),
		m_maturity(maturity)
	{
		std::shared_ptr<DeterministicAssetVol>  xv = std::make_shared<ConstVol>(m_asset->GetAsset()->GetImpliedVol());
		auto vol_level = xv->volproduct(0,maturity,*xv);
		m_gramCharlierAsset = std::unique_ptr<GramCharlierAsset>(new GramCharlierAsset(xgc, vol_level, \
			asset->GetTradePrice(), maturity/365));
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
	double GramCharlierAssetAdapter::calibrate(double domestic_discount,double foreign_discount,int highest_moment)
	{
		std::shared_ptr<IStockValue> stockVal = dynamic_pointer_cast<IStockValue>(m_asset);

		/// retrieve all the options.
		std::vector<std::shared_ptr<IDailyEquityOptionValue> > options;
		dd::date today = dd::day_clock::local_day();
		PrimaryUtil::FindOptionValues<IDailyEquityOptionValue>(stockVal->GetStock()->GetSymbol(), today, m_maturity, options);

		/// Get domestic interest rate of the stock
		auto cntry = stockVal->GetStock()->GetCountry().GetCode();
		auto maturity = Maturity::getMaturity(m_maturity);
		double r = PrimaryUtil::FindInterestRate(cntry, maturity/365, IRCurve::LIBOR);

		/// construct black scholes asset		
		std::shared_ptr<DeterministicAssetVol>  v = std::make_shared<ConstVol>(m_asset->GetAsset()->GetImpliedVol());
		/// now construct the BlackScholesAdapter from the stock value.
		std::shared_ptr<BlackScholesAssetAdapter> asset = \
			std::make_shared<BlackScholesAssetAdapter>(stockVal,v);

		/// for each option, calculate the implied volatility
		/// add the implied volatility and strike price into two seperate
		/// vectors
		std::shared_ptr<Array<double,1> > strikes = std::make_shared<Array<double,1> >(options.size());
		std::shared_ptr<Array<double,1> > vols = std::make_shared<Array<double,1> >(options.size());
		int i = 0;
		for(std::shared_ptr<IDailyEquityOptionValue> option: options)
		{
			try
			{
				/// calculate the implied volatility
			   auto vol = asset->CalculateImpliedVolatility(option->GetTradePrice(), (double)m_maturity/(double)365,option->GetStrikePrice(), r);
			   (*strikes)(i) = option->GetStrikePrice();
			   (*vols)(i) = vol;
			}
			catch(std::out_of_range& e)
			{
				LOG(INFO) << "Out of rage vol for " << option->GetStrikePrice() << endl;
			}
			catch(std::runtime_error& e)
			{
				LOG(INFO) << "Run time error occurred for vol with strike price  " << option->GetStrikePrice() << endl;
			}
			++i;
		}

		LOG(INFO) << " Start Caliberation for " << m_asset->GetAsset()->GetSymbol() << endl;
		LOG(INFO) << " Strike prices " << (*strikes) << endl;
		LOG(INFO) << " Volatilities " << (*vols) << endl;

		cout << " Start Caliberation for " << m_asset->GetAsset()->GetSymbol() << endl;
		cout << " Strike prices " << (*strikes) << endl;
		cout << " Volatilities " << (*vols) << endl;

		/// Now call the Caliberate function to caliberate the contained GramCharlier asset
		/// with those implied volatilities and strike prices.
		return m_gramCharlierAsset->calibrate(strikes, vols, domestic_discount,foreign_discount, highest_moment);
	}

	std::shared_ptr<GramCharlierAssetAdapter> GramCharlierAssetAdapter::Create(GramCharlier& xgc,const std::shared_ptr<IAssetValue>& asset, int maturity)
	{
		/// register the exemplar here until the bug in visual C++ is fixed.
		GroupRegister GramCharlierAssetAdapterValGrp(GramCharlierAssetAdapter::TYPEID,  std::make_shared<GramCharlierAssetAdapter>(Exemplar())); 				
	
		/// Construct GramCharlierAssetAdapter from given name and register with EntityManager
		std::shared_ptr<GramCharlierAssetAdapter> value = make_shared<GramCharlierAssetAdapter>(xgc, asset, maturity);
		EntityMgrUtil::registerObject(value->GetName(), value);		
		LOG(INFO) << " GramCharlierAssetAdapter  " << value->GetName() << " is constructed and registered with EntityManager" << endl;

		/// return constructed object if no exception is thrown
		return value;
	}
}



