/// Copyright (C) Nathan Muruganantha 2013 - 2015

#ifndef _DERIVATIVE_GARCH_H_
#define _DERIVATIVE_GARCH_H_

#include <memory>
#include <string>
#include <algorithm>

#include "ClassType.hpp"
#include "Global.hpp"
#include "DeterministicVol.hpp"
#include "Name.hpp"

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
	class IAsset;

	class PRICINGENGINE_DLL_API GARCH
		: virtual public IObject
	{
	public:

		enum { TYPEID = CLASS_GARCH_TYPE };

		struct GARCHParamType
		{
			double omega;

			double alpha;

			double beta;

			double r;

			double V;
		};

		enum AssetClassTypeEnum {EQUITY = 1, FUTURES = 2, FOREX = 3};

		GARCH()
		{}

		GARCH(AssetClassTypeEnum assetCls, const std::shared_ptr<IAssetValue>& asset, const dd::date& processDate);

		///destructor
		virtual ~GARCH()
		{}

		dd::date GetProcessedDate() const
		{
			return m_processedDate;
		}

		/// return the DeterministicAssetVol given strike price
		std::shared_ptr<DeterministicAssetVol> GetVolatility();

		/// build GARCH parameters
		void Build();

		/// load historical data from external source
		virtual void LoadData() = 0;

		virtual Array<double, 1> GetAssetValues() = 0;

	protected:

		AssetClassTypeEnum m_assetClass;
				
		/// underlying
		std::shared_ptr<IAssetValue> m_asset;

		/// processed date
		dd::date m_processedDate;

		GARCHParamType m_garch;

		bool m_initialized;

		std::shared_ptr<DeterministicAssetVol> m_vol;

		/// disallow the copy constructor and operator= functions
		DISALLOW_COPY_AND_ASSIGN(GARCH);

	private:

		Array<double, 1> GARCH::NelderMead(int N, double NumIters, double MaxIters,
			double Tolerance, Array<double, 2> x);

		double LogLikelihood(Array<double, 1> B);
	};
}

/* namespace derivative */
#endif /* _DERIVATIVE_GARCH_H_ */