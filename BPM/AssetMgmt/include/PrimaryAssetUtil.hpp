/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_PRIMARYASSETUTIL_H_
#define _DERIVATIVE_PRIMARYASSETUTIL_H_

#include <memory>
#include "Global.hpp"
#include "Maturity.hpp"
#include "IRCurve.hpp"
#include "EntityMgrUtil.hpp"

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
	class IDailyEquityOptionValue;
	class IFuturesValue;

	namespace PrimaryUtil
	{
		template <typename T>
		std::shared_ptr<T> ConstructEntity(const Name& nm)
		{
			/// First get the T exemplar object from the registry
			/// Get the EntityManager instance
			EntityManager& entMgr = EntityManager::getInstance();

			/// get the concrete types for the given alias name
			std::vector<Name> names;
			entMgr.findAlias(nm, names);
			if (names.empty())
			{
				LOG(ERROR) << " No concrete name found for " << nm << endl;
				throw DataSourceException("type T not bound with correct alias");
			}

			Name entityName((*names.begin()).GetGrpId(), nm.GetObjId());

			/// copy the keys from Alias Name to concrete name
			entityName.SetKeys(nm.GetKeyMap());

			/// get the examplar object for the IRValue
			/// Exemplar objects should be initialized
			/// during global initialization time.
			std::shared_ptr<IObject> exemplar = entMgr.findObject(Name(entityName.GetGrpId()));

			/// Now we have the exampler object. 
			/// Make the exemplar T to construct T for the given name		
			std::shared_ptr<T> obj = dynamic_pointer_cast<T>(dynamic_pointer_cast<IMake>(exemplar)->Make(entityName));

			return obj;
		};

		template <typename derived, typename parent>
		void FindOptionValues(const string& symbol, const dd::date& tdate, int maturity, std::vector<std::shared_ptr<parent> > & options, double strike = 0)
		{
			/// Get option data for apple with trade date as today
			dd::date maturityDate = tdate + dd::date_duration(maturity);
			derived::OptionType opt = derived::OptionType::VANILLA_CALL;
			Name nm = derived::ConstructName(symbol, tdate, opt, maturityDate, strike);

			std::vector<std::shared_ptr<IObject> > objects;
			EntityMgrUtil::findObjects(nm, objects);
			if (objects.empty()) throw std::domain_error("No option data found");
			EntityMgrUtil::registerObjects(objects);			
			for (std::shared_ptr<IObject> obj : objects)
			{
				std::shared_ptr<parent> option = dynamic_pointer_cast<parent>(obj);
				options.push_back(option);
			}
		}

		template <typename derived, typename parent>
		void FindOptionValues(const string& symbol, const dd::date& tdate, const dd::date& maturityDate, std::vector<std::shared_ptr<parent> > & options, double strike = 0)
		{
			/// Get option data for apple with trade date as today
			derived::OptionType opt = derived::OptionType::VANILLA_CALL;
			Name nm = derived::ConstructName(symbol, tdate, opt, maturityDate, strike);

			std::vector<std::shared_ptr<IObject> > objects;
			EntityMgrUtil::findObjects(nm, objects);
			for (std::shared_ptr<IObject> obj : objects)
			{
				std::shared_ptr<parent> option = dynamic_pointer_cast<parent>(obj);
				options.push_back(option);
			}
		}

		PRIMARYASSET_EXT_API inline double getSimpleRateToDF(double rate, double tenor, int frequency);

		PRIMARYASSET_EXT_API inline double getDFToSimpleRate(double rate, double tenor, int frequency);

		PRIMARYASSET_EXT_API double getCompoundRateToDF(double rate, double tenor);

		PRIMARYASSET_EXT_API double getDFToCompoundRate(double df, double tenor);

		PRIMARYASSET_EXT_API std::shared_ptr<IStockValue> getStockValue(const std::string& symbol);

		PRIMARYASSET_EXT_API std::shared_ptr<IFuturesValue> getFuturesValue(const std::string& symbol, const dd::date& tdate, const dd::date& ddate);

		PRIMARYASSET_EXT_API  std::shared_ptr<IExchangeRateValue> getExchangeRateValue(const std::string& domestic, const std::string& foreign);

		PRIMARYASSET_EXT_API std::shared_ptr<IIRValue> FindInterestRate(const std::string& cntry, Maturity::MaturityType& maturity, dd::date& issue);

		PRIMARYASSET_EXT_API double FindInterestRate(const std::string& cntry, double tenor, IRCurve::DataSourceType src);
	}
}

/* namespace derivative */

#endif /* _DERIVATIVE_PRIMARYASSETUTIL_H_ */