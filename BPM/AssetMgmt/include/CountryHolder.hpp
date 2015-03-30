/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#if defined _WIN32
/// disable warnings on 255 char debug symbols
#pragma warning (disable : 4786)
/// disable warnings on extern before template instantiation
#pragma warning (disable : 4231)
#endif


#ifndef _DERIVATIVE_COUNTRYHOLDER_H_
#define _DERIVATIVE_COUNTRYHOLDER_H_

#include <memory>

#include "ClassType.hpp"
#include "Global.hpp"
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
	/// Provides callers with Country objects
	/// This is a Singleton but should not be intended for 
	/// lazy initialization. CountryHolder is initialized
	/// during Loading of this Library.
	class FIN_UTIL_DLL_API CountryHolder
	{

	public:

		enum { TYPEID = CLASS_COUNTRYHOLDER_TYPE};	

		/// default constructor
		CountryHolder();

		/// desstructor
		~CountryHolder();

		/// Provide a named constructor to construct CountryHolder
		static CountryHolder& getInstance();

		/// initialize the CountryHolder
		void Init(ushort source);

		/// Get the currency object given the ISO code
		const Country& GetCountry(const std::string& name);

	private:	

		class Impl;
		std::unique_ptr<Impl> m_Impl;

		bool m_initialized;

		/// disallow copy and assignment
		DISALLOW_COPY_AND_ASSIGN(CountryHolder);

	};

} /* namespace derivative */

#endif /* _DERIVATIVE_CURRENCYDATALOADER_H_ */
