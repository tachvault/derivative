/// Copyright (C) Nathan Muruganantha 2013 - 2014

#ifndef _DERIVATIVE_EQUITYVOLATILITYSURFACE_H_
#define _DERIVATIVE_EQUITYVOLATILITYSURFACE_H_

#include <memory>
#include <string>
#include <algorithm>

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
	class IStockValue;
	class IDailyEquityOptionValue;
	class BlackScholesAssetAdapter;

	class PRICINGENGINE_DLL_API EquityVolatilitySurface	: public VolatilitySurface
	{
      public:
       	  
		  enum { TYPEID = CLASS_EQUITYVOLATILITYSURFACE_TYPE };

		static Name ConstructName(const string& symbol, const dd::date& processDate)
		{
			Name nm(TYPEID, std::hash<std::string>()(symbol + dd::to_simple_string(processDate)));
			nm.AppendKey(string("symbol"), boost::any_cast<string>(symbol));
			nm.AppendKey(string("processDate"), boost::any_cast<dd::date>(processDate));
			return nm;
		}

		inline static void GetKeys(const Name& nm, string& symbol, dd::date& processDate)
		{
			Name::KeyMapType keys = nm.GetKeyMap();
			auto i = keys.find("symbol");
			symbol = boost::any_cast<string>(i->second);
			auto j = keys.find("processDate");
			processDate = boost::any_cast<dd::date>(j->second);
		}

		/// Constructor with Exemplar 
		EquityVolatilitySurface(const Exemplar &ex);

		EquityVolatilitySurface(const string& symbol, const std::shared_ptr<IAssetValue>& asset, const dd::date& processDate);

		///destructor
		~EquityVolatilitySurface()
		{
			m_vol.clear();
		}

		const Name& GetName()
		{
			return m_name;
		}

		/// return constant vol by CramCharlier (bootstrapped from adjacent maturities
		std::shared_ptr<DeterministicAssetVol> GetConstVol(const dd::date& mat, double strike, int rateType) const;

		/// load options data from external source
		void LoadOptions();

	private:
		
		Name m_name;

		/// disallow the copy constructor and operator= functions
		DISALLOW_COPY_AND_ASSIGN(EquityVolatilitySurface);
    };	

	/// utility function to build Volatility Surface given
	/// underlying ticker symbol and and effective date
	PRICINGENGINE_DLL_API std::shared_ptr<EquityVolatilitySurface> BuildEquityVolSurface(const std::string& symbol, const dd::date& edate);
}

/* namespace derivative */
#endif /* _DERIVATIVE_EQUITYVOLATILITYSURFACE_H_ */