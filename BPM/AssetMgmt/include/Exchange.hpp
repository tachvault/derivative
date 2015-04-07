/// Copyright (C) Nathan Muruganantha 2013 - 2014

#ifndef _DERIVATIVE_EXCHANGE_H_
#define _DERIVATIVE_EXCHANGE_H_

#if defined _WIN32
#pragma once
#pragma warning (push)
#pragma warning (disable : 4251)
#endif

#include <memory>
#include <string>
#include <iostream>
#include <iostream>

#include "IObject.hpp"
#include "ClassType.hpp"
#include "Name.hpp"
#include "SpinLock.hpp"
#include "Country.hpp"
 
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
    /// Exchange represents the stock and option
	/// exchanges used in the system. All the exchanges
	/// would be loaded and registered during the system startup
	class FIN_UTIL_DLL_API Exchange
	{
      public:
       	  
		enum {TYPEID = CLASS_EXCHANGE_TYPE};
		
		/// Constructor for constructing Exemplar asset objects
		Exchange()
		{}

		Exchange(const std::string& exchangeName, const Country& country,  \
				 int timeOffSet);

		///destructor
		virtual ~Exchange();

		/// provide explict assignment  operator
		/// and copy constructor
		Exchange(const Exchange& rhs);
        Exchange& operator=(const Exchange& rhs);

        /// return exchange Name
        inline const std::string& GetExchangeName() const;
        
		/// return country used by the exchange
        inline const Country& GetCountry() const;

		/// get time offset
		int GetTimeOffSet()
		{
			std::lock_guard<SpinLock> lock(m_lock);
			return m_timeOffSet;
		}		

	private:
		
		/// used by copy constructor and assignment operators.
		void Clone(const Exchange& src);

		/// Name of the echange
		std::string m_exchangeName;

		/// Country used by the echange
		Country m_country;

		/// time offset from GMT
		int m_timeOffSet;

		mutable SpinLock m_lock;
			
		friend std::ostream& operator<<(std::ostream& os, const Exchange& ex);

    };

	/// compares two exchanges
	inline bool operator==(const Exchange& e1, const Exchange& e2) 
	{
		return e1.GetExchangeName().compare(e2.GetExchangeName()) == 0;
	}

	inline bool operator!=(const Exchange& e1, const Exchange& e2) 
	{
		return !(e1 == e2);
	}

	inline std::ostream& operator<<(std::ostream& os, const Exchange& ex)
	{
		os << "Exchange Name : " << ex.m_exchangeName;
		return os;
	}
}

/* namespace derivative */
#pragma warning(pop)
#endif /* _DERIVATIVE_EXCHANGE_H_ */