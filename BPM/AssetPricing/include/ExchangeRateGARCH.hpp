/// Copyright (C) Nathan Muruganantha 2013 - 2015

#ifndef _DERIVATIVE_EXCHANGERATEGARCH_H_
#define _DERIVATIVE_EXCHANGERATEGARCH_H_

#include <memory>
#include <string>
#include <algorithm>

#include "GARCH.hpp"
 
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
	class IDailyExchangeRateValue;

	class PRICINGENGINE_DLL_API ExchangeRateGARCH	: public GARCH
	{
      public:
       	  
	    enum { TYPEID = CLASS_EXCHANGERATEGARCH_TYPE };

		static Name ConstructName(const std::string& domestic, const std::string& foreign, const dd::date& tradeDate)
		{
			Name nm(TYPEID, std::hash<std::string>()(domestic + foreign + dd::to_simple_string(tradeDate)));
			nm.AppendKey(string("domestic"), boost::any_cast<string>(domestic));
			nm.AppendKey(string("foreign"), boost::any_cast<string>(foreign));
			nm.AppendKey(string("tradeDate"), boost::any_cast<dd::date>(tradeDate));
			return nm;
		}

		inline static void GetKeys(const Name& nm, std::string& domestic, std::string& foreign, dd::date& tradeDate)
		{
			Name::KeyMapType keys = nm.GetKeyMap();
			auto i = keys.find("domestic");
			domestic = boost::any_cast<std::string>(i->second);
			auto j = keys.find("foreign");
			foreign = boost::any_cast<std::string>(j->second);
			auto k = keys.find("tradeDate");
			tradeDate = boost::any_cast<dd::date>(k->second);
		}
		
		/// Constructor with Exemplar 
		ExchangeRateGARCH(const Exemplar &ex);

		ExchangeRateGARCH(const std::string& domestic, const std::string& foreign, const std::shared_ptr<IAssetValue>& asset, const dd::date& processDate);

		///destructor
		virtual ~ExchangeRateGARCH()
		{
		}

		const Name& GetName()
		{
			return m_name;
		}

		/// load historical data from external source
		void LoadData();

		virtual Array<double, 1> GetAssetValues()
		{
			return m_values;
		}

	private:
		
		Name m_name;

		std::string m_domestic;

		std::string m_foreign;

		deque<shared_ptr<IDailyExchangeRateValue> > m_exchangeRateValues;

		Array<double, 1> m_values;

		/// disallow the copy constructor and operator= functions
		DISALLOW_COPY_AND_ASSIGN(ExchangeRateGARCH);
    };	

	/// utility function to build garch model given
	/// underlying ticker symbol and and effective date
	PRICINGENGINE_DLL_API std::shared_ptr<ExchangeRateGARCH> BuildExchangeRateGARCH(const std::string& domestic, const std::string& foreign, const dd::date& tradeDate);
}

/* namespace derivative */
#endif /* _DERIVATIVE_EXCHANGERATEGARCH_H_ */