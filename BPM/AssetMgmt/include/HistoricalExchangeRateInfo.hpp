/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_HISTORICALEXCHANGERATEVALUE_H_
#define _DERIVATIVE_HISTORICALEXCHANGERATEVALUE_H_

#if defined _WIN32
/// disable warnings on 255 char debug symbols
#pragma warning (disable : 4786)
/// disable warnings on extern before template instantiation
#pragma warning (disable : 4231)
#endif

#include <string>

#include "Name.hpp"
#include "IDailyExchangeRateValue.hpp"
#include "Exchange.hpp"


#if defined _WIN32 || defined __CYGWIN__
#ifdef PRIMARYASSET_EXT_EXPORTS
#ifdef __GNUC__
#define PRIMARYASSET_EXT_API __attribute__ ((dllexport))
#else
#define PRIMARYASSET_EXT_API __declspec(dllexport)
#define EXPIMP_TEMPLATE
#endif
#else
#ifdef __GNUC__
#define PRIMARYASSET_EXT_API __attribute__ ((dllimport))
#else
#define PRIMARYASSET_EXT_API __declspec(dllimport)
#define EXPIMP_TEMPLATE extern
#endif
#endif
#define PRIMARYASSET_EXT_LOCAL
#else
#if __GNUC__ >= 4
#define PRIMARYASSET_EXT_API __attribute__ ((visibility ("default")))
#define PRIMARYASSET_EXT_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define PRIMARYASSET_EXT_API
#define PRIMARYASSET_EXT_LOCAL
#endif
#endif

class IDailyExchangeRateValue;

EXPIMP_TEMPLATE template class PRIMARYASSET_EXT_API std::deque<shared_ptr<IDailyExchangeRateValue> >;

namespace derivative
{
	/// Class HistoricalExchangeRateInfo represents daily value of a exchangeRate
	/// snapshot after the closing bell from start date to end date. 
	/// It is constructed as a Named type. That is, only one shared object
	/// per exchangeRate would be shared among all client(s) threads.
	class PRIMARYASSET_EXT_API HistoricalExchangeRateInfo : public IObject
	{

	public:

		enum {TYPEID = CLASS_HISTORICALEXCHANGERATEINFO_TYPE};

		static Name ConstructName(const std::string& domestic,const std::string& foreign, \
			const dd::date& start, const dd::date& end)
		{
			Name nm(TYPEID, std::hash<std::string>()(domestic + foreign + \
				dd::to_simple_string(start) + dd::to_simple_string(end)));
			nm.AppendKey(string("domestic"), boost::any_cast<string>(domestic));
			nm.AppendKey(string("foreign"), boost::any_cast<string>(foreign));
			nm.AppendKey(string("startDate"), boost::any_cast<dd::date>(start));
			nm.AppendKey(string("endDate"), boost::any_cast<dd::date>(end));
			return nm;
		}

		inline static void GetKeys(const Name& nm, std::string& domestic, std::string& foreign, \
			dd::date& start, dd::date& end)
		{
			Name::KeyMapType keys = nm.GetKeyMap();
			auto i = keys.find("domestic");
			domestic = boost::any_cast<std::string>(i->second);
			auto j = keys.find("foreign");
			foreign = boost::any_cast<std::string>(j->second);
			auto k = keys.find("start");
			start = boost::any_cast<dd::date>(k->second);
			auto l = keys.find("end");
			end = boost::any_cast<dd::date>(k->second);
		}

		/// Constructor with Exemplar for the Creator historical ExchangeRate value object
		HistoricalExchangeRateInfo (const Exemplar &ex);

		/// construct HistoricalExchangeRateInfo from a given Name
		HistoricalExchangeRateInfo (const Name& nm);

		/// construct HistoricalExchangeRateInfo from 
		/// ExchangeRate symbol, Exchange, start date and end date
		/// Note: This constructor will not register itself
		/// itself with EntityManager. It is the responsibility of the
		/// caller to register and build the DailyExchangeRateInfo from data source
		HistoricalExchangeRateInfo(const std::string& domesticCurrCode, const std::string& foreignCurrCode,	\
			const dd::date& startDate, const dd::date& endDate);

		/// construct HistoricalExchangeRateInfo from 
		/// ExchangeRate symbol, Exchange, start date and end date
		HistoricalExchangeRateInfo::HistoricalExchangeRateInfo (const Name& nm, const Currency& domesticCurrency, \
			const Currency& foreignCurrency, const dd::date& startDate, const dd::date& endDate)
			:m_name(nm), m_domesticCurrency(domesticCurrency), m_foreignCurrency(domesticCurrency), \
			m_startDate(startDate), m_endDate(endDate)
		{		
		}

		const Name& GetName()
		{
			return m_name;
		}

		const Currency& GetDomesticCurrency() const
		{
			return m_domesticCurrency;
		}

		const Currency& GetForeignCurrency() const
		{
			return m_domesticCurrency;
		}

		const dd::date& GetStartDate() const
		{
			return m_startDate;
		}

		const dd::date& GetEnddate() const
		{
			return m_endDate;
		}

		const std::deque<shared_ptr<IDailyExchangeRateValue> >& GetDailyExchangeRateValues() const
		{
			return m_exchangeRateValues;
		}

		void SetDailyExchangeRateValues(std::deque<shared_ptr<IDailyExchangeRateValue> >& exchangeRates)
		{
			m_exchangeRateValues = std::move(exchangeRates);
		}

		/// Populate the ExchangeRateInfo deque
		void BuildExchangeRateInfo(unsigned int source);

	private:

		
		/// The domestic and foreign currencies of the exchangeRate.
		Currency m_domesticCurrency;
		Currency m_foreignCurrency;

		/// The start and end time of the historical exchangeRate info snapshots.
		dd::date m_startDate;
		dd::date m_endDate;

		/// The historical snapshots of exchangeRate information.
		std::deque<shared_ptr<IDailyExchangeRateValue> > m_exchangeRateValues;

		/// Name(HistoricalExchangeRateInfo::TYPEID, std::hash<std::string>() \
		///  (domesticCurrency, foreignCurrency, startDate, endDate))
		/// Key[0] => "domestic"
		/// Key[1] => "foreign"
		/// key[3] => "startDate"
		/// key[4] => "endDate"
		Name m_name;
	};

	/// utility function to build historical exchangeRate information given
	/// domestic and foreign currency codes, start date and end dates
	PRIMARYASSET_EXT_API void BuildHistoricalExchangeRateInfo(ushort source, const std::string& domCode, std::string& foreignCode, \
		                      const dd::date& start, const dd::date& end);

	/// utility function to build historical exchangeRate information given
	/// ticker symbol, start date and end date
	PRIMARYASSET_EXT_API void StoreHistoricalExchangeRateInfo(ushort source, const std::shared_ptr<HistoricalExchangeRateInfo> & histExchangeRate);
}

/* namespace derivative */

#endif /* _DERIVATIVE_HISTORICALEXCHANGERATEVALUE_H_ */