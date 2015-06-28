/// Copyright (C) Nathan Muruganantha 2013 - 2014

#ifndef _DERIVATIVE_FUTURESVOLATILITYSURFACE_H_
#define _DERIVATIVE_FUTURESVOLATILITYSURFACE_H_

#include "VolatilitySurface.hpp"

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
	class IDailyFuturesOptionValue;

	class PRICINGENGINE_DLL_API FuturesVolatilitySurface : public VolatilitySurface
	{
	public:

		enum { TYPEID = CLASS_FUTURESVOLATILITYSURFACE_TYPE };

		static Name ConstructName(const string& symbol, const dd::date& processDate, const dd::date& deliveryDate)
		{
			Name nm(TYPEID, std::hash<std::string>()(symbol + dd::to_simple_string(processDate) + dd::to_simple_string(deliveryDate)));
			nm.AppendKey(string("symbol"), boost::any_cast<string>(symbol));
			nm.AppendKey(string("processDate"), boost::any_cast<dd::date>(processDate));
			nm.AppendKey(string("deliveryDate"), boost::any_cast<dd::date>(deliveryDate));
			return nm;
		}

		inline static void GetKeys(const Name& nm, string& symbol, dd::date& processDate, dd::date& deliveryDate)
		{
			Name::KeyMapType keys = nm.GetKeyMap();
			auto i = keys.find("symbol");
			symbol = boost::any_cast<string>(i->second);
			auto j = keys.find("processDate");
			processDate = boost::any_cast<dd::date>(j->second);
			auto k = keys.find("deliveryDate");
			processDate = boost::any_cast<dd::date>(k->second);
		}

		/// Constructor with Exemplar 
		FuturesVolatilitySurface(const Exemplar &ex);

		FuturesVolatilitySurface(const string& symbol, const std::shared_ptr<IAssetValue>& asset, const dd::date& processDate, const dd::date& deliveryDate);

		///destructor
		~FuturesVolatilitySurface()
		{
			m_vol.clear();
		}

		const Name& GetName()
		{
			return m_name;
		}

		dd::date GetdeliveryDate() const
		{
			return m_deliveryDate;
		}

		/// return constant vol by CramCharlier (bootstrapped from adjacent maturities
		std::shared_ptr<DeterministicAssetVol> GetConstVol(const dd::date& mat, double strike) const;

		/// load options data from external source
		void LoadOptions();

	private:

		Name m_name;

		/// delivery date
		dd::date m_deliveryDate;

		/// disallow the copy constructor and operator= functions
		DISALLOW_COPY_AND_ASSIGN(FuturesVolatilitySurface);
	};

	/// utility function to build Volatility Surface given
	/// underlying ticker symbol and and effective date
	PRICINGENGINE_DLL_API std::shared_ptr<FuturesVolatilitySurface> BuildFuturesVolSurface(const std::string& symbol, const dd::date& edate, const dd::date& ddate);
}

/* namespace derivative */
#endif /* _DERIVATIVE_FUTURESVOLATILITYSURFACE_H_ */