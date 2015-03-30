/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include <memory>
#include "Name.hpp"
#include "DailyFuturesOptionValueMySQLDAO.hpp"
#include "GroupRegister.hpp"
#include "MySqlConnection.hpp"
#include "EntityManager.hpp"
#include "EntityMgrUtil.hpp"
#include "IDataSource.hpp"
#include "CountryHolder.hpp"
#include "IFutures.hpp"
#include "IDailyFuturesOptionValue.hpp"

namespace derivative
{
	GROUP_REGISTER(DailyFuturesOptionValueMySQLDAO);
	DAO_REGISTER(IDailyFuturesOptionValue, MYSQL, DailyFuturesOptionValueMySQLDAO);

	std::shared_ptr<IMake> DailyFuturesOptionValueMySQLDAO::Make(const Name &nm)
	{
		/// Construct DailyFuturesOptionValueMySQLDAO from given name and register with EntityManager
		std::shared_ptr<DailyFuturesOptionValueMySQLDAO> dao = make_shared<DailyFuturesOptionValueMySQLDAO>(nm);
		EntityMgrUtil::registerObject(nm, dao);
		LOG(INFO) << " DailyFuturesOptionValueMySQLDAO  " << nm << " is constructed and registered with EntityManager" << endl;

		/// return constructed object if no exception is thrown
		return dao;
	}

	std::shared_ptr<IDailyFuturesOptionValue> DailyFuturesOptionValueMySQLDAO::constructEntity(const Name& nm)
	{
		/// First get the DailyFuturesOptionValue exemplar object from the registry
		/// Get the EntityManager instance
		EntityManager& entMgr = EntityManager::getInstance();

		/// get the concrete types for the given alias name
		std::vector<Name> names;
		entMgr.findAlias(nm, names);
		if (names.empty())
		{
			LOG(ERROR) << " No concrete name found for " << nm << endl;
			throw DataSourceException("DailyFuturesOptionValue not bound with correct alias");
		}

		Name entityName((*names.begin()).GetGrpId(), nm.GetObjId());

		/// copy the keys from Alias Name to concrete name
		entityName.SetKeys(nm.GetKeyMap());

		/// get the examplar object for the DailyFuturesOptionValue
		/// Exemplar objects should be initialized
		/// during global initialization time.
		std::shared_ptr<IObject> exemplar = entMgr.findObject(Name(entityName.GetGrpId()));

		/// Now we have the exampler object. 
		/// Make the exemplar DailyFuturesOptionValue to construct DailyFuturesOptionValue for the given name
		std::shared_ptr<IDailyFuturesOptionValue> value;
		try
		{
			value = dynamic_pointer_cast<IDailyFuturesOptionValue>(dynamic_pointer_cast<IMake>(exemplar)->Make(entityName));
		}
		catch (RegistryException& e)
		{
			LOG(WARNING) << " Object already found in registry " << e.what() << endl;
			return value;
		}
		/// we don't want to catch other exceptions here.. Let the client code handle that.

		/// if the option is not in the registry and constructed sucessfully
		/// then we need to find and associate DailyFuturesOption with DailyFuturesOptionValue
		std::string symbol;
		dd::date tradeDate;
		IDailyFuturesOptionValue::OptionType optType;
		dd::date matDate;
		double strike;
		IDailyFuturesOptionValue::GetKeys(nm, symbol, tradeDate, optType, matDate, strike);
		Name DailyFuturesOptionName = IFutures::ConstructName(symbol);
		try
		{
			/// get the DailyFuturesOption from registry or from data source
			/// set the DailyFuturesOption object with DailyFuturesOptionValue
			value->SetOption(dynamic_pointer_cast<IFutures>(EntityMgrUtil::findObject(DailyFuturesOptionName)));
		}
		catch (RegistryException& e)
		{
			LOG(WARNING) << " RegistryException thrown " << e.what() << endl;
			throw e;
		}
		catch (...)
		{
			LOG(WARNING) << " Unknown Exception thrown " << endl;
			throw;
		}
		return value;
	}

	void DailyFuturesOptionValueMySQLDAO::find(const Name& nm, std::vector<std::shared_ptr<IObject> > & entities)
	{
		LOG(INFO) << "DailyFuturesOptionValueMySQLDAO::find(..) is called for " << nm << endl;

		/// If we are here means, DailyFuturesOptionValue object with trade date in nm are
		/// not in the registry. That's we should fetch the 
		/// object from the data data source.
		try
		{
			if (!m_con)
			{
				MySQLConnection::InitConnection(m_driver, m_con);
			}
		}
		catch (sql::SQLException &e)
		{
			LOG(ERROR) << " MySQL throws exception while connecting to the database " << nm << endl;
			LOG(ERROR) << "# ERR: " << e.what();
			throw e;
		}

		/// Populate the option value specific attributes
		findDailyFuturesOptionValue(nm, entities);
	}

	void DailyFuturesOptionValueMySQLDAO::findDailyFuturesOptionValue(const Name& nm, \
		std::vector<std::shared_ptr<IObject> > & entities)
	{
		/// These keys are valid attributes given in Name "nm".
		std::string symbol;
		IDailyFuturesOptionValue::OptionType optType = IDailyFuturesOptionValue::OPTION_TYPE_UNKNOWN;
		dd::date tradedate;
		dd::date matDate;
		double strike = 0;
		IDailyFuturesOptionValue::GetKeys(nm, symbol, tradedate, optType, matDate, strike);

		try
		{
			EntityManager& entMgr = EntityManager::getInstance();

			std::auto_ptr<sql::Statement> stmt(m_con->createStatement());
			auto ddate = dd::to_iso_extended_string(matDate);
			string query("CALL get_DailyFuturesOptionValue_2(\"" + symbol + "\",\"" + ddate + "\")");
			stmt->execute(query);
			do
			{
				std::unique_ptr<sql::ResultSet> res;
				res.reset(stmt->getResultSet());
				/// `option_id`,tradedate`, contract_date`, option_maturitydate`, opt_type`,
				/// strike`, open_price`, high`,low`, last_price`,settle`,volume`, open_int`
				while (res->next())
				{
					auto strikeVal = res->getDouble("strike");
					auto tdateStr = res->getString("tradedate").asStdString();
					auto tradedate = dd::from_simple_string(tdateStr);

					auto optType = atoi(res->getString("opt_type").asStdString().c_str());
					auto matDateStr = res->getString("contract_date").asStdString();
					auto matDate = dd::from_simple_string(matDateStr);
					Name optName = IDailyFuturesOptionValue::ConstructName(symbol, tradedate, static_cast<IDailyFuturesOptionValue::OptionType>(optType), matDate, strikeVal);

					/// try getting if from EntityManager. If not in entitity manager
					/// then fetch from database
					/// get the concrete Name for the given alias Name
					unsigned int concreteType = entMgr.findAlias(optName);
					Name concreteName = optName;
					if (concreteType > 0)
					{
						concreteName = Name(concreteType, optName.GetObjId());
					}

					std::shared_ptr<IObject> obj = entMgr.findObject(concreteName);
					if (obj)
					{
						entities.push_back(obj);
					}
					else
					{
						/// option not in registry.
						/// get all other data from database
						auto rettradePrice = res->getDouble("last_price");
						auto high = res->getDouble("high");
						auto low = res->getDouble("low");
						auto settle = res->getDouble("settle");
						auto retvol = res->getInt("volume");
						auto retopenInt = res->getInt("open_int");

						/// construct the Option object.
						std::shared_ptr<IDailyFuturesOptionValue> option = constructEntity(optName);
						option->SetTradeDate(tradedate);

						/// contract date and maturity date are the same in the CME settlement files.
						/// if there is an option that differs we will handle it later
						option->SetMaturityDate(matDate);
						option->SetDeliveryDate(matDate);
						option->SetStrikePrice(strikeVal);
						option->SetOptionType(static_cast<IDailyFuturesOptionValue::OptionType>(optType));

						/// set settlement price as the trade price since the last price is always zero in the
						/// settlement file.
						option->SetTradePrice(settle);
						option->SetHighPrice(high);
						option->SetLowPrice(low);
						option->SetSettledPrice(settle);
						option->SetVolume(retvol);
						option->SetOpenInterest(retopenInt);

						LOG(INFO) << " DailyFuturesOptionValue " << symbol << " with trade date " << tradedate << " constructed with " \
							<< "maturity date" << matDate \
							<< "strike price" << strike \
							<< "settled price" << settle \
							<< "trade price date" << rettradePrice << endl;

						entities.push_back(option);
					}
				}
			} while (stmt->getMoreResults());
		}
		catch (sql::SQLException &e)
		{
			LOG(ERROR) << " No data found in source for " << nm << endl;
			LOG(ERROR) << "# ERR: " << e.what();
			throw e;
		}
	};
} /* namespace derivative */