/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/


#include "Money.hpp"
#include "DException.hpp"
#include "EntityMgrUtil.hpp"
#include "IExchangeRateValue.hpp"
#include "IDataSource.hpp"

namespace derivative 
{
	Money Money::Convert(const Currency& target)  const
	{
		if (GetCurrency() != target)
		{
			/// Get today's exchange rate using EntityUtil			
			Name exName(IExchangeRateValue::TYPEID, std::hash<std::string>()(m_currency.GetCode() \
				+ target.GetCode() + dd::to_simple_string(dd::day_clock::local_day())));

			std::shared_ptr<IObject> obj = EntityMgrUtil::findObject(exName, YAHOO);
			std::shared_ptr<IExchangeRateValue> rateVal = dynamic_pointer_cast<IExchangeRateValue>(obj);

			Money money(target, m_value*rateVal->GetTradePrice());

			return money;
		}
		return *this;
	}
	
	// inline definitions

	inline Money::Money()
		: m_value(0.0), m_currency() {}

	inline Money::Money(const Currency& currency, double value)
		: m_value(value), m_currency(currency) 
	{}

	inline Money::Money(double value, const Currency& currency)
		: m_value(value), m_currency(currency) 
	{}

	inline const Currency& Money::GetCurrency() const 
	{
		return m_currency;
	}

	inline double Money::value() const 
	{
		return m_value;
	}

	inline Money Money::operator+() const 
	{
		return *this;
	}

	inline Money Money::operator-() const 
	{
		return Money(-m_value, m_currency);
	}

	inline Money& Money::operator*=(double x) 
	{
		m_value *= x;
		return *this;
	}

	inline Money& Money::operator/=(double x) 
	{
		m_value /= x;
		return *this;
	}


	inline Money operator+(const Money& m1, const Money& m2) 
	{
		Money tmp(m2.Convert(m1.GetCurrency()));
		tmp.m_value = m1.m_value + tmp.m_value;
		return tmp;
	}

	inline Money operator-(const Money& m1, const Money& m2) 
	{
		Money tmp = m2.Convert(m1.GetCurrency());		
		tmp.m_value = m1.m_value - tmp.m_value;
		return tmp;
	}

	inline Money operator*(const Money& m, double x) {
		Money tmp = m;
		tmp *= x;
		return tmp;
	}

	inline Money operator*(double x, const Money& m) {
		return m*x;
	}

	inline Money operator/(const Money& m, double x) {
		Money tmp = m;
		tmp /= x;
		return tmp;
	}

	inline bool operator!=(const Money& m1, const Money& m2) {
		return !(m1 == m2);
	}

	inline bool operator>(const Money& m1, const Money& m2) {
		return m2 < m1;
	}

	inline bool operator>=(const Money& m1, const Money& m2) {
		return m2 <= m1;
	}

	inline Money operator*(double value, const Currency& c) {
		return Money(value,c);
	}

	inline Money operator*(const Currency& c, double value) {
		return Money(value,c);
	}

	Money& Money::operator+=(const Money& m) 
	{
		if (m_currency == m.m_currency)
		{
			m_value += m.m_value;
		}
		else
		{
			Money tmp = m.Convert(GetCurrency());
			m_value += tmp.m_value;
		}		
		return *this;
	}

	Money& Money::operator-=(const Money& m) 
	{
		if (m_currency == m.m_currency)
		{
			m_value -= m.m_value;
		}
		else
		{
			Money tmp = m.Convert(GetCurrency());
			m_value -= tmp.m_value;
		}		
		return *this;
	}

	double operator/(const Money& m1, const Money& m2) 
	{
		if (m1.GetCurrency() == m2.GetCurrency())
		{
			return m1.value()/m2.value();
		}
		else 
		{
			Money tmp = m2.Convert(m1.GetCurrency());
			return m1.value()/tmp.value();
		} 
	}

	bool operator==(const Money& m1, const Money& m2) 
	{
		if (m1.GetCurrency() == m2.GetCurrency()) 
		{
			return m1.value() == m2.value();
		}
		else
	    {
			Money tmp = m2.Convert(m1.GetCurrency());
			return m1.value() == tmp.value();
		} 
	}

	bool operator<(const Money& m1, const Money& m2)
	{
		if (m1.GetCurrency() == m2.GetCurrency()) 
		{
			return m1.value() < m2.value();
		} 
		else
		{
			Money tmp = m2.Convert(m1.GetCurrency());
			return m1 < tmp;
		} 
	}

	bool operator<=(const Money& m1, const Money& m2)
	{
		if (m1.GetCurrency() == m2.GetCurrency()) 
		{
			return m1.value() <= m2.value();
		} 
		else
		{
			Money tmp = m2.Convert(m1.GetCurrency());
			return m1 <= tmp;
		} 
	}

	std::ostream& operator<<(std::ostream& out, const Money& m) {
		return out;
	}
}
