/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_LIBOR_H_
#define _DERIVATIVE_LIBOR_H_

#include <string>
#include <locale>
#include <functional>
#include <algorithm>

#include "IMake.hpp"
#include "IIBOR.hpp"

#include "Currency.hpp"

namespace derivative
{
	/// LIBOR implements the IIBOR interface.
	class LIBOR : virtual public IIBOR,
		virtual public IMake
	{
	public:

		enum {TYPEID = CLASS_LIBOR_TYPE};

		/// Constructor with Exemplar for the Creator LIBOR object
		LIBOR (const Exemplar &ex);

		/// construct LIBOR from a given Name
		/// used in IMake::Make(..)
		LIBOR (const Name& nm);

		/// constructor.
		LIBOR(const Currency& domestic, const Maturity::MaturityType type);

		virtual ~LIBOR();

		virtual std::shared_ptr<IMake> Make (const Name &nm);

		virtual std::shared_ptr<IMake> Make (const Name &nm, const std::deque<boost::any>& agrs);

		const Name& GetName()
		{
			return m_name;
		}

		const Currency& GetCurrency() const
		{
			return m_currency;
		}

		Maturity::MaturityType GetMaturityType() const
		{
			return m_maturityType;
		}

		/// return type of rate. Influenced by Xignite rate API.
		const std::string& GetRateType() const
		{
			return m_type;
		}

		void SetCurrency(const Currency& cur)
		{
			m_currency = cur;
		}
		
		void SetMaturityType(Maturity::MaturityType type)
		{
			m_maturityType = type;
		}

		void SetRateType(const std::string& type)
		{
			m_type = type;
		}
							
	private:

		/// Name(TYPEID, std::hash<std::string>()(m_maturityType + m_domesticCurrency.Code))
		/// Key[0] => "maturity type"
		/// Key[1] => "currency code"
		Name m_name;

		Currency m_currency;

		Maturity::MaturityType m_maturityType;
		
		std::string m_type;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_LIBOR_H_ */
