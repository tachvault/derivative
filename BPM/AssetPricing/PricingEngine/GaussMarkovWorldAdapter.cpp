/*
Copyright (C) Nathan Muruganantha 2013 - 2014
*/

#include <algorithm>
#include <fstream>
#include "GaussMarkovWorldAdapter.hpp"
#include "PrimaryAssetUtil.hpp"
#include "Currency.hpp"
#include "Country.hpp"
#include "IExchangeRateValue.hpp"
#include "GaussMarkovWorld.hpp"
#include "GaussianEconomyAdapter.hpp"

namespace derivative
{
	// Constructor
	GaussMarkovWorldAdapter::GaussMarkovWorldAdapter(const Country& domestic, std::vector<std::shared_ptr<GaussianEconomyAdapter> > xeconomies)
		:m_domestic(domestic)
	{
		initialise(xeconomies);
	}

	void GaussMarkovWorldAdapter::initialise(std::vector<std::shared_ptr<GaussianEconomyAdapter> > xeconomies)
	{
		/// from the economies, get the currencies involved.
		const Currency& domesticCurr = m_domestic.GetCurrency();
		
		/// now for each foreign economy if the currency is not different
		/// from domestic currency.		
		std::vector<std::shared_ptr<GaussianEconomy> > economies;
		for(auto& economy: xeconomies)
		{
			const Currency& foreign = economy->GetCountry().GetCurrency();
			if (m_domestic.GetCurrency() != foreign)
			{
				m_exchangeRates.push_back(PrimaryUtil::getExchangeRateValue(m_domestic.GetCurrency().GetCode(), foreign.GetCode()));
				economies.push_back(economy->GetOrigin());
			}
		}

		/// now construct GaussMarkovWorld initial value and deterministic volatility
		std::vector<std::shared_ptr<DeterministicAssetVol> > xvols;
		Array<double,1> xinitial_exchange_rates(m_exchangeRates.size());
		int i = 0;
		for(auto& exchangeRate: m_exchangeRates)
		{
			/// NATHAN : FIXME
			//xvols.push_back(exchangeRate->GetVolatility());
			xinitial_exchange_rates(i) = exchangeRate->GetTradePrice();
		}					
				
		m_GaussMarkovWorld = std::unique_ptr<GaussMarkovWorld>(new GaussMarkovWorld(economies, xvols, xinitial_exchange_rates));
	}
}