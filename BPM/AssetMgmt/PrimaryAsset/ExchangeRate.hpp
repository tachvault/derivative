/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_EXCHANGERATE_H_
#define _DERIVATIVE_EXCHANGERATE_H_

#include <string>
#include <locale>
#include <functional>
#include <algorithm>
#include <exception>

#include "IMake.hpp"
#include "IExchangeRate.hpp"

#include "Currency.hpp"

namespace derivative
{
	/// ExchangeRate implements the IExchangeRate interface.
	class ExchangeRate : virtual public IExchangeRate,
		virtual public IMake
	{
	public:

		enum {TYPEID = CLASS_EXCHANGERATE_TYPE};

		/// Constructor with Exemplar for the Creator ExchangeRate object
		ExchangeRate (const Exemplar &ex);

		/// construct ExchangeRate from a given Name
		/// used in IMake::Make(..)
		ExchangeRate (const Name& nm);

		/// constructor.
		ExchangeRate(const Currency& domestic, const Currency& foreign);

		virtual ~ExchangeRate();

		virtual std::shared_ptr<IMake> Make (const Name &nm);

		virtual std::shared_ptr<IMake> Make (const Name &nm, const std::deque<boost::any>& agrs);

		const Name& GetName()
		{
			return m_name;
		}

		const std::string& GetSymbol() const
		{
			throw std::logic_error("Call not supported");
		}

		virtual double GetImpliedVol() const
		{
			throw std::logic_error("Not supported");
		}

		virtual double GetHistVol() const
		{
			throw std::logic_error("Not supported");
		}

		void SetSymbol(const std::string& sym)
		{
			throw std::logic_error("Call not supported");
		}

		const Currency& GetDomesticCurrency() const
		{
			return m_domesticCurrency;
		}

		const Currency& GetForeignCurrency() const
		{
			return m_foreignCurrency;
		}

		void SetDomesticCurrency(const Currency& cur)
		{
			m_domesticCurrency = cur;
		}

		void SetForeignCurrency(const Currency& cur)
		{
			m_foreignCurrency = cur;
		}
			
	private:

		/// Name(TYPEID, std::hash<std::string>()(domestic.GetCode() + foreign.GetCode()))
		/// Key[0] => domestic - > "domestic currency code"
		/// key[1] => foreign -> "foreign currency code"
		Name m_name;

		Currency m_domesticCurrency;

		Currency m_foreignCurrency;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_EXCHANGERATE_H_ */
