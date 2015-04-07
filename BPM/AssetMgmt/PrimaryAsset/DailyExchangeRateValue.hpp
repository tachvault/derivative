/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_DAILYEXCHANGERATEVALUE_H_
#define _DERIVATIVE_DAILYEXCHANGERATEVALUE_H_

#define _VARIADIC_MAX 10 

#include <string>

#include "IMake.hpp"
#include "IDailyExchangeRateValue.hpp"
#include "IExchangeRate.hpp"
#include "Name.hpp"

namespace derivative
{
	/// Class DailyExchangeRateValue represents daily value of a exchangeRate.
	/// snapshot after the closing bell. It is constructed as a Named type. 
	/// That is, only one shared object per exchangeRate would be shared among all client(s)
	/// threads. Clients will have the ability to refresh from external source.
	/// clients will also have the ability to flush the values to Database
	class DailyExchangeRateValue : virtual public IDailyExchangeRateValue,
		virtual public IMake
	{

	public:

		enum {TYPEID = CLASS_DAILYEXCHANGERATEVALUE_TYPE};

		/// Constructor with Exemplar for the Creator ExchangeRate value object
		DailyExchangeRateValue (const Exemplar &ex);

		/// construct DailyExchangeRateValue from all its attributes
		/// used in IMake::Make(..)
		DailyExchangeRateValue (const Name& nm);

		DailyExchangeRateValue (const Name& nm, double openPrice, double closePrice, double priceHigh, \
			double priceLow, const dd::date& tradeDate);

		std::shared_ptr<IMake> Make (const Name &nm);

		std::shared_ptr<IMake> Make (const Name &nm, const std::deque<boost::any>& agrs);

		/// construct DailyExchangeRateValue from input stream.		
		virtual void convert( istringstream  &input);

		const Name& GetName()
		{
			std::lock_guard<SpinLock> lock(m_lock);
			return m_name;
		}

		void SetName(const Name& nm)
		{
			std::lock_guard<SpinLock> lock(m_lock);
			m_name = nm;
		}

		//// Return the date this exchangeRate was last traded.
		boost::gregorian::date   GetTradeDate() const
		{
			std::lock_guard<SpinLock> lock(m_lock);
			return m_tradeDate;
		}

		/// return day's open price
		virtual double GetPriceOpen() const
		{
			return m_priceOpen;
		}

		/// return day's  closing price
		virtual double GetPriceClose() const
		{
			return m_priceClose;
		}

		/// return day's High price 
		virtual double GetPriceHigh() const
		{
			return m_priceHigh;
		}

		/// return day's low price 
		virtual double GetPriceLow() const
		{
			return m_priceLow;
		}
		
		std::shared_ptr<IAsset> GetAsset() const
		{
			std::lock_guard<SpinLock> lock(m_lock);
			return m_exchangeRate;
		}

		/// return exchangeRate.
		std::shared_ptr<const IExchangeRate> GetExchangeRate() const
		{
			std::lock_guard<SpinLock> lock(m_lock);
			return m_exchangeRate;
		}

		void SetTradeDate(const boost::gregorian::date& d)
		{
			std::lock_guard<SpinLock> lock(m_lock);
			m_tradeDate = d;
		}

		/// set day's open price
		virtual void SetPriceOpen(double price)
		{
			m_priceOpen = price;
		}

		/// set day's closing price
		virtual void SetPriceClose(double price)
		{
			m_priceClose  = price;
		}

		/// set day's High price 
		virtual void SetPriceHigh(double price)
		{
			m_priceHigh = price;
		}

		/// set day's low price 
		virtual void SetPriceLow(double price)
		{
			m_priceLow = price;
		}

		/// no last reported value for historical data
		virtual double GetTradePrice() const
		{
			return 0;
		}

		virtual double GetDivYield() const
		{
			/// div yield is not applicable for ex
			throw std::logic_error("Invalid call");
		}

		virtual void SetExchangeRate(std::shared_ptr<IExchangeRate> exchangeRate)
		{
			std::lock_guard<SpinLock> lock(m_lock);
			m_exchangeRate = exchangeRate;
		}

	private:

		/// open price of the exchangeRate. 
		std::atomic<double> m_priceOpen;

		/// The closing price of the exchangeRate.
		std::atomic<double>   m_priceClose;

		/// High price on the day. 
		std::atomic<double>    m_priceHigh;

		/// Low price on the day. 
		std::atomic<double>    m_priceLow;

		/// The date this exchangeRate value.
		dd::date m_tradeDate;

		/// Name(TYPEID, std::hash<std::string>()(domestic.GetCode() + \
		/// foreign.GetCode() + to_string(tradeDate)))
		/// Key[0] => "domestic"
		/// Key[1] => "foreign"
		/// key[2] => "tradeDate"
		Name m_name;

		std::shared_ptr<IExchangeRate> m_exchangeRate;

		mutable SpinLock m_lock;
	};
}


/* namespace derivative */

#endif /* _DERIVATIVE_DAILYEXCHANGERATEVALUE_H_ */