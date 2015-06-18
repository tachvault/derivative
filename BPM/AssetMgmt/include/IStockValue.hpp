/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_ISTOCKVALUE_H_
#define _DERIVATIVE_ISTOCKVALUE_H_

#include <string>
#include <memory>

#include "IAssetValue.hpp"
#include "Name.hpp"

namespace derivative
{
	class IStock;

	/// IStockValue interface exposes value of a stock
	/// at a given time. Current value of stock
	/// and day end of the stock will realize IStockValue
	class IStockValue : virtual public IAssetValue
	{

	public:

		enum {TYPEID = INTERFACE_STOCKVALUE_TYPE};

		static Name ConstructName(const std::string& symbol)
		{
			Name nm(TYPEID, std::hash<std::string>()(symbol));
			nm.AppendKey(string("symbol"), boost::any_cast<string>(symbol));
			return nm;
		}

		inline static void GetKeys(const Name& nm, std::string& symbol)
		{
			Name::KeyMapType keys = nm.GetKeyMap();
			auto i = keys.find("symbol");
			symbol = boost::any_cast<std::string>(i->second);
		}

		/// The time, data accessed from external system
		virtual pt::ptime GetAccessTime() const = 0;

		/// return last asking price
		/// yahoo symbol 'a'
		virtual double GetAskingPrice() const = 0;

		/// return bid price 
		/// yahoo symbol 'b'
		virtual double GetBidPrice() const = 0;

		/// get the opening price on the latest trading day
		/// yahoo symbol 'o'
		virtual double GetPriceOpen() const = 0;

		/// return last closed price
		/// yahoo symbol 'p'
		virtual double GetPriceClose() const = 0;

		/// return current market price
		/// yahoo symbol 'l1'
		virtual double GetTradePrice() const = 0;

		/// return price change
		/// yahoo symbol 'c1'
		virtual double GetPriceChange() const = 0;

		/// return percentage of price change
		/// yahoo symbol 'p2'
		virtual double  GetChangePct() const = 0;

		/// return the div yield (IAssetValue virtual function)
		/// yahoo symbol 'y'
		///virtual double GetDivYield() const = 0;

		/// return div share
		/// yahoo symbol 'd'
		virtual double GetDivShare() const = 0;

		/// return earning per share
		/// yahoo symbol 'e'
		virtual double  GetEPS() const = 0;

		//// Return the price-to-earnings ratio for this stock.
		/// yahoo symbol 'r'
		virtual double  GetPE() const = 0;

		//// Return the double of shares outstanding of this stock.
		/// yahoo symbol 'j2'
		virtual int GetShares() const = 0;
		/// yahoo symbol 'g'
		virtual double GetdayLow() const = 0;

		/// yahoo symbol 'h'
		virtual double  GetDayHigh() const = 0;

		/// yahoo symbol 'j'
		virtual double Get52WkLow() const = 0;
		
		/// yahoo symbol 'k'
		virtual double Get52WkHigh() const = 0;

		/// yahoo symbol  'd1'
		virtual dd::date   GetTradeDate() const = 0;

		//// Return the time this stock was last traded.
		/// yahoo symbol  't1'
		virtual pt::ptime   GetTradeTime() const = 0;

		//// Return the volume of the stock.
		/// yahoo symbol 'v'
		virtual int GetVolume() const = 0;

		/// get Beta
		virtual double GetBeta() const = 0;

		/// Get the associated stock
		virtual std::shared_ptr<IStock> GetStock() const = 0;

		virtual void SetAccessTime(const pt::ptime& t) = 0;

		virtual void SetAskingPrice(double ask) = 0;

		virtual void SetBidPrice(double bid) = 0;

		virtual void SetPriceOpen(double price) = 0;

		virtual void SetClosingPrice(double price) = 0;

		virtual void SetDayLow(double dayLow) = 0;

		virtual void SetDayHigh(double dayHigh) = 0;

		virtual void Set52WkLow(double _52WkLow) = 0;
		
		virtual void Set52WkHigh(double _52WkHigh) = 0; 

		virtual void SetTradePrice(double price) = 0;

		virtual void SetPriceChange(double change) = 0;

		virtual void SetChangePct(double pct) = 0;

		virtual void SetDivYield(double div) = 0;

		virtual void SetDivShare(double div) = 0; 

		virtual void SetEPS(double eps) = 0;

		virtual void SetPE(int pe) = 0;

		virtual void SetShares(int share) = 0;

		virtual void SetTradeDate(const dd::date& d) = 0;

		virtual void SetTradeTime(const pt::ptime& time) = 0;

		virtual void SetVolume(int vol) = 0;

		virtual void SetBeta(double beta) = 0;

		virtual void convert( istringstream  &input) = 0;

		virtual void SetStock(std::shared_ptr<IStock> stock) = 0;

	protected:

		/// you should know the derived type if you are deleting.
		virtual ~IStockValue() 
		{
		}  
	};

	inline istringstream& operator >> (istringstream& input, std::shared_ptr<IStockValue>& stockVal)
	{
		stockVal->convert(input);
		return input;
	}	
}
/* namespace derivative */

#endif /* _IDERIVATIVE_STOCKVALUE_H_ */