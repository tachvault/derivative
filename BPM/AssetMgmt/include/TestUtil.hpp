/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schl�gl
*/

#ifndef _DERIVATIVE_TESTUTIL_H_
#define _DERIVATIVE_TESTUTIL_H_

#include <memory>
#include "Global.hpp"
#include "Maturity.hpp"

#if defined _WIN32 || defined __CYGWIN__
#ifdef PRIMARYASSET_EXT_EXPORTS
#ifdef __GNUC__
#define PRIMARYASSET_EXT_API __attribute__ ((dllexport))
#else
#define PRIMARYASSET_EXT_API __declspec(dllexport)
#endif
#else
#ifdef __GNUC__
#define PRIMARYASSET_EXT_API __attribute__ ((dllimport))
#else
#define PRIMARYASSET_EXT_API __declspec(dllimport)
#endif
#endif
#define PRIMARYASSET_EXT_LOCAL
#else
#if __GNUC__ >= 4
#define PRIMARYASSET_EXT_API __attribute__ ((visibility ("default")))
#define PRIMARYASSET_EXT_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define PRIMARYASSET_EXT_API
#define PRIMARYASSET_EXT_LOCAL
#endif
#endif

namespace derivative
{
	class IIRValue;
	class IAssetValue;
	class IStockValue;
	class IExchangeRateValue;
	class IDailyOptionValue;

	namespace TestUtil
	{
		std::shared_ptr<IObject> GetNamedObject(const Name& nm);

		std::shared_ptr<IStockValue> getStockValue(const std::string& symbol);

		std::shared_ptr<IExchangeRateValue> getExchangeRateValue(const std::string& domestic, const std::string& foreign);
	}
}

/* namespace derivative */

#endif /* _DERIVATIVE_TESTUTIL_H_ */