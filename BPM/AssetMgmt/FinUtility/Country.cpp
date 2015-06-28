/*
Modified and ported: 2013, Nathan Muruganantha. All rights reserved.
*/

#include "Country.hpp"

namespace derivative 
{
	Country::Country(const std::string& code, const std::string& countryName, const Currency& curr)
				 :m_code(code),
				 m_countryName(countryName),
				 m_currency(curr)
	{		
	}

	Country::Country(const Country& rhs)
	{
		Clone(rhs);
	}

    Country& Country::operator=(const Country& rhs)
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
	
	const std::string& Country::GetCode() const
	{
		std::lock_guard<SpinLock> lock(m_lock);
		return m_code;
	}

	const std::string& Country::GetCountryName() const
	{
		std::lock_guard<SpinLock> lock(m_lock);
		return m_countryName;
	}
        
	const Currency& Country::GetCurrency() const
	{
		std::lock_guard<SpinLock> lock(m_lock);
		return m_currency;
	}

	void Country::Clone(const Country& src)
	{
		std::lock_guard<SpinLock> lock(m_lock);
		m_code = src.m_code;
		m_countryName = src.m_countryName;
		m_currency = src.m_currency;
	}

	Country::~Country()
	{		
	}	
}