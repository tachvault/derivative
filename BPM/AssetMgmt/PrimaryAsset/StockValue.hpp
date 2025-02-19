/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_STOCKVALUE_H_
#define _DERIVATIVE_STOCKVALUE_H_

#include <string>
#include <memory>

#include "IMake.hpp"
#include "IStockValue.hpp"
#include "Name.hpp"
#include "IStock.hpp"
#include "ConstVol.hpp"

namespace derivative
{
	/// Class StockValue represents current value of a stock
	/// It is constructed as a Named type. That is, only only one
	/// shared object per stock would be shared among all client(s)
	/// threads. Callers will have the ability to refresh from external source.
	class StockValue : virtual public IStockValue,
		virtual public IMake
	{

	public:

		enum {TYPEID = CLASS_STOCKVALUE_TYPE};

		/// Constructor with Exemplar for the Creator Stock value object
		StockValue (const Exemplar &ex);

		/// construct StockValue from a given Name
		/// used in IMake::Make(..)
		/// The object ID would be the same as that of underlying stock.
		StockValue (const Name& nm);

		virtual ~StockValue()
		{}

		shared_ptr<IMake> Make (const Name &nm);

		shared_ptr<IMake> Make (const Name &nm, const deque<boost::any>& agrs);

		/// construct StockValue from input stream.		
		virtual void convert( istringstream  &input);

		virtual const Name& GetName()
		{
			return m_name;
		}

		/// The time, data accessed from external system
		virtual pt::ptime GetAccessTime() const
		{
			std::lock_guard<SpinLock> lock(m_lock);
			return m_accessTime;
		}

		/// return last asking price
		virtual double GetAskingPrice() const
		{
			return m_priceAsk;
		}

		/// return bid price 
		virtual double GetBidPrice() const
		{
			return m_priceBid;
		}

		/// return last openned price
		virtual double GetPriceOpen() const
		{
			return m_priceOpen;
		}

		/// return last closed price
		virtual double GetPriceClose() const
		{
			return m_priceClose;
		}

		/// return current market price
		virtual double GetTradePrice() const
		{
			return m_priceTrade;
		}

		/// return price change
		virtual double GetPriceChange() const
		{
			return m_change;
		}

		/// return percentage of price change
		virtual double  GetChangePct() const
		{
			return m_changePct;
		}

		/// return the div yield
		virtual double GetDivYield() const
		{
			return m_divYield;
		}

		/// return div share
		virtual double GetDivShare() const
		{
			return m_divShare;
		}

		/// return earning per share
		virtual double  GetEPS() const
		{
			return m_eps;
		}

		//// Return the price-to-earnings ratio for this stock.
		virtual double  GetPE() const
		{
			return m_pe;
		}

		//// Return the double of shares outstanding of this stock.
		virtual int GetShares() const
		{
			return m_shares;
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

		//// Return the date this stock was last traded.
		virtual dd::date   GetTradeDate() const
		{
			std::lock_guard<SpinLock> lock(m_lock);
			return m_tradeDate;
		}

		//// Return the time this stock was last traded.
		virtual pt::ptime  GetTradeTime() const
		{
			std::lock_guard<SpinLock> lock(m_lock);
			return m_tradeTime;
		}

		//// Return the volume of the stock.
		virtual int GetVolume() const
		{
			return m_volume;
		}	

		virtual double GetBeta() const
		{
			return m_beta;
		}

		virtual std::shared_ptr<IAsset> GetAsset() const
		{
			std::lock_guard<SpinLock> lock(m_lock);
			return m_stock;
		}

		/// return stock.
		virtual shared_ptr<IStock> GetStock() const
		{
			std::lock_guard<SpinLock> lock(m_lock);
			return m_stock;
		}

		virtual void SetAccessTime(const pt::ptime& t)
		{
			std::lock_guard<SpinLock> lock(m_lock);
			m_accessTime = t;
		}

		virtual void SetAskingPrice(double ask)
		{
			m_priceAsk = ask;
		}

		virtual void SetBidPrice(double bid)
		{
			m_priceBid = bid;
		}

		virtual void SetPriceOpen(double price)
		{
			m_priceOpen = price;
		}

		virtual void SetClosingPrice(double price)
		{
			m_priceClose = price;
		}

		virtual void SetDayLow(double dayLow)
		{
			m_dayLow = dayLow;
		}

		virtual void SetDayHigh(double dayHigh)
		{
			m_dayHigh = dayHigh;
		}

		virtual void Set52WkLow(double _52WkLow)
		{
			m_52WkLow = _52WkLow;
		}
		
		virtual void Set52WkHigh(double _52WkHigh)
		{
			m_52WkHigh = _52WkHigh;
		}

		virtual void SetTradePrice(double price)
		{
			m_priceTrade = price;
		}

		virtual void SetPriceChange(double change)
		{
			m_change = change;
		}

		virtual void SetChangePct(double pct)
		{
			m_changePct = pct;
		}

		virtual void SetDivYield(double div)
		{
			m_divYield = div;
		}

		virtual void SetDivShare(double div)
		{
			m_divShare = div;
		}

		virtual void SetEPS(double eps)
		{
			m_eps = eps;
		}

		virtual void SetPE(int pe)
		{
			m_pe = pe;
		}

		virtual void SetShares(int share)
		{
			m_shares = share;
		}

		virtual void SetTradeDate(const dd::date& d)
		{
			std::lock_guard<SpinLock> lock(m_lock);
			m_tradeDate = d;
		}

		virtual void SetTradeTime(const pt::ptime& time)
		{
			std::lock_guard<SpinLock> lock(m_lock);
			m_tradeTime = time;
		}

		virtual void SetVolume(int vol)
		{
			m_volume = vol;
		}	

		virtual void SetBeta(double beta)
		{
			m_beta = beta;
		}

		void SetStock(std::shared_ptr<IStock> stock)
		{
			std::lock_guard<SpinLock> lock(m_lock);
			m_stock = stock;
		}
				
	private:

		pt::ptime m_accessTime;

		/// ask price of the stock. 
		/// yahoo symbol 'a'
		std::atomic<double> m_priceAsk;

		/// bid price of the stock. 
		/// yahoo symbol 'b'
		std::atomic<double> m_priceBid;

		/// The opening price of the stock.
		/// yahoo symbol 'o'
		std::atomic<double> m_priceOpen;

		/// previous close price of the stock. 
		/// yahoo symbol 'p'
		std::atomic<double>  m_priceClose;

		/// The last Traded price of the stock.
		/// yahoo symbol 'l1'
		std::atomic<double> m_priceTrade;

		/// price change
		/// yahoo symbol 'c1'
		std::atomic<double> m_change;

		/// price change percentage
		/// yahoo symbol 'p2'
		std::atomic<double> m_changePct;

		/// dividend yield
		/// yahoo symbol 'y'
		std::atomic<double> m_divYield;

		/// dividend per share
		/// yahoo symbol 'd'
		std::atomic<double> m_divShare;

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

		/// The earings per share value of the stock. 
		/// yahoo symbol 'e'
		std::atomic<double>    m_eps;

		/// The price-to-earnings ratio for this stock.
		/// yahoo symbol 'r'
		std::atomic<double>    m_pe;

		/// The number of shares outstanding of this stock.
		/// yahoo symbol 'j2'
		std::atomic<int>    m_shares;

		/// The volume of the stock.
		/// yahoo symbol 'v'
		std::atomic<int>    m_volume;

		/// The beta of the stock
		std::atomic<double> m_beta;

		/// Name(TYPEID, std::hash<std::string>()(symbol)) 
		/// Key[0] => "symbol"
		Name m_name;

		std::shared_ptr<IStock> m_stock;

		mutable SpinLock m_lock;
	};
}


/* namespace derivative */

#endif /* _DERIVATIVE_STOCKVALUE_H_ */