/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_ZEROCOUPONBOND_H_
#define _DERIVATIVE_ZEROCOUPONBOND_H_

#include <memory>

#include "IMake.hpp"
#include "Name.hpp"
#include "IFixedRateBond.hpp"
#include "Country.hpp"
#include "Exchange.hpp"

namespace derivative
{
	/// IFixedRateBond provides public interface for FixedRateBond class.
	class FixedRateBond : virtual public IFixedRateBond,
		virtual public IMake
	{
	public:

		enum {TYPEID = CLASS_FIXEDRATEBOND_TYPE};

		/// Constructor with Exemplar for the Creator FixedRateBond object
		FixedRateBond (const Exemplar &ex);

		/// construct FixedRateBond from a given Name
		/// used in IMake::Make(..)
		FixedRateBond (const Name& nm);

		virtual ~FixedRateBond();

		/// constructor.
		FixedRateBond(const std::string& symbol, const std::string& description, const Currency& curr, \
			const Country& cntry, double faceVal, CouponPeriodType period, double couponRate);

		virtual std::shared_ptr<IMake> Make (const Name &nm);

		virtual std::shared_ptr<IMake> Make (const Name &nm, const std::deque<boost::any>& agrs);

		const Name& GetName()
		{
			return m_name;
		}

		const std::string& GetSymbol() const
		{
			return m_symbol;
		}

		const std::string& GetDescription() const
		{
			return m_description;
		}

		virtual CategoryType GetCategory() const
		{
			return m_category;
		}

		virtual double GetImpliedVol() const
		{
			throw std::logic_error("Not supported");
		}

		virtual double GetHistVol() const
		{
			throw std::logic_error("Not supported");
		}

		virtual void SetCategory(const CategoryType& cat)
		{
			m_category = cat;
		}

        const Currency& GetDomesticCurrency() const
		{
			return m_currency;
		}

		const Country& GetCountry() const
		{
			return m_country;
		}

		void SetSymbol(const std::string& sym)
		{
			m_symbol = sym;
		}

		void SetDomesticCurrency(const Currency& cur)
		{
			m_currency = cur;
		}

		void SetDescription(const std::string& des)
		{
			m_description = des;
		}
		
		void SetCountry(const Country& cntry)
		{
			m_country = cntry;
		}

		virtual DayCount::DayCountType GetDayCount() const
		{
			return m_dayCount;
		}

		virtual void SetDayCount(const DayCount::DayCountType& count)
		{
			m_dayCount = count;
		}

		virtual double GetFaceValue() const
		{
			return m_faceValue;
		}

		virtual void SetFaceValue(double val)
		{
			m_faceValue = val;
		}

		virtual CouponPeriodType GetCouponPeriod() const
		{
			return m_couponPeriod;
		}

		virtual void SetCouponPeriod(const CouponPeriodType& val)
		{
			m_couponPeriod = val;
		}

		virtual double GetCouponRate() const
		{
			return m_couponRate;
		}

		virtual void SetCouponRate(double val)
		{
			m_couponRate = val;
		}

	private:

		/// Name(TYPEID, std::hash<std::string>()(ticker symbol))
		/// Key[0] => "symbol"
		Name m_name;

		std::string m_symbol;

		Currency m_currency;

		std::string m_description;

		Country m_country;

		CategoryType m_category;

		double m_faceValue;

		/// couponPeriod type
		CouponPeriodType m_couponPeriod;

		/// couponRate as percentage value
		double m_couponRate;

		DayCount::DayCountType m_dayCount;
	};	

} /* namespace derivative */

#endif /* _DERIVATIVE_IZEROCOUPONBOND_H_ */
