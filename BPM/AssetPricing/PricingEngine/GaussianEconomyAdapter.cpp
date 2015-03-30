/*
Copyright (C) Nathan Muruganantha 2013 - 2014
*/

#include "GaussianEconomyAdapter.hpp"
#include "EntityMgrUtil.hpp"
#include "IAssetValue.hpp"
#include "IPrimitiveSecurity.hpp"
#include "Currency.hpp"
#include "Country.hpp"
#include "IRCurve.hpp"
#include "GaussianEconomy.hpp"

namespace derivative
{
	int GaussianEconomyAdapter::economy_id = 0;

	GaussianEconomyAdapter::GaussianEconomyAdapter(const Name& nm, const Country& cntry, \
		const std::vector<Name>& assets, IRCurve::DataSourceType src)
		:m_country(cntry),
		m_name(nm)
	{
		/// resolve all the assets and create corresponding BlackScholesAdapter objects.
		std::vector<std::shared_ptr<BlackScholesAssetAdapter> > underlying;
		for (auto& nm: assets)
		{
			/// find the object with Name nm from registry or from external source
			/// all exceptions would be propogated to calling function.
			std::shared_ptr<IAssetValue> assetValue = dynamic_pointer_cast<IAssetValue>(EntityMgrUtil::findObject(nm));

			/// get the asset and check if all the assets have the same domestic currency.
			std::shared_ptr<IPrimitiveSecurity> asset = dynamic_pointer_cast<IPrimitiveSecurity>(assetValue->GetAsset());

			if (!cntry.GetCurrency().GetCode().compare(asset->GetDomesticCurrency().GetCode()) == 0)
			{
				throw std::domain_error("Not all assets have the same currency code");
			}

			/// if no exceptions are thrown we are good to go and create BlackScholesAdapter object.
			std::shared_ptr<BlackScholesAssetAdapter> bs = std::make_shared<BlackScholesAssetAdapter>(assetValue);
			underlying.push_back(bs);
		}

		/// Construct the IRCurve and get the interest rate volatility and
		/// term structure of discount factors.
		Name irCurveName = IRCurve::ConstructName(m_country.GetCode(), dd::day_clock::local_day(), IRCurve::YIELD);

		/// find IR Curve from registry or construct and register a new IRCurve
		std::shared_ptr<IRCurve> irCurve = dynamic_pointer_cast<IRCurve>(EntityMgrUtil::findObject(irCurveName));

		/// building of IRCurve should build term structure of volatility and discount factors
		irCurve->BuildIRCurve();

		/// get the termstructure and volatility from the IRCurve.
		std::shared_ptr<DeterministicAssetVol> vol = irCurve->GetVolatility();
		std::shared_ptr<TermStructure> term = irCurve->GetTermStructure();

		/// Now construct GaussianEconomy object.
		m_economy = std::make_shared<GaussianEconomy>(underlying, vol, term);
	}	
}
