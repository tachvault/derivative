/// Copyright (C) Nathan Muruganantha 2013 - 2014

#ifndef _DERIVATIVE_VOLATILITYSURFACE_H_
#define _DERIVATIVE_VOLATILITYSURFACE_H_

#include <memory>
#include <string>
#include <algorithm>

#include "ClassType.hpp"
#include "Global.hpp"
#include "DeterministicVol.hpp"
#include "Name.hpp"
#include "Country.hpp"

#if defined _WIN32 || defined __CYGWIN__
#ifdef PRICINGENGINE_EXPORTS
#ifdef __GNUC__
#define PRICINGENGINE_DLL_API __attribute__ ((dllexport))
#else
#define PRICINGENGINE_DLL_API __declspec(dllexport)
#endif
#else
#ifdef __GNUC__
#define PRICINGENGINE_DLL_API __attribute__ ((dllimport))
#else
#define PRICINGENGINE_DLL_API __declspec(dllimport)
#endif
#endif
#define PRICINGENGINE_LOCAL
#else
#if __GNUC__ >= 4
#define PRICINGENGINE_DLL_API __attribute__ ((visibility ("default")))
#define PRICINGENGINE_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define PRICINGENGINE_DLL_API
#define PRICINGENGINE_LOCAL
#endif
#endif

namespace derivative
{
	class IAssetValue;
	class IDailyOptionValue;
	class BlackScholesAssetAdapter;

	class PRICINGENGINE_DLL_API VolatilitySurface
		: virtual public IObject
	{
	public:

		enum { TYPEID = CLASS_VOLATILITYSURFACE_TYPE };

		enum AssetClassTypeEnum {EQUITY = 1, FUTURES = 2};

		VolatilitySurface()
		{}

		VolatilitySurface(AssetClassTypeEnum assetCls, const string& symbol, const std::shared_ptr<IAssetValue>& asset, const dd::date& processDate);

		///destructor
		virtual ~VolatilitySurface()
		{
			m_vol.clear();
		}

		/// returns underlying asset symbol
		std::string GetUnderlyingSymbol() const
		{
			return m_symbol;
		}

		dd::date GetProcessedDate() const
		{
			return m_processedDate;
		}

		/// return the DeterministicAssetVol given strike price
		std::shared_ptr<DeterministicAssetVol> GetVolatility(const dd::date& mat, double strike, int rateType, bool exactMatch = false);

		/// return constant vol by CramCharlier (bootstrapped from adjacent maturities
		std::shared_ptr<DeterministicAssetVol> GetConstVol(const dd::date& mat, double strike, double domestic_discount, double foreign_discount, int rateType) const;

		/// return GramCharlier vol.
		double GetVolByGramCharlier(const dd::date& mat, double strike, double domestic_discount, double foreign_discount, int rateType) const;

		/// load options data from external source
		virtual void LoadOptions() = 0;

		/// build vol surface for the given underlying
		void Build(double strike, int rateType);

	protected:

		AssetClassTypeEnum m_assetClass;
				
		/// underlying symbol
		std::string m_symbol;

		/// underlying
		std::shared_ptr<IAssetValue> m_asset;

		/// BlackScholes asset
		std::shared_ptr<BlackScholesAssetAdapter> m_bsasset;

		/// processed date
		dd::date m_processedDate;

		/// options for the underlying for the given processed date
		std::vector<std::shared_ptr<IDailyOptionValue> > m_options;

		/// country
		Country m_cntry;

		std::map<double, std::shared_ptr<DeterministicAssetVol> > m_vol;

		/// disallow the copy constructor and operator= functions
		DISALLOW_COPY_AND_ASSIGN(VolatilitySurface);
	};
}

/* namespace derivative */
#endif /* _DERIVATIVE_VOLATILITYSURFACE_H_ */