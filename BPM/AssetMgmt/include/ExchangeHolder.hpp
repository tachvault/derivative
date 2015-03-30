/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#if defined _WIN32
  /// disable warnings on 255 char debug symbols
  #pragma warning (disable : 4786)
  /// disable warnings on extern before template instantiation
  #pragma warning (disable : 4231)
#endif


#ifndef _DERIVATIVE_EXCHANGEHOLDER_H_
#define _DERIVATIVE_EXCHANGEHOLDER_H_

#include <memory>

#include "ClassType.hpp"
#include "Global.hpp"
#include "Exchange.hpp"

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
	/// Provides callers with Exchange objects
	/// This is a Singleton but should not be intended for 
	/// lazy initialization. ExchangeHolder is initialized
	/// during Loading of this Library.
	class FIN_UTIL_DLL_API ExchangeHolder
	{

	public:

		enum { TYPEID = CLASS_EXCHANGEHOLDER_TYPE};	

		/// default constructor
		ExchangeHolder();

		/// desstructor
		~ExchangeHolder();

		/// Provide a named constructor to construct ExchangeHolder
		static ExchangeHolder& getInstance();

		/// initialize the ExchangeHolder
		void Init(ushort source);

		/// Get the currency object given the ISO code
		const Exchange& GetExchange(const std::string& name);

	private:	

		class Impl;
		std::unique_ptr<Impl> m_Impl;

		bool m_initialized;

		/// disallow copy and assignment
		DISALLOW_COPY_AND_ASSIGN(ExchangeHolder);

	};

} /* namespace derivative */

#endif /* _DERIVATIVE_CURRENCYDATALOADER_H_ */
