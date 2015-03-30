/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_IEXCHANGERATEVALUE_H_
#define _DERIVATIVE_IEXCHANGERATEVALUE_H_

#include <string>
#include <memory>

#include "IAssetValue.hpp"
#include "Name.hpp"

namespace derivative
{
	class IExchangeRate;

	/// IExchangeRateValue interface exposes value of an exchange
	/// at a given time. Current value of exchange rate
	/// and day end of the exchange rate will realize IExchangeRateValue
	class IExchangeRateValue : public virtual IAssetValue
	{

	public:

		enum {TYPEID = INTERFACE_IEXCHANGERATEVALUE_TYPE};

		static Name ConstructName(const std::string& domestic,const std::string& foreign)
		{
			Name nm(TYPEID, std::hash<std::string>()(domestic + foreign));
			nm.AppendKey(string("domestic"), boost::any_cast<string>(domestic));
			nm.AppendKey(string("foreign"), boost::any_cast<string>(foreign));
			return nm;
		}

		inline static void GetKeys(const Name& nm, std::string& domestic, std::string& foreign)
		{
			Name::KeyMapType keys = nm.GetKeyMap();
			auto i = keys.find("domestic");
			domestic = boost::any_cast<std::string>(i->second);
			auto j = keys.find("foreign");
			foreign = boost::any_cast<std::string>(j->second);
		}

		/// return last asking price
		/// yahoo symbol 
		virtual double GetAskingPrice() const = 0;

		/// return bid price 
		/// yahoo symbol 
		virtual double GetBidPrice() const = 0;

		/// return last openned price
		/// yahoo symbol 
		virtual double GetPriceOpen() const = 0;

		/// return last closed price
		/// yahoo symbol 
		virtual double GetPriceClose() const = 0;

		/// return current market price
		/// yahoo symbol 'l1'
		virtual double GetTradePrice() const = 0;

		/// return price change
		/// yahoo symbol 
		virtual double GetdayLow() const = 0;

		/// return percentage of price change
		/// yahoo symbol 
		virtual double  GetDayHigh() const = 0;

		/// return the div yield
		/// yahoo symbol 
		virtual double Get52WkLow() const = 0;

		/// return div share
		/// yahoo symbol 
		virtual double Get52WkHigh() const = 0;

		//// Return the date this stock was last traded.
		/// yahoo symbol  
		virtual dd::date   GetTradeDate() const = 0;

		//// Return the time this stock was last traded.
		/// yahoo symbol  
		virtual pt::ptime   GetTradeTime() const = 0;

		/// Get the associated stock
		virtual std::shared_ptr<const IExchangeRate> GetExchangeRate() const = 0;

		virtual void SetAskingPrice(double ask) = 0;

		virtual void SetBidPrice(double bid) = 0;

		virtual void SetPriceOpen(double price) = 0;

		virtual void SetClosingPrice(double price) = 0;

		virtual void SetTradePrice(double price) = 0;

		virtual void SetDayLow(double dayLow) = 0;

		virtual void SetDayHigh(double dayHigh) = 0;

		virtual void Set52WkLow(double _52WkLow) = 0;

		virtual void Set52WkHigh(double _52WkHigh) = 0; 

		virtual void SetTradeDate(const dd::date& d) = 0;

		virtual void SetTradeTime(const pt::ptime& time) = 0;

		virtual void convert( istringstream  &input) = 0;

		virtual void SetExchangeRate(std::shared_ptr<IExchangeRate> exchangeRate) = 0;

	protected:

		/// you should know the derived type if you are deleting.
		virtual ~IExchangeRateValue() 
		{
		}  
	};

	inline istringstream& operator >> (istringstream& input, std::shared_ptr<IExchangeRateValue>& exchangeRateVal)
	{
		exchangeRateVal->convert(input);
		return input;
	}
}


/* namespace derivative */

#endif /* _IDERIVATIVE_EXCHANGERATEVALUE_H_ */