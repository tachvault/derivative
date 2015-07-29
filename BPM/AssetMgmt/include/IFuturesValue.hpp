/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_IFUTURESVALUE_H_
#define _DERIVATIVE_IFUTURESVALUE_H_

#include <string>
#include <memory>

#include "IAssetValue.hpp"
#include "Name.hpp"

namespace derivative
{
	class IFutures;

	/// IFuturesValue interface exposes value of a futures
	/// at a given time. Current value of futures
	/// and day end of the futures will realize IFuturesValue
	class IFuturesValue : virtual public IAssetValue
	{

	public:

		enum {TYPEID = INTERFACE_FUTURESVALUE_TYPE};

		static Name ConstructName(const std::string& symbol, const dd::date& tradedate, const dd::date& deliverydate)
		{
			Name nm(TYPEID, std::hash<std::string>()(symbol + dd::to_simple_string(tradedate) + dd::to_simple_string(deliverydate)));
			nm.AppendKey(string("symbol"), boost::any_cast<string>(symbol));
			nm.AppendKey(string("tradedate"), boost::any_cast<dd::date>(tradedate));
			nm.AppendKey(string("deliverydate"), boost::any_cast<dd::date>(deliverydate));
			return nm;
		}

		inline static void GetKeys(const Name& nm, std::string& symbol, dd::date& tradedate, dd::date& deliverydate)
		{
			Name::KeyMapType keys = nm.GetKeyMap();
			auto i = keys.find("symbol");
			symbol = boost::any_cast<std::string>(i->second);
			auto j = keys.find("tradedate");
			tradedate = boost::any_cast<dd::date>(j->second);
			auto k = keys.find("deliverydate");
			deliverydate = boost::any_cast<dd::date>(k->second);
		}

		/// The time, data accessed from external system
		virtual pt::ptime GetAccessTime() const = 0;

		virtual double GetHighPrice() const = 0;

		virtual double GetLowPrice() const = 0;

		virtual double GetPriceOpen() const = 0;

		virtual double GetPriceLast() const = 0;

		virtual double GetTradePrice() const = 0;

		virtual double GetSettledPrice() const = 0;

		virtual dd::date GetTradeDate() const = 0;

		virtual dd::date GetDeliveryDate() const = 0;
		
		virtual int GetVolume() const = 0;

		virtual int GetOpenInterest() const = 0;

		virtual std::shared_ptr<IFutures> GetFutures() const = 0;
		
		virtual void SetAccessTime(const pt::ptime& t) = 0;
		
		virtual void SetHighPrice(double ask) = 0;

		virtual void SetLowPrice(double bid) = 0;

		virtual void SetPriceOpen(double price) = 0;

		virtual void SetLastPrice(double price) = 0;

		virtual void SetSettledPrice(double price) = 0;

		virtual void SetTradePrice(double price) = 0;

		virtual void SetTradeDate(const dd::date& d) = 0;

		virtual void SetDeliveryDate(const dd::date& d) = 0;

		virtual void SetVolume(int vol) = 0;

		virtual void SetOpenInterest(int openInt) = 0;

		virtual void convert( istringstream  &input) = 0;

		virtual void SetFutures(std::shared_ptr<IFutures> future) = 0;

	protected:

		/// you should know the derived type if you are deleting.
		virtual ~IFuturesValue() 
		{
		}  
	};

	inline istringstream& operator >> (istringstream& input, std::shared_ptr<IFuturesValue>& futuresVal)
	{
		futuresVal->convert(input);
		return input;
	}
}
/* namespace derivative */

#endif /* _IDERIVATIVE_FUTURESVALUE_H_ */