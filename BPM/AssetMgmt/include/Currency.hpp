/// Copyright (C) Nathan Muruganantha 2013 - 2014

#ifndef _DERIVATIVE_CURRENCY_H_
#define _DERIVATIVE_CURRENCY_H_

#if defined _WIN32
#pragma once
#pragma warning (push)
#pragma warning (disable : 4251)
#endif

#include <memory>
#include <string>
#include <iostream>
#include "ClassType.hpp"
#include "IObject.hpp"
#include "Name.hpp"

#if defined _WIN32 || defined __CYGWIN__
  #ifdef FINUTILITY_EXPORTS
    #ifdef __GNUC__
      #define FIN_UTIL_DLL_API __attribute__ ((dllexport))
    #else
      #define FIN_UTIL_DLL_API __declspec(dllexport)
    #endif
  #else
    #ifdef __GNUC__
      #define FIN_UTIL_DLL_API __attribute__ ((dllimport))
    #else
      #define FIN_UTIL_DLL_API __declspec(dllimport)
    #endif
  #endif
  #define FIN_UTIL_DLL_LOCAL
#else
  #if __GNUC__ >= 4
    #define FIN_UTIL_DLL_API __attribute__ ((visibility ("default")))
    #define FIN_UTIL_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define FIN_UTIL_DLL_API
    #define FIN_UTIL_DLL_LOCAL
  #endif
#endif
 

namespace derivative
{
	/// Model Currency. Currency is not a named type
	/// It is copyable and assignable
	class FIN_UTIL_DLL_API Currency
	{
	public:

		enum {TYPEID = CLASS_CURRENCY_TYPE};

		Currency()
		{}
		
		Currency(const std::string& currName, const std::string& code,  \
			ushort numbericCode,  const std::string& symbol, \
			const std::string& fractionSymbol, ushort fractionUnits);

		/// destructor to unbind this object
		virtual ~Currency();

		/// provide explict assignment  operator
		/// and copy constructor
		Currency(const Currency& rhs);
        Currency& operator=(const Currency& rhs);

		/// currency name, e.g, "U.S. Dollar"
		const std::string& GetCurrName() const;

		/// ISO 4217 three-letter code, e.g, "USD"
		const std::string& GetCode() const;

		/// ISO 4217 numeric code, e.g, "840"
		ushort GetNumericCode() const;

		/// symbol, e.g, "$"
		const std::string& GetSymbol() const;

		/// fraction symbol, e.g, "¢"
		const std::string& GetFractionSymbol() const;

		/// number of fractionary parts in a unit, e.g, 100
		ushort GetFractionsPerUnit() const;

	private:
		
		/// used by copy constructor and assignment operators.
		void Clone(const Currency& src);

		std::string m_currName;

		std::string m_code;

		ushort m_numbericCode;

		std::string m_symbol;

		std::string m_fractionSymbol;

		ushort m_fractionUnits;

		friend std::ostream& operator<<(std::ostream& os, const Currency& curr);
	};

	/// compares two currencies for their ISO codes
	bool operator==(const Currency&, const Currency&);

	/// compares two currencies for their ISO codes
	bool operator!=(const Currency&, const Currency&);

	/*! \relates Currency */
	std::ostream& operator<<(std::ostream&, const Currency&);

	// inline definitions    
	inline const std::string& Currency::GetCurrName() const
	{
		return m_currName;
	}

	inline const std::string& Currency::GetCode() const 
	{
		return m_code;
	}

	inline ushort Currency::GetNumericCode() const
	{
		return m_numbericCode;
	}

	inline const std::string& Currency::GetSymbol() const
	{
		return m_symbol;
	}

	inline const std::string& Currency::GetFractionSymbol() const
	{
		return m_fractionSymbol;
	}

	inline ushort Currency::GetFractionsPerUnit() const 
	{
		return m_fractionUnits;
	}

	inline bool operator==(const Currency& c1, const Currency& c2) 
	{
		return c1.GetCode().compare(c2.GetCode()) == 0;
	}

	inline bool operator!=(const Currency& c1, const Currency& c2) 
	{
		return !(c1 == c2);
	}

	inline std::ostream& operator<<(std::ostream& os, const Currency& curr)
	{
		os << "Name : " << curr.m_currName << ", Code : " << curr.m_code \
			<< "Numeric Code : " << curr.m_numbericCode << ", Symbol : " << curr.m_symbol \
			<< ", Fraction Symbol : " << curr.m_fractionSymbol;

		return os;
	}
}

/* namespace derivative */
#pragma warning(pop)
#endif /* _DERIVATIVE_CURRENCY_H_ */