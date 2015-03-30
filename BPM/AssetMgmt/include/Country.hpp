/// Copyright (C) Nathan Muruganantha 2013 - 2014

#ifndef _DERIVATIVE_COUNTRY_H_
#define _DERIVATIVE_COUNTRY_H_

#if defined _WIN32
#pragma once
#pragma warning (push)
#pragma warning (disable : 4251)
#endif

#include <memory>
#include <string>
#include <iostream>
#include "IObject.hpp"
#include "ClassType.hpp"
#include "Name.hpp"
#include "Currency.hpp"

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
    class FIN_UTIL_DLL_API Country
	{
      public:
       	  
		enum {TYPEID = CLASS_COUNTRY_TYPE};
		
		/// Constructor for constructing Exemplar asset objects
		Country()
		{}

		Country(const std::string& code, const std::string& countryName, const Currency& currency);

		///destructor
		virtual ~Country();

		/// provide explict assignment  operator
		/// and copy constructor
		Country(const Country& rhs);
        Country& operator=(const Country& rhs);

		/// return iso country code
        const std::string& GetCode() const;

        /// return country Name
        const std::string& GetCountryName() const;
        
		/// return currency used by the exchange
        const Currency& GetCurrency() const;
		
	private:
		
		/// used by copy constructor and assignment operators.
		void Clone(const Country& src);

		/// ISO code of the country
		std::string m_code;

		/// Name of the country
		std::string m_countryName;

		/// Currency used by the country
		Currency m_currency;
					
		friend std::ostream& operator<<(std::ostream& os, const Country& ex);

    };

	/// compares two exchanges
	inline bool operator==(const Country& e1, const Country& e2) 
	{
		return e1.GetCode().compare(e2.GetCode()) == 0;
	}

	inline bool operator!=(const Country& e1, const Country& e2) 
	{
		return !(e1 == e2);
	}	

	inline std::ostream& operator<<(std::ostream& os, const Country& ex)
	{
		os << "Country Name : " << ex.m_code << " Country Name : " << ex.m_countryName;
		return os;
	}
}

/* namespace derivative */
#pragma warning(pop)
#endif /* _DERIVATIVE_COUNTRY_H_ */