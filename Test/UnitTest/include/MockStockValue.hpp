/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_STOCKVALUE_H_
#define _DERIVATIVE_STOCKVALUE_H_

#include <string>
#include <memory>
#include "GroupRegister.hpp"
#include "IStockValue.hpp"
#include "Name.hpp"
#include "IStock.hpp"
#include "ConstVol.hpp"

namespace derivative
{
	/// Class MockStockValue represents current value of a stock
	/// It is constructed as a Named type. That is, only only one
	/// shared object per stock would be shared among all client(s)
	/// threads. Callers will have the ability to refresh from external source.
	class MockStockValue : public IStockValue
	{

	public:

		enum { TYPEID = CLASS_MOCKSTOCKVALUE_TYPE };

		MockStockValue(const Name& nm)
			:m_name(nm)
		{}

		const Name& GetName()
		{
			return m_name;
		}

		void SetName(const Name& nm)
		{
			m_name = nm;
		}

		virtual pt::ptime GetAccessTime() const
		{
			throw std::logic_error("not supported");
		}

		virtual void SetAccessTime(const pt::ptime& t)
		{
			throw std::logic_error("not supported");
		}

		shared_ptr<IStock> GetStock() const
		{
			return m_asset;
		}

		void SetStock(std::shared_ptr<IStock> stock)
		{
			m_asset = stock;
		}

		/// return current market price
		double GetTradePrice() const
		{
			return m_priceTrade;
		}

		/// return the div yield
		double GetDivYield() const
		{
			return m_divYield;
		}

		virtual std::shared_ptr<IAsset> GetAsset() const
		{
			throw std::logic_error("not supported");
		}

		/// return current market price
		void SetTradePrice(double price)
		{
			m_priceTrade = price;
		}

		/// return the div yield
		void SetDivYield(double yield)
		{
			m_divYield = yield;
		}

		virtual std::shared_ptr<DeterministicAssetVol> GetVolatility(double strike) const
		{
			Array<double, 1> sgm1(2);
			sgm1 = m_asset->GetImpliedVol();
			return std::make_shared<ConstVol>(sgm1);
		}

		virtual void SetVolatility(double vol)
		{
			throw std::logic_error("not supported");
		}

		//// Return the date this stock was last traded.
		virtual dd::date   GetTradeDate() const
		{
			throw std::logic_error("not supported");
		}

		//// Return the time this stock was last traded.
		virtual pt::ptime  GetTradeTime() const
		{
			throw std::logic_error("not supported");
		}

		virtual void SetTradeDate(const dd::date& d)
		{
			throw std::logic_error("not supported");
		}

		virtual void SetTradeTime(const pt::ptime& time)
		{
			throw std::logic_error("not supported");
		}

		/// construct StockValue from input stream.		
		virtual void convert(istringstream  &input)
		{
			throw std::logic_error("not supported");
		}

		virtual double Get52WkHigh() const
		{
			throw std::logic_error("not supported");
		}
		
		/// return last asking price
		double GetAskingPrice() const
		{
			throw std::logic_error("not supported");
		}

		/// return bid price 
		double GetBidPrice() const
		{
			throw std::logic_error("not supported");
		}

		/// return last openned price
		double GetPriceOpen() const
		{
			throw std::logic_error("not supported");
		}

		/// return last closed price
		double GetPriceClose() const
		{
			throw std::logic_error("not supported");
		}

		/// return price change
		double GetPriceChange() const
		{
			throw std::logic_error("not supported");
		}

		/// return percentage of price change
		double  GetChangePct() const
		{
			throw std::logic_error("not supported");
		}

		/// return div share
		double GetDivShare() const
		{
			throw std::logic_error("not supported");
		}

		/// return earning per share
		double  GetEPS() const
		{
			throw std::logic_error("not supported");
		}

		//// Return the price-to-earnings ratio for this stock.
		double  GetPE() const
		{
			throw std::logic_error("not supported");
		}

		//// Return the double of shares outstanding of this stock.
		int GetShares() const
		{
			throw std::logic_error("not supported");
		}

		/// yahoo symbol 'g'
		virtual double GetdayLow() const
		{
			throw std::logic_error("not supported");
		}

		/// yahoo symbol 'h'
		virtual double  GetDayHigh() const
		{
			throw std::logic_error("not supported");
		}

		/// yahoo symbol 'j'
		virtual double Get52WkLow() const
		{
			throw std::logic_error("not supported");
		}

		//// Return the volume of the stock.
		virtual int GetVolume() const
		{
			throw std::logic_error("not supported");
		}

		virtual double GetBeta() const
		{
			throw std::logic_error("not supported");
		}

		virtual void SetAskingPrice(double ask)
		{
			throw std::logic_error("not supported");
		}

		virtual void SetBidPrice(double bid)
		{
			throw std::logic_error("not supported");
		}

		virtual void SetPriceOpen(double price)
		{
			throw std::logic_error("not supported");
		}

		virtual void SetClosingPrice(double price)
		{
			throw std::logic_error("not supported");
		}

		virtual void SetDayLow(double dayLow)
		{
			throw std::logic_error("not supported");
		}

		virtual void SetDayHigh(double dayHigh)
		{
			throw std::logic_error("not supported");
		}

		virtual void Set52WkLow(double _52WkLow)
		{
			throw std::logic_error("not supported");
		}

		virtual void Set52WkHigh(double _52WkHigh)
		{
			throw std::logic_error("not supported");
		}

		virtual void SetPriceChange(double change)
		{
			throw std::logic_error("not supported");
		}

		virtual void SetChangePct(double pct)
		{
			throw std::logic_error("not supported");
		}

		virtual void SetDivShare(double div)
		{
			throw std::logic_error("not supported");
		}

		virtual void SetEPS(double eps)
		{
			throw std::logic_error("not supported");
		}

		virtual void SetPE(int pe)
		{
			throw std::logic_error("not supported");
		}

		virtual void SetShares(int share)
		{
			throw std::logic_error("not supported");
		}

		virtual void SetVolume(int vol)
		{
			throw std::logic_error("not supported");
		}

		virtual void SetBeta(double beta)
		{
			throw std::logic_error("not supported");
		}
		
	private:

		Name m_name;

		double m_priceTrade;

		double m_divYield;

		std::shared_ptr<IStock> m_asset;
	};

	ALIAS_REGISTER(MockStockValue, IAssetValue);
	ALIAS_REGISTER(MockStockValue, IStockValue);
}


/* namespace derivative */

#endif /* _DERIVATIVE_STOCKVALUE_H_ */