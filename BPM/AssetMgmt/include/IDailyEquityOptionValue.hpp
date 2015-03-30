/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_IDAILYEQUITYOPTIONVALUE_H_
#define _DERIVATIVE_IDAILYEQUITYOPTIONVALUE_H_

#include <memory>
#include <string>
#include <iostream>  
#include <sstream>      // std::istringstream

#include "IDailyOptionValue.hpp"

namespace derivative
{
	class IStock;
	class IStockValue;

	/// IDailyEquityOptionValue interface exposes daily value of options.
	class IDailyEquityOptionValue : public virtual IDailyOptionValue
	{

	public:

		enum {TYPEID = INTERFACE_DAILYEQUITYOPTIONVALUE_TYPE};

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
		/// IDailyEquityOptionValue.
		virtual void SetName(const Name& nm) = 0;

		/// return last asking price
		virtual double GetAskingPrice() const = 0;

		/// return bid price 
		virtual double GetBidPrice() const = 0;

		/// Get the associated stock
		virtual shared_ptr<const IStock> GetStock() const = 0;

		/// set day's asking price
		virtual void SetAskingPrice(double price) = 0;

		/// set day's bid price
		virtual void SetBidPrice(double price) = 0;
				
		virtual void convert(istringstream  &input) = 0;
				
		virtual void SetOption(shared_ptr<IStock> stock) = 0;

	protected:

		/// you should know the derived type if you are deleting.
		virtual ~IDailyEquityOptionValue() 
		{
		}  
	};

	inline istringstream& operator >> (istringstream& input, shared_ptr<IDailyEquityOptionValue>& optionVal)
	{
		optionVal->convert(input);
		return input;
	}
}


/* namespace derivative */

#endif /* _IDERIVATIVE_DAILYEQUITYOPTIONVALUE_H_ */