/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_FIXEDRATEBONDVALUE_H_
#define _DERIVATIVE_FIXEDRATEBONDVALUE_H_

#include <string>
#include <memory>
#include <exception>

#include "IMake.hpp"
#include "Name.hpp"
#include "IFixedRateBondValue.hpp"
#include "IIRDataSrc.hpp"

namespace derivative
{
	/// Class FixedRateBondValue represents current value of a fixedRateBond
	/// It is constructed as a Named type. That is, only only one
	/// shared object per fixedRateBond would be shared among all client(s)
	/// threads. Callers will have the ability to refresh from external source.
	class FixedRateBondValue : virtual public IFixedRateBondValue,
		virtual public IIRDataSrc,
		virtual public IMake
	{

	public:

		enum {TYPEID = CLASS_FIXEDRATEBONDVALUE_TYPE};

		/// Constructor with Exemplar for the Creator FixedRateBond value object
		FixedRateBondValue (const Exemplar &ex);

		/// construct FixedRateBondValue from a given Name
		/// used in IMake::Make(..)
		/// The object ID would be the same as that of underlying fixedRateBond.
		FixedRateBondValue (const Name& nm);

		virtual ~FixedRateBondValue()
		{}

		shared_ptr<IMake> Make (const Name &nm);

		shared_ptr<IMake> Make (const Name &nm, const deque<boost::any>& agrs);

		const Name& GetName()
		{
			return m_name;
		}

		virtual double GetTradePrice() const
		{
			return m_tradePrice;
		}

		virtual double GetQuotedPrice() const
		{
			return m_quotedPrice;
		}

		/// returns the trade date when the price is reported
		virtual dd::date GetTradeDate() const
		{
			return m_tradeDate;
		}

		/// return the yield
		virtual double GetYield() const
		{
			return m_yield;
		}

		/// The maturity date of the bond
		virtual dd::date GetMaturityDate() const
		{
			return m_maturityDate;
		}

		/// Set the trade date when the price is reported
		virtual  void SetTradeDate(const dd::date& date)
		{
			m_tradeDate = date;
		}

		virtual dd::date GetProcessedDate() const 
		{
			return m_processedDate;
		}

		/// The processed date of the bond
		virtual void  SetProcessedDate(const dd::date& pDate)
		{
			m_processedDate = pDate;
		}

		/// Set the div yield
		virtual void SetYield(double yield)
		{
			m_yield = yield;
		}

		virtual void SetTradePrice(double price)
		{
			m_tradePrice = price;
		}

		virtual void SetQuotedPrice(double price)
		{
			m_quotedPrice = price;
		}

		/// The maturity date of the bond
		virtual void  SetMaturityDate(const dd::date& matDate)
		{
			m_maturityDate = matDate;
		}

		virtual double GetDivYield() const
		{
			throw std::logic_error("Invalid call");
		}

		virtual std::shared_ptr<IAsset> GetAsset() const
		{
			return m_bond;
		}

		/// return bond.
		shared_ptr<const IFixedRateBond> GetBond() const
		{
			return m_bond;
		}
		
		virtual std::shared_ptr<IFixedRateBond> GetFixedRateBond() const
		{
			return m_bond;
		}

		virtual void SetFixedRateBond(std::shared_ptr<IFixedRateBond> bond)
		{
			m_bond = bond;
		}

		/// construct FixedRateBondValue from input stream.		
		virtual void convert( istringstream  &input);

		virtual void generateCashFlow();

		/// return cash flow generated by this bond
		virtual std::shared_ptr<cashFlowSetType> getCashFlowMap() const
		{
			return m_cashFlow;
		}

	private:

		/// Name(TYPEID, std::hash<std::string>()(symbol)) 
		/// Key[0] => "symbol"
		Name m_name;

		double m_quotedPrice;

		double m_tradePrice;

		dd::date m_tradeDate;

		dd::date m_processedDate;

		double m_yield;

		dd::date m_maturityDate;

		std::shared_ptr<IFixedRateBond> m_bond;

		std::shared_ptr<cashFlowSetType> m_cashFlow;
	};
}


/* namespace derivative */

#endif /* _DERIVATIVE_FIXEDRATEBONDVALUE_H_ */