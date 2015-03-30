/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#if defined _WIN32
/// disable warnings on 255 char debug symbols
#pragma warning (disable : 4786)
/// disable warnings on extern before template instantiation
#pragma warning (disable : 4231)
#endif


#ifndef _DERIVATIVE_CURRENCYHOLDER_H_
#define _DERIVATIVE_CURRENCYHOLDER_H_

#include <memory>

#include "ClassType.hpp"
#include "Global.hpp"
#include "Currency.hpp"

#if defined FINUTILITY_EXPORTS
#define FIN_UTIL_DLL_API __declspec(dllexport)
#else
#define FIN_UTIL_DLL_API __declspec(dllimport)
#endif

namespace derivative
{
	/// Provides callers with Currency objects
	/// This is a Singleton but should not be intended for 
	/// lazy initialization. CurrencyHolder is initialized
	/// during Loading of this Library.
	class FIN_UTIL_DLL_API CurrencyHolder
	{
	public:

		enum { TYPEID = CLASS_CURRENCYLOADER};

		/// Constructor
		CurrencyHolder();

		/// destructor
		~CurrencyHolder();
				
		/// Provide a named constructor to construct CurrencyHolder
		static CurrencyHolder& getInstance();

		/// initialize the CurrencyHolder
		void Init(ushort source);

		/// Get the currency object given the ISO code
		const Currency& GetCurrency(const std::string& code);

	private:

		class Impl;
		std::unique_ptr<Impl> m_Impl;

		bool m_initialized;

		/// disallow copy and assignment
		DISALLOW_COPY_AND_ASSIGN(CurrencyHolder);

	};

} /* namespace derivative */

#endif /* _DERIVATIVE_CURRENCYDATALOADER_H_ */
