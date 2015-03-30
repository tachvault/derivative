/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_IDAILYEXCHANGERATEVALUE_H_
#define _DERIVATIVE_IDAILYEXCHANGERATEVALUE_H_

#include <memory>
#include <string>
#include <iostream>  
#include <sstream>      // std::istringstream

#include "Global.hpp"
#include "IAssetValue.hpp"
#include "IExchangeRate.hpp"

namespace derivative
{
	/// IDailyExchangeRateValueinterface exposes value of a day after the
	/// closing bell.
	class IDailyExchangeRateValue : public virtual IObject,
		virtual public IAssetValue
	{

	public:

		enum {TYPEID = INTERFACE_DAILYEXCHANGERATEVALUE_TYPE};

		static Name ConstructName(const std::string& domestic,const std::string& foreign, const dd::date& tradeDate)
		{
			Name nm(TYPEID, std::hash<std::string>()(domestic + foreign + dd::to_simple_string(tradeDate)));
			nm.AppendKey(string("domestic"), boost::any_cast<string>(domestic));
			nm.AppendKey(string("foreign"), boost::any_cast<string>(foreign));
			nm.AppendKey(string("tradeDate"), boost::any_cast<dd::date>(tradeDate));
			return nm;
		}

		inline static void GetKeys(const Name& nm, std::string& domestic, std::string& foreign, dd::date& tradeDate)
		{
			Name::KeyMapType keys = nm.GetKeyMap();
			auto i = keys.find("domestic");
			domestic = boost::any_cast<std::string>(i->second);
			auto j = keys.find("foreign");
			foreign = boost::any_cast<std::string>(j->second);
			auto k = keys.find("tradeDate");
			tradeDate = boost::any_cast<dd::date>(k->second);
		}

		/// SetName with given Name nm
		/// would be used when Setting the actual name
		/// and register with EntityManager after the
		/// IDailyExchangeRateValue.
		virtual void SetName(const Name& nm) = 0;

		//// Return the date for this exchangeRate value.
		virtual dd::date   GetTradeDate() const = 0;

		/// return day's open price
		virtual double GetPriceOpen() const = 0;

		/// return day's  closing price
		virtual double GetPriceClose() const = 0;

		/// return day's High price 
		virtual double GetPriceHigh() const = 0;

		/// return day's low price 
		virtual double GetPriceLow() const = 0;

		/// Get the associated exchangeRate
		virtual shared_ptr<const IExchangeRate> GetExchangeRate() const = 0;

		virtual void SetTradeDate(const dd::date& d) = 0;

		/// set day's open price
		virtual void SetPriceOpen(double price) = 0;

		/// set day's closing price
		virtual void SetPriceClose(double price) = 0;

		/// set day's High price 
		virtual void SetPriceHigh(double price) = 0;

		/// set day's low price 
		virtual void SetPriceLow(double price) = 0;

		virtual void convert(istringstream  &input) = 0;

		virtual void SetExchangeRate(shared_ptr<IExchangeRate> exchangeRate) = 0;

	protected:

		/// you should know the derived type if you are deleting.
		virtual ~IDailyExchangeRateValue() 
		{
		}  
	};

	istringstream& operator >> (istringstream& input, shared_ptr<IDailyExchangeRateValue>& exchangeRateVal)
	{
		exchangeRateVal->convert(input);
		return input;
	}
}


/* namespace derivative */

#endif /* _IDERIVATIVE_DAILYEXCHANGERATEVALUE_H_ */