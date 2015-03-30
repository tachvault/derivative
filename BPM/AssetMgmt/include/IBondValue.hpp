/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_IBONDVALUE_H_
#define _DERIVATIVE_IBONDVALUE_H_

#include <string>
#include <memory>

#include "IAssetValue.hpp"
#include "DeterministicCashflow.hpp"

namespace derivative
{
	class IBond;

	/// IBondValue interface exposes value of a bond
	/// (all type of bonds ) at a given time. 
	class IBondValue : virtual public IAssetValue
	{

	public:

		enum {TYPEID = INTERFACE_BONDVALUE_TYPE};

		/// return current market price. Trade price includes
		/// quoted price plus acurred interest
		virtual double GetTradePrice() const = 0;

		/// price quoted.
		virtual double GetQuotedPrice() const = 0;

		/// returns the trade date when the price is reported
		virtual dd::date GetTradeDate() const = 0;

		/// returns the processed date. i.e when this IRCurve is created.
		virtual dd::date GetProcessedDate() const = 0;

		/// return the yield
		virtual double GetYield() const = 0;

		/// The maturity date of the bond
		virtual dd::date GetMaturityDate() const = 0;

		/// Set the current trade price (quoted + acurred interest)
		virtual void SetTradePrice(double price) = 0;

		/// Set the current market (quoted) price
		virtual void SetQuotedPrice(double price) = 0;

		/// Set the trade date when the price is reported
		virtual  void SetTradeDate(const dd::date& date) = 0;

		/// Set the div yield
		virtual void SetYield(double yield) = 0;

		/// The maturity date of the bond
		virtual void  SetMaturityDate(const dd::date& matDate) = 0;

		/// Setter for processed date
		virtual void SetProcessedDate(const dd::date& pdate) = 0;

	protected:

		/// you should know the derived type if you are deleting.
		virtual ~IBondValue() 
		{
		}  
	};	
}


/* namespace derivative */

#endif /* _IDERIVATIVE_BONDVALUE_H_ */