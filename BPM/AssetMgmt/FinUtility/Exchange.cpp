/*
Modified and ported: 2013, Nathan Muruganantha. All rights reserved.
*/

#include "Exchange.hpp"

namespace derivative 
{
	Exchange::Exchange(const std::string& exchangeName, const Country& curr,  \
				 int timeOffSet)
				 :m_exchangeName(exchangeName),
				 m_country(curr),
				 m_timeOffSet(timeOffSet)
	{		
	}

	Exchange::Exchange(const Exchange& rhs)
	{
		Clone(rhs);
	}

    Exchange& Exchange::operator=(const Exchange& rhs)
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

	

	const std::string& Exchange::GetExchangeName() const
	{
		return m_exchangeName;
	}
        
	const Country& Exchange::GetCountry() const
	{
		return m_country;
	}

	void Exchange::Clone(const Exchange& src)
	{
		m_exchangeName = src.m_exchangeName;
		m_country = src.m_country;
		m_timeOffSet = src.m_timeOffSet;
	}

	Exchange::~Exchange()
	{		
	}	
}