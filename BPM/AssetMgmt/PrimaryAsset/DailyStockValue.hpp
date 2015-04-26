/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_DAILYSTOCKVALUE_H_
#define _DERIVATIVE_DAILYSTOCKVALUE_H_

#define _VARIADIC_MAX 10 

#include <string>

#include "IMake.hpp"
#include "IDailyStockValue.hpp"
#include "Name.hpp"

namespace derivative
{
	class IStock;

	/// Class DailyStockValue represents daily value of a stock
	/// snapshot after the closing bell. It is constructed as a Named type. 
	/// That is, only one shared object per stock would be shared among all client(s)
	/// threads. Clients will have the ability to refresh from external source.
	/// clients will also have the ability to flush the values to Database
	class DailyStockValue : virtual public IDailyStockValue,
		virtual public IMake
	{

	public:

		enum {TYPEID = CLASS_DAILYSTOCKVALUE_TYPE};

		/// Constructor with Exemplar for the Creator Stock value object
		DailyStockValue (const Exemplar &ex);

		/// construct DailyStockValue from all its attributes
		/// used in IMake::Make(..)
		DailyStockValue (const Name& nm);

		DailyStockValue (const Name& nm, double openPrice, double closePrice, double priceHigh, \
			double priceLow, double PriceAdjClose, const dd::date& tradeDate);

		std::shared_ptr<IMake> Make (const Name &nm);

		std::shared_ptr<IMake> Make (const Name &nm, const std::deque<boost::any>& agrs);

		/// construct DailyStockValue from input stream.		
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

		//// Return the date this stock was last traded.
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

		/// return day's adjusted close price
		virtual double GetPriceAdjClose() const
		{
			return m_priceClose;
		}

		virtual double GetDivYield() const
		{
			return m_divYield;
		}

		/// return stock.
		std::shared_ptr<IAsset> GetAsset() const
		{
			std::lock_guard<SpinLock> lock(m_lock);
			return m_stock;
		}

		/// return stock.
		std::shared_ptr<const IStock> GetStock() const
		{
			std::lock_guard<SpinLock> lock(m_lock);
			return m_stock;
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

		/// now last reported value for historical data
		double GetTradePrice() const
		{
			return m_priceClose;
		}

		/// set day's adjusted close price
		virtual void SetPriceAdjClose(double price)
		{
			m_priceAdjustedClose = price;
		}

		virtual void SetDivYield(double yield)
		{
			m_divYield = yield;
		}

		void SetStock(std::shared_ptr<IStock> stock)
		{
			std::lock_guard<SpinLock> lock(m_lock);
			m_stock = stock;
		}

	private:

		/// open price of the stock. 
		std::atomic<double> m_priceOpen;

		/// The closing price of the stock.
		std::atomic<double>   m_priceClose;

		/// High price on the day. 
		std::atomic<double>    m_priceHigh;

		/// Low price on the day. 
		std::atomic<double>    m_priceLow;

		/// Adjusted closing price on the day. 
		std::atomic<double>    m_priceAdjustedClose;

		/// The date this stock value.
		dd::date m_tradeDate;

		std::atomic<double> m_divYield;

		/// Name(DailyStockValue::TYPEID, std::hash<std::string>() \
		///  (string(symbol + m_tradedate)))
		/// Key[0] => "symbol"
		/// key[1] => "trade date"
		Name m_name;

		std::shared_ptr<IStock> m_stock;

		mutable SpinLock m_lock;
	};
}


/* namespace derivative */

#endif /* _DERIVATIVE_DAILYSTOCKVALUE_H_ */