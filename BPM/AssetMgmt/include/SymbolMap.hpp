/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_SYMBOLMAP_H_
#define _DERIVATIVE_SYMBOLMAP_H_

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
	/// SymbolMap is a singleton object responsible for
	/// maintaining <key, value> pair for <pair<source, symbol>, alias symbol> 
	
	/// For ex: we will maintain standard symbol based on reuter symbol (Apple - AAPL.OQ)
	/// in our database. If a data source use different symbol then we need to have a 
	/// mapping from our symbol to data source symbol. (Yahoo uses AAPL for apple).
	/// It means when communicating with apple, we use AAPL instead of AAPL.OQ.
	/// That is where SymbolMap helps. In apple and yahoo source case, we will main an entry
	/// map<<pair<YAHOO, AAPL.OQ>, AAPL>

	class  FIN_UTIL_DLL_API SymbolMap
	{

	public:	

		typedef std::map<std::pair<ushort, std::string>,  std::string> SymbolMapType;

		/// Get the SymbolMap singleton instance
		static SymbolMap& getInstance();

		/// get symbol for the given ticker symbol for a source
		const std::string& GetSymbolAlias(ushort src, const std::string& symbol);

		/// constructor
		SymbolMap();

		~SymbolMap();

		/// Initialize the map
		void Init(ushort source);

	private:

		class Impl;
		std::unique_ptr<Impl> m_Impl;		

		bool m_initialized;
	};

} /* namespace derivative */

#pragma warning(pop)
#endif /* _DERIVATIVE_SYMBOLMAP_H_ */
