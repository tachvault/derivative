/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_IR_H_
#define _DERIVATIVE_IR_H_

#include <string>
#include <locale>
#include <functional>
#include <algorithm>

#include "IMake.hpp"
#include "IIR.hpp"
#include "Country.hpp"

namespace derivative
{
	/// IR (Interest Rate) implements the IIR interface.
	class IR : virtual public IIR,
		virtual public IMake
	{
	public:

		enum {TYPEID = CLASS_IR_TYPE};

		/// Constructor with Exemplar for the Creator IR object
		IR (const Exemplar &ex);

		/// construct IR from a given Name
		/// used in IMake::Make(..)
		IR (const Name& nm);

		/// constructor.
		IR(const Country& domestic, const Maturity::MaturityType type);

		virtual ~IR();

		virtual std::shared_ptr<IMake> Make (const Name &nm);

		virtual std::shared_ptr<IMake> Make (const Name &nm, const std::deque<boost::any>& agrs);

		const Name& GetName()
		{
			std::lock_guard<SpinLock> lock(m_lock);
			return m_name;
		}

		const Country& GetCountry() const
		{
			return m_country;
		}

		Maturity::MaturityType GetMaturityType() const
		{
			std::lock_guard<SpinLock> lock(m_lock);
			return m_maturityType;
		}

		/// return type of rate. Influenced by Xignite rate API.
		const std::string& GetRateType() const
		{
			std::lock_guard<SpinLock> lock(m_lock);
			return m_type;
		}

		void SetCountry(const Country& cntry)
		{
			m_country = cntry;
		}
		
		void SetMaturityType(Maturity::MaturityType type)
		{
			std::lock_guard<SpinLock> lock(m_lock);
			m_maturityType = type;
		}

	    void SetRateType(const std::string& type)
		{
			std::lock_guard<SpinLock> lock(m_lock);
			m_type = type;
		}
							
	private:

		/// Name(TYPEID, std::hash<std::string>()(m_country.Code + m_maturityType))		
		/// Key[0] => "country code"
		/// Key[1] => "maturity type"
		Name m_name;

		Country m_country;

		Maturity::MaturityType m_maturityType;

		std::string m_type;

		mutable SpinLock m_lock;

	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_IR_H_ */
