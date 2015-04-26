/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_HISTORICALSTOCKVALUE_H_
#define _DERIVATIVE_HISTORICALSTOCKVALUE_H_

#if defined _WIN32
/// disable warnings on 255 char debug symbols
#pragma warning (disable : 4786)
/// disable warnings on extern before template instantiation
#pragma warning (disable : 4231)
#endif

#include <string>

#include "Name.hpp"
#include "IDailyStockValue.hpp"
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

class IDailyStockValue;

EXPIMP_TEMPLATE template class PRIMARYASSET_EXT_API std::deque<shared_ptr<IDailyStockValue> >;

namespace derivative
{
	/// Class HistoricalStockInfo represents daily value of a stock
	/// snapshot after the closing bell from start date to end date. 
	/// It is constructed as a Named type. That is, only one shared object
	/// per stock would be shared among all client(s) threads.
	class PRIMARYASSET_EXT_API HistoricalStockInfo : public IObject
	{

	public:

		enum {TYPEID = CLASS_HISTORICALSTOCKINFO_TYPE};

		inline static Name ConstructName(const std::string& symbol, const dd::date& start, const dd::date& end)
		{
			Name nm(TYPEID, std::hash<std::string>()(symbol + dd::to_simple_string(start) + dd::to_simple_string(end)));
			nm.AppendKey(string("symbol"), boost::any_cast<string>(symbol));
			nm.AppendKey(string("startDate"), boost::any_cast<dd::date>(start));
			nm.AppendKey(string("endDate"), boost::any_cast<dd::date>(end));
			return nm;
		}

		inline static void GetKeys(const Name& nm, std::string& symbol, dd::date& start, dd::date& end)
		{
			Name::KeyMapType keys = nm.GetKeyMap();
			auto i = keys.find("symbol");
			symbol = boost::any_cast<std::string>(i->second);
			auto j = keys.find("startDate");
			start = boost::any_cast<dd::date>(j->second);
			auto k = keys.find("endDate");
			end = boost::any_cast<dd::date>(k->second);
		}

		/// Constructor with Exemplar for the Creator historical Stock value object
		HistoricalStockInfo (const Exemplar &ex);

		/// construct HistoricalStockInfo from a given Name
		HistoricalStockInfo (const Name& nm);

		/// construct HistoricalStockInfo from 
		/// Stock symbol, Exchange, start date and end date
		/// Note: This constructor will not register itself
		/// itself with EntityManager. It is the responsibility of the
		/// caller to register and build the DailyStockInfo from data source
		HistoricalStockInfo (const std::string& symbol, \
			const dd::date& startDate, const dd::date& endDate);

		/// construct HistoricalStockInfo from 
		/// Stock symbol, Exchange, start date and end date
		HistoricalStockInfo::HistoricalStockInfo (const Name& nm, const std::string& symbol, \
			const dd::date& startDate, const dd::date& endDate)
			:m_name(nm), m_symbol(symbol), m_startDate(startDate), m_endDate(endDate)
		{		
		}

		const Name& GetName()
		{
			return m_name;
		}

		const std::string& GetSymbol() const
		{
			return m_symbol;
		}

		const dd::date& GetStartDate() const
		{
			return m_startDate;
		}

		const dd::date& GetEnddate() const
		{
			return m_endDate;
		}

		const deque<shared_ptr<IDailyStockValue> >& GetDailyStockValues() const
		{
			return m_stockValues;
		}

		void SetDailyStockValues(deque<shared_ptr<IDailyStockValue> >& stocks)
		{
			m_stockValues = std::move(stocks);
		}

		/// Populate the StockInfo deque
		void BuildStockInfo(unsigned int source);

	private:

		/// The symbol of the stock.
		string m_symbol;

		/// The start time of the historical stock info snapshots.
		dd::date m_startDate;

		/// The end time of the historical stock info snapshots.
		dd::date m_endDate;

		/// The historical snapshots of stock information.
		deque<shared_ptr<IDailyStockValue> > m_stockValues;

		/// Name(DailyStockValue::TYPEID, std::hash<std::string>() \
		///  (symbol, startDate, endDate))
		/// Key[0] => "symbol"
		/// key[1] => "start date"
		/// key[2] => "end date"
		Name m_name;
	};

	/// utility function to build historical stock information given
	/// ticker symbol, start date and end date
	PRIMARYASSET_EXT_API std::shared_ptr<HistoricalStockInfo> BuildHistoricalStockInfo(ushort source, const std::string& symbol, const dd::date& start, const dd::date& end);

	/// utility function to build historical stock information given
	/// ticker symbol, start date and end date
	PRIMARYASSET_EXT_API void StoreHistoricalStockInfo(ushort source, const std::shared_ptr<HistoricalStockInfo> & histStock);
}


/* namespace derivative */

#endif /* _DERIVATIVE_HISTORICALSTOCKVALUE_H_ */