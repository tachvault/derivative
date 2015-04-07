/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_EXCHANGERATEVALUE_H_
#define _DERIVATIVE_EXCHANGERATEVALUE_H_

#include <string>
#include <memory>

#include "IMake.hpp"
#include "IExchangeRate.hpp"
#include "IExchangeRateValue.hpp"
#include "ConstVol.hpp"

namespace derivative
{
	/// Class ExchangeRateValue represents current value of an
	/// exhange rate given the type, domestic and foreign currencies
	class ExchangeRateValue : virtual public IExchangeRateValue,
		virtual public IMake
	{

	public:

		enum {TYPEID = CLASS_EXCHANGERATEVALUE_TYPE};

		static Name ConstructName(const std::string& domestic,const std::string& foreign)
		{
			Name nm(TYPEID, std::hash<std::string>()(domestic + foreign));
			nm.AppendKey(string("domestic"), boost::any_cast<string>(domestic));
			nm.AppendKey(string("foreign"), boost::any_cast<string>(foreign));
			return nm;
		}

		inline static void GetKeys(const Name& nm, std::string& domestic,const std::string& foreign)
		{
			Name::KeyMapType keys = nm.GetKeyMap();
			auto i = keys.find("domestic");
			domestic = boost::any_cast<std::string>(i->second);
			auto j = keys.find("foreign");
			domestic = boost::any_cast<std::string>(j->second);
		}

		/// Constructor with Exemplar for the Creator ExchangeRate value object
		ExchangeRateValue (const Exemplar &ex);

		/// construct ExchangeRateValue from a given Name
		/// used in IMake::Make(..)
		/// The object ID would be the same as that of underlying stock.
		ExchangeRateValue (const Name& nm);

		shared_ptr<IMake> Make (const Name &nm);

		shared_ptr<IMake> Make (const Name &nm, const std::deque<boost::any>& agrs);

		/// construct ExchangeRateValue from input stream.		
		virtual void convert( istringstream  &input);

		const Name& GetName()
		{
			std::lock_guard<SpinLock> lock(m_lock);
			return m_name;
		}

		/// return last asking price
		double GetAskingPrice() const
		{
			return m_priceAsk;
		}

		/// return bid price 
		double GetBidPrice() const
		{
			return m_priceBid;
		}

		/// return last openned price
		double GetPriceOpen() const
		{
			return m_priceOpen;
		}

		/// return last closed price
		double GetPriceClose() const
		{
			return m_priceClose;
		}

		/// yahoo symbol 'g'
		virtual double GetdayLow() const
		{
			return m_dayLow;
		}

		/// yahoo symbol 'h'
		virtual double  GetDayHigh() const 
		{
			return m_dayHigh;
		}

		/// yahoo symbol 'j'
		virtual double Get52WkLow() const
		{
			return m_52WkLow;
		}

		/// yahoo symbol 'k'
		virtual double Get52WkHigh() const
		{
			return m_52WkHigh;
		}

		/// return current market price
		double GetTradePrice() const
		{
			return m_priceTrade;
		}

		/// Return the date this stock was last traded.
		dd::date   GetTradeDate() const
		{
			std::lock_guard<SpinLock> lock(m_lock);
			return m_tradeDate;
		}

		//// Return the time this stock was last traded.
		pt::ptime  GetTradeTime() const
		{
			std::lock_guard<SpinLock> lock(m_lock);
			return m_tradeTime;
		}

		virtual double GetDivYield() const
		{
			throw std::logic_error("Invalid call");
		}

		std::shared_ptr<IAsset> GetAsset() const
		{
			std::lock_guard<SpinLock> lock(m_lock);
			return m_exchangeRate;
		}

		/// return stock.
		std::shared_ptr<const IExchangeRate> GetExchangeRate() const
		{
			std::lock_guard<SpinLock> lock(m_lock);
			return m_exchangeRate;
		}

		void SetAskingPrice(double ask)
		{
			m_priceAsk = ask;
		}

		void SetBidPrice(double bid)
		{
			m_priceBid = bid;
		}

		void SetPriceOpen(double price)
		{
			m_priceOpen = price;
		}

		void SetClosingPrice(double price)
		{
			m_priceClose = price;
		}

		void SetDayLow(double dayLow)
		{
			m_dayLow = dayLow;
		}

		void SetDayHigh(double dayHigh)
		{
			m_dayHigh = dayHigh;
		}

		void Set52WkLow(double _52WkLow)
		{
			m_52WkLow = _52WkLow;
		}

		void Set52WkHigh(double _52WkHigh)
		{
			m_52WkHigh = _52WkHigh;
		}

		void SetTradePrice(double price)
		{
			m_priceTrade = price;
		}

		void SetTradeDate(const dd::date& d)
		{
			std::lock_guard<SpinLock> lock(m_lock);
			m_tradeDate = d;
		}

		void SetTradeTime(const pt::ptime& time)
		{
			std::lock_guard<SpinLock> lock(m_lock);
			m_tradeTime = time;
		}

		void SetExchangeRate(std::shared_ptr<IExchangeRate> exRate)
		{
			std::lock_guard<SpinLock> lock(m_lock);
			m_exchangeRate = exRate;
		}		

	private:

		/// ask price of the stock. 
		/// yahoo symbol 'a'
		std::atomic<double> m_priceAsk;

		/// bid price of the stock. 
		/// yahoo symbol 'b'
		std::atomic<double> m_priceBid;

		/// The opening price of the stock.
		/// yahoo symbol 'o'
		std::atomic<double>   m_priceOpen;

		/// previous close price of the stock. 
		/// yahoo symbol 'p'
		std::atomic<double>    m_priceClose;

		/// The last Traded price of the stock.
		/// yahoo symbol 'l1'
		std::atomic<double> m_priceTrade;

		/// yahoo symbol 'g'
		std::atomic<double> m_dayLow;

		/// yahoo symbol 'h'
		std::atomic<double> m_dayHigh;

		/// yahoo symbol 'j'
		std::atomic<double> m_52WkLow;

		/// yahoo symbol 'k'
		std::atomic<double> m_52WkHigh;

		/// The date this stock was last traded.
		/// yahoo symbol  'd1'
		dd::date m_tradeDate;

		/// The time this stock was last traded.
		/// yahoo symbol  't1'
		pt::ptime m_tradeTime;
		
		/// Name(TYPEID, std::hash<std::string>()(domestic.GetCode() + \
		/// foreign.GetCode()))
		/// Key[0] => "domestic"
		/// Key[1] => "foreign"
		Name m_name;

		std::shared_ptr<IExchangeRate> m_exchangeRate;

		mutable SpinLock m_lock;
	};
}


/* namespace derivative */

#endif /* _DERIVATIVE_EXCHANGERATEVALUE_H_ */