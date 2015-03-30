/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_IDAILYFUTURESOPTIONVALUE_H_
#define _DERIVATIVE_IDAILYFUTURESOPTIONVALUE_H_

#include <memory>
#include <string>
#include <iostream>  
#include <sstream>      // std::istringstream

#include "IDailyOptionValue.hpp"

namespace derivative
{
	class IFutures;
	class IFuturesValue;

	/// IDailyFuturesOptionValue interface exposes daily value of options.
	class IDailyFuturesOptionValue : public virtual IDailyOptionValue
	{

	public:

		enum {TYPEID = INTERFACE_DAILYFUTURESOPTIONVALUE_TYPE};

		inline static Name ConstructName(const std::string& symbol, const dd::date& tDate, \
			const OptionType& optType, const dd::date& mDate, double strike)
		{
			Name nm(TYPEID, std::hash<std::string>()(symbol + to_string(optType) + dd::to_simple_string(tDate) + \
				dd::to_simple_string(mDate) + to_string(strike)));
			nm.AppendKey(string("symbol"), boost::any_cast<string>(symbol));
			nm.AppendKey(string("tradeDate"), boost::any_cast<dd::date>(tDate));
			nm.AppendKey(string("optionType"), boost::any_cast<int>(static_cast<int>(optType)));
			nm.AppendKey(string("maturityDate"), boost::any_cast<dd::date>(mDate));
			nm.AppendKey(string("strike"), boost::any_cast<double>(strike));
			return nm;
		}

		inline static void GetKeys(const Name& nm, std::string& symbol, dd::date& tDate, \
			OptionType& optType, dd::date& mDate, double& strike)
		{
			Name::KeyMapType keys = nm.GetKeyMap();
			auto i = keys.find("symbol");
			symbol = boost::any_cast<std::string>(i->second);
			auto j = keys.find("tradeDate");
			tDate = boost::any_cast<dd::date>(j->second);
			auto k = keys.find("maturityDate");
			mDate = boost::any_cast<dd::date>(k->second);
			auto l = keys.find("optionType");
			optType = static_cast<OptionType>(boost::any_cast<int>(l->second));
			auto m = keys.find("strike");
			strike = boost::any_cast<double>(m->second);
		}

		/// SetName with given Name nm
		/// would be used when Setting the actual name
		/// and register with EntityManager after the
		/// IDailyFuturesOptionValue.
		virtual void SetName(const Name& nm) = 0;

		//// Return futures delivery date
		virtual dd::date GetDeliveryDate() const = 0;

		virtual double GetSettledPrice() const = 0;

		virtual double GetHighPrice() const = 0;

		virtual double GetLowPrice() const = 0;

		/// Get the associated futures
		virtual shared_ptr<const IFutures> GetFutures() const = 0;

		virtual void SetHighPrice(double price) = 0;

		virtual void SetLowPrice(double price) = 0;

		virtual void SetSettledPrice(double price) = 0;

		virtual void SetDeliveryDate(const dd::date& ddate) = 0;

		virtual void convert(istringstream  &input) = 0;
				
		virtual void SetOption(shared_ptr<IFutures> futures) = 0;

	protected:

		/// you should know the derived type if you are deleting.
		virtual ~IDailyFuturesOptionValue() 
		{
		}  
	};

	inline istringstream& operator >> (istringstream& input, shared_ptr<IDailyFuturesOptionValue>& optionVal)
	{
		optionVal->convert(input);
		return input;
	}
}


/* namespace derivative */

#endif /* _IDERIVATIVE_DAILYFUTURESOPTIONVALUE_H_ */