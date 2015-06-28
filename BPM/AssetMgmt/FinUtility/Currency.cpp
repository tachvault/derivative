/*
Modified and ported: 2013, Nathan Muruganantha. All rights reserved.
*/

#include "currency.hpp"
#include "Name.hpp"
#include "EntityManager.hpp"

namespace derivative 
{
	Currency::Currency(const std::string& currName, const std::string& code,  \
		ushort numbericCode,  const std::string& symbol, \
		const std::string& fractionSymbol, ushort fractionUnits)
		:m_currName(currName), m_code(code), m_numbericCode(numbericCode),  \
		m_symbol(symbol), m_fractionSymbol(fractionSymbol), m_fractionUnits(fractionUnits)
	{		
	}

	Currency::~Currency()
	{
	}

	Currency::Currency(const Currency& rhs)
	{
		Clone(rhs);
	}

    Currency& Currency::operator=(const Currency& rhs)
	{
		/// if rhs this the same object as this then
		/// return reference to this
		if (*this == rhs)
		{
			return *this;
		}

		/// If rhs is different from this
		/// then clone the rhs and reference to this
		Clone(rhs);

		return *this;
	}

	void Currency::Clone(const Currency& src)
	{
		std::lock_guard<SpinLock> lock(m_lock);

		m_currName = src.m_currName;
		m_code = src.m_code;
		m_numbericCode = src.m_numbericCode;
		m_symbol = src.m_symbol;
		m_fractionSymbol = src.m_fractionSymbol;
		m_fractionUnits = src.m_fractionUnits;
	}	
}