/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_EXCHANGEEXT_H_
#define _DERIVATIVE_EXCHANGEEXT_H_

#if defined _WIN32
#pragma once
#pragma warning (push)
#pragma warning (disable : 4251)
#endif

#include <memory>
#include <unordered_map>

#include "Global.hpp"

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
	/// ExchangeExt is a singleton object responsible for maintaining <key, value> pair
	/// for <pair<source, exchange>, exchange ext> 
	class  FIN_UTIL_DLL_API ExchangeExt
	{

	public:	

		enum { TYPEID = CLASS_EXCHANGEEXT_TYPE };
		
		/// Ex: <<YAHOO, "NYSE">, "N">   
		typedef std::map<std::pair<ushort, std::string>,  std::string> ExchangeExtType;

		/// Get the ExchangeExt singleton instance
		static ExchangeExt& getInstance();

		/// get exchange ext for the given ticker exchange for a source
		const std::string& GetExchangeExt(ushort src, const std::string& ext);

		/// constructor
		ExchangeExt();

		~ExchangeExt();

		/// Initialize the map
		void Init(ushort source);

	private:

		class Impl;
		std::unique_ptr<Impl> m_Impl;		

		bool m_initialized;
	};

} /* namespace derivative */

#pragma warning(pop)
#endif /* _DERIVATIVE_EXCHANGEEXT_H_ */
