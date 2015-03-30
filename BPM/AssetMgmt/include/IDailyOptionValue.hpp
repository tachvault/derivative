/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_IDAILYOPTIONVALUE_H_
#define _DERIVATIVE_IDAILYOPTIONVALUE_H_

#include <memory>
#include <string>
#include <iostream>  
#include <sstream>      // std::istringstream

#include "Global.hpp"
#include "IAssetValue.hpp"
#include "IStock.hpp"

namespace derivative
{
	/// IDailyEquityOptionValue interface exposes daily value of options.
	class IDailyOptionValue : public virtual IObject,
		virtual public IAssetValue
	{

	public:

		enum {TYPEID = INTERFACE_DAILYOPTIONVALUE_TYPE};

		enum OptionType { OPTION_TYPE_UNKNOWN = 0, VANILLA_CALL = 1, VANILLA_PUT =2};
				
		virtual OptionType GetOptionType() const = 0;

		//// Return the date for this option value.
		virtual dd::date   GetTradeDate() const = 0;

		/// return strike price
		virtual double GetStrikePrice() const = 0;

		/// maturity date
		virtual dd::date GetMaturityDate() const = 0;

		/// get volume
		virtual int GetVolume() const = 0;

		/// get open interest
		virtual int GetOpenInterest() const = 0;

		virtual void SetTradeDate(const dd::date& d) = 0;

		virtual void SetTradePrice(double price) = 0;

		virtual void SetOptionType(const OptionType& optType) = 0;

		virtual void SetMaturityDate(const dd::date& date) = 0;

		virtual void SetStrikePrice(double strike) = 0;

		virtual void SetVolume(int vol) = 0;

		virtual void SetOpenInterest(int openInt) = 0;

	protected:

		/// you should know the derived type if you are deleting.
		virtual ~IDailyOptionValue() 
		{
		}  
	};
}


/* namespace derivative */

#endif /* _IDERIVATIVE_DAILYOPTIONVALUE_H_ */