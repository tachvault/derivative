/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_ZEROCOUPONBOND_H_
#define _DERIVATIVE_ZEROCOUPONBOND_H_

#include <memory>
#include "IMake.hpp"
#include "Name.hpp"
#include "IZeroCouponBond.hpp"
#include "Country.hpp"
#include "Currency.hpp"

namespace derivative
{
	/// IZeroCouponBond provides public interface for ZeroCouponBond class.
	class ZeroCouponBond : virtual public IZeroCouponBond,
		virtual public IMake
	{
	public:

		enum {TYPEID = CLASS_ZEROCOUPONBOND_TYPE};

		/// Constructor with Exemplar for the Creator ZeroCouponBond object
		ZeroCouponBond (const Exemplar &ex);

		/// construct ZeroCouponBond from a given Name
		/// used in IMake::Make(..)
		ZeroCouponBond (const Name& nm);

		virtual ~ZeroCouponBond();

		/// constructor.
		ZeroCouponBond(const std::string& symbol, const std::string& description, const Currency& curr, \
			const Country& cntry, double faceVal);

		virtual std::shared_ptr<IMake> Make (const Name &nm);

		virtual std::shared_ptr<IMake> Make (const Name &nm, const std::deque<boost::any>& agrs);

		const Name& GetName()
		{
			std::lock_guard<SpinLock> lock(m_lock);
			return m_name;
		}

		const std::string& GetSymbol() const
		{
			std::lock_guard<SpinLock> lock(m_lock);
			return m_symbol;
		}

		virtual CategoryType GetCategory() const
		{
			std::lock_guard<SpinLock> lock(m_lock);
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
			std::lock_guard<SpinLock> lock(m_lock);
			m_category = cat;
		}

		virtual const std::string& GetDescription() const
		{
			std::lock_guard<SpinLock> lock(m_lock);
			return m_description;
		}

		virtual const Currency& GetDomesticCurrency() const
		{
			return m_currency;
		}

		virtual const Country& GetCountry() const
		{
			return m_country;
		}

		virtual const Exchange& GetExchange() const
		{
			throw std::logic_error("not applicable for bonds");
		}

		virtual DayCount::DayCountType GetDayCount() const
		{
			return m_dayCount;
		}

		virtual void SetDayCount(const DayCount::DayCountType& count)
		{
			m_dayCount = count;
		}

		virtual double GetDivYield() const
		{
			throw std::logic_error("Invalid call");
		}

		void SetSymbol(const std::string& sym)
		{
			std::lock_guard<SpinLock> lock(m_lock);
			m_symbol = sym;
		}

		void SetDomesticCurrency(const Currency& cur)
		{
			m_currency = cur;
		}

		void SetDescription(const std::string& des)
		{
			std::lock_guard<SpinLock> lock(m_lock);
			m_description = des;
		}
		
		void SetCountry(const Country& cntry)
		{
			m_country = cntry;
		}

		virtual double GetFaceValue() const
		{
			return m_faceValue;
		}

		virtual void SetFaceValue(double val)
		{
			m_faceValue = val;
		}

	private:

		/// Name(TYPEID, std::hash<std::string>()(symbol))
		/// Key[0] => "symbol"
		Name m_name;

		std::string m_symbol;

		Currency m_currency;

		CategoryType m_category;

		std::string m_description;
		
		Country m_country;

		std::atomic<double> m_faceValue;

		DayCount::DayCountType m_dayCount;

		mutable SpinLock m_lock;
	};	

} /* namespace derivative */

#endif /* _DERIVATIVE_IZEROCOUPONBOND_H_ */
