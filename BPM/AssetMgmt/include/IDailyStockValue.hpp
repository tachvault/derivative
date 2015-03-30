/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_IDAILYSTOCKVALUE_H_
#define _DERIVATIVE_IDAILYSTOCKVALUE_H_

#include <memory>
#include <string>
#include <iostream>  
#include <sstream>      // std::istringstream

#include "Global.hpp"
#include "IAssetValue.hpp"
#include "IStock.hpp"

namespace derivative
{
	/// IDailyStockValueinterface exposes value of a day after the
	/// closing bell.
	class IDailyStockValue : public virtual IObject,
		virtual public IAssetValue
	{

	public:

		enum {TYPEID = INTERFACE_DAILYSTOCKVALUE_TYPE};

		inline static Name ConstructName(const std::string& symbol, const dd::date& date)
		{
			Name nm(TYPEID, std::hash<std::string>()(symbol + dd::to_simple_string(date)));
			nm.AppendKey(string("symbol"), boost::any_cast<string>(symbol));
			nm.AppendKey(string("tradeDate"), boost::any_cast<dd::date>(date));
			return nm;
		}

		inline static void GetKeys(const Name& nm, std::string& symbol, dd::date& date)
		{
			Name::KeyMapType keys = nm.GetKeyMap();
			auto i = keys.find("symbol");
			symbol = boost::any_cast<std::string>(i->second);
			auto k = keys.find("tradeDate");
			date = boost::any_cast<dd::date>(k->second);
		}

		/// SetName with given Name nm
		/// would be used when Setting the actual name
		/// and register with EntityManager after the
		/// IDailyStockValue.
		virtual void SetName(const Name& nm) = 0;

		//// Return the date for this stock value.
		virtual dd::date   GetTradeDate() const = 0;

		/// return day's open price
		virtual double GetPriceOpen() const = 0;

		/// return day's  closing price
		virtual double GetPriceClose() const = 0;

		/// return day's High price 
		virtual double GetPriceHigh() const = 0;

		/// return day's low price 
		virtual double GetPriceLow() const = 0;

		/// return day's adjusted close price
		virtual double GetPriceAdjClose() const = 0;
		
		/// Get the associated stock
		virtual shared_ptr<const IStock> GetStock() const = 0;

		virtual void SetTradeDate(const dd::date& d) = 0;

		/// set day's open price
		virtual void SetPriceOpen(double price) = 0;

		/// set day's closing price
		virtual void SetPriceClose(double price) = 0;

		/// set day's High price 
		virtual void SetPriceHigh(double price) = 0;

		/// set day's low price 
		virtual void SetPriceLow(double price) = 0;

		/// set day's adjusted close price
		virtual void SetPriceAdjClose(double price) = 0;

		virtual void convert(istringstream  &input) = 0;

		virtual void SetDivYield(double yield) = 0;

		virtual void SetStock(shared_ptr<IStock> stock) = 0;

	protected:

		/// you should know the derived type if you are deleting.
		virtual ~IDailyStockValue() 
		{
		}  
	};

	istringstream& operator >> (istringstream& input, shared_ptr<IDailyStockValue>& stockVal)
	{
		stockVal->convert(input);
		return input;
	}
}


/* namespace derivative */

#endif /* _IDERIVATIVE_DAILYSTOCKVALUE_H_ */