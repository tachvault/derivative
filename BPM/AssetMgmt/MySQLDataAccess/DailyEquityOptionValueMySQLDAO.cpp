/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include <memory>
#include "Name.hpp"
#include "DailyEquityOptionValueMySQLDAO.hpp"
#include "GroupRegister.hpp"
#include "MySqlConnection.hpp"
#include "EntityManager.hpp"
#include "EntityMgrUtil.hpp"
#include "IDataSource.hpp"
#include "CountryHolder.hpp"
#include "IStock.hpp"
#include "IDailyEquityOptionValue.hpp"

namespace derivative
{
	GROUP_REGISTER(DailyEquityOptionValueMySQLDAO);
	DAO_REGISTER(IDailyEquityOptionValue, MYSQL, DailyEquityOptionValueMySQLDAO);

	std::shared_ptr<IMake> DailyEquityOptionValueMySQLDAO::Make(const Name &nm)
	{
		/// Construct DailyEquityOptionValueMySQLDAO from given name and register with EntityManager
		std::shared_ptr<DailyEquityOptionValueMySQLDAO> dao = make_shared<DailyEquityOptionValueMySQLDAO>(nm);
		EntityMgrUtil::registerObject(nm, dao);
		LOG(INFO) << " DailyEquityOptionValueMySQLDAO  " << nm << " is constructed and registered with EntityManager" << endl;

		/// return constructed object if no exception is thrown
		return dao;
	}

	std::shared_ptr<IDailyEquityOptionValue> DailyEquityOptionValueMySQLDAO::constructEntity(const Name& nm)
	{
		/// First get the DailyEquityOptionValue exemplar object from the registry
		/// Get the EntityManager instance
		EntityManager& entMgr = EntityManager::getInstance();

		/// get the concrete types for the given alias name
		std::vector<Name> names;
		entMgr.findAlias(nm, names);
		if (names.empty())
		{
			LOG(ERROR) << " No concrete name found for " << nm << endl;
			throw DataSourceException("DailyEquityOptionValue not bound with correct alias");
		}

		Name entityName((*names.begin()).GetGrpId(), nm.GetObjId());

		/// copy the keys from Alias Name to concrete name
		entityName.SetKeys(nm.GetKeyMap());

		/// get the examplar object for the DailyEquityOptionValue
		/// Exemplar objects should be initialized
		/// during global initialization time.
		std::shared_ptr<IObject> exemplar = entMgr.findObject(Name(entityName.GetGrpId()));

		/// Now we have the exampler object. 
		/// Make the exemplar DailyEquityOptionValue to construct DailyEquityOptionValue for the given name
		std::shared_ptr<IDailyEquityOptionValue> value;
		try
		{
			value = dynamic_pointer_cast<IDailyEquityOptionValue>(dynamic_pointer_cast<IMake>(exemplar)->Make(entityName));
		}
		catch (RegistryException& e)
		{
			LOG(WARNING) << " Object already found in registry " << e.what() << endl;
			return value;
		}
		/// we don't want to catch other exceptions here.. Let the client code handle that.

		/// if the option is not in the registry and constructed sucessfully
		/// then we need to find and associate DailyEquityOption with DailyEquityOptionValue
		std::string symbol;
		dd::date tradeDate;
		IDailyEquityOptionValue::OptionType optType;
		dd::date matDate;
		double strike;
		IDailyEquityOptionValue::GetKeys(nm, symbol, tradeDate, optType, matDate, strike);
		Name DailyEquityOptionName = IStock::ConstructName(symbol);
		try
		{
			/// get the DailyEquityOption from registry or from data source
			/// set the DailyEquityOption object with DailyEquityOptionValue
			value->SetOption(dynamic_pointer_cast<IStock>(EntityMgrUtil::findObject(DailyEquityOptionName)));
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

	const std::shared_ptr<IObject> DailyEquityOptionValueMySQLDAO::find(const Name& nm)
	{
		LOG(INFO) << "DailyEquityOptionValueMySQLDAO::find(..) is called for " << nm << endl;

		/// Make a connection to the MySQL if not connected
		/// log the SQLException error if thrown
		/// and retrow the exception so that the client
		/// can catch and take required action
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

		/// Populate the stock specific attributes
		return findOption(nm);
	}

	void DailyEquityOptionValueMySQLDAO::find(const Name& nm, std::vector<std::shared_ptr<IObject> > & entities)
	{
		LOG(INFO) << "DailyEquityOptionValueMySQLDAO::find(..) is called for " << nm << endl;

		/// If we are here means, DailyEquityOptionValue object with trade date in nm are
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
		findDailyEquityOptionValue(nm, entities);
	}

	std::shared_ptr<IObject> DailyEquityOptionValueMySQLDAO::findOption(const Name& nm)
	{
		try
		{
			std::unique_ptr<sql::PreparedStatement> pstmt;
			std::unique_ptr<sql::ResultSet> res_2;

			/// Get the key attributes of the given name
			std::string symbol;
			IDailyEquityOptionValue::OptionType optType;
			dd::date matDate;
			dd::date tradeDate;
			double strike = 0;
			IDailyEquityOptionValue::GetKeys(nm, symbol, tradeDate, optType, matDate, strike);

			std::string tdateStr = dd::to_iso_extended_string(tradeDate);
			sql::SQLString tdate(tdateStr.c_str());
			std::string mdateStr = dd::to_iso_extended_string(matDate);
			sql::SQLString mdate(mdateStr.c_str());

			pstmt.reset(m_con->prepareStatement("CALL get_DailyEquityOptionValue(?, ?, ?, ?, ?, @out_tradedPrice, @out_askPrice, @out_bidPrice, @out_vol, @out_openInt)"));

			pstmt->setString(1, symbol.c_str());
			pstmt->setDateTime(2, tdate);
			pstmt->setDateTime(3, mdate);
			pstmt->setInt(4, static_cast<int>(optType));
			pstmt->setDouble(5, strike);
			pstmt->execute();
			cout << " function executed " << endl;

			pstmt.reset(m_con->prepareStatement("SELECT @out_tradedPrice AS tPrice, @out_askPrice AS aPrice, @out_bidPrice AS bPrice, @out_vol AS vol, @out_openInt AS openInt"));
			res_2.reset(pstmt->executeQuery());
			while (res_2->next())
			{
				/// set the price, volume and Open interest attributes
				auto rettradePrice = res_2->getDouble("tPrice");
				auto retaskPrice = res_2->getDouble("aPrice");
				auto retbidPrice = res_2->getDouble("bPrice");
				auto retvol = res_2->getDouble("vol");
				auto retopenInt = res_2->getDouble("openInt");

				/// construct the Option object.
				Name optName = IDailyEquityOptionValue::ConstructName(symbol, tradeDate, static_cast<IDailyEquityOptionValue::OptionType>(optType), matDate, strike);
				std::shared_ptr<IDailyEquityOptionValue> option = constructEntity(optName);

				option->SetTradeDate(tradeDate);
				option->SetMaturityDate(matDate);
				option->SetStrikePrice(strike);
				option->SetOptionType(optType);

				option->SetTradePrice(rettradePrice);
				option->SetAskingPrice(retaskPrice);
				option->SetBidPrice(retbidPrice);
				option->SetVolume(retvol);
				option->SetOpenInterest(retopenInt);

				LOG(INFO) << " DailyEquityOptionValue " << symbol << " with trade date " << tradeDate << " constructed with " \
					<< "maturity date" << matDate \
					<< "strike price" << strike \
					<< "trade price date" << rettradePrice << endl;

				return option;
			}
		}
		catch (sql::SQLException &e)
		{
			cout << " No option data found in source for " << m_name << endl;
			cout << "# ERR: " << e.what();
			throw e;
		}
	}

	void DailyEquityOptionValueMySQLDAO::findDailyEquityOptionValue(const Name& nm, \
		std::vector<std::shared_ptr<IObject> > & entities)
	{
		/// These keys are valid attributes given in Name "nm".
		std::string symbol;
		IDailyEquityOptionValue::OptionType optType;
		dd::date tdate;
		dd::date matDate;

		/// These two keys are dummies. The function should retrive
		/// all records with different strike prices.			
		double strike = 0;
		IDailyEquityOptionValue::GetKeys(nm, symbol, tdate, optType, matDate, strike);



		EntityManager& entMgr = EntityManager::getInstance();

		try
		{
			EntityManager& entMgr = EntityManager::getInstance();

			std::auto_ptr<sql::Statement> stmt(m_con->createStatement());
			auto ddate = dd::to_iso_extended_string(matDate);
			string query("CALL get_DailyEquityOptionValue_2(\"" + symbol + "\")");
			stmt->execute(query);
			do
			{
				std::unique_ptr<sql::ResultSet> res;
				res.reset(stmt->getResultSet());
				/// `stock_id`,`optiontype`, `trade_date`, `maturitydate`,`strike`, `tradeprice`,
				/// `asking_price`,`bid_price`,`volume`,`open_interest`
				while (res->next())
				{
					auto strikeVal = res->getDouble("strike");
					auto tdateStr = res->getString("trade_date").asStdString();
					auto traddate = dd::from_simple_string(tdateStr);

					auto optType = atoi(res->getString("optiontype").asStdString().c_str());
					auto matDateStr = res->getString("maturitydate").asStdString();
					auto matDate = dd::from_simple_string(matDateStr);
					Name optName = IDailyEquityOptionValue::ConstructName(symbol, traddate, static_cast<IDailyOptionValue::OptionType>(optType), matDate, strikeVal);

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
						auto rettradePrice = res->getDouble("tradeprice");
						auto ask = res->getDouble("asking_price");
						auto bid = res->getDouble("bid_price");
						auto retvol = res->getInt("volume");
						auto retopenInt = res->getInt("open_interest");

						/// construct the Option object.
						std::shared_ptr<IDailyEquityOptionValue> option = constructEntity(optName);
						option->SetTradeDate(traddate);

						/// contract date and maturity date are the same in the CME settlement files.
						/// if there is an option that differs we will handle it later
						option->SetMaturityDate(matDate);
						option->SetStrikePrice(strikeVal);
						option->SetOptionType(static_cast<IDailyEquityOptionValue::OptionType>(optType));

						/// set settlement price as the trade price since the last price is always zero in the
						/// settlement file.
						option->SetTradePrice(rettradePrice);
						option->SetAskingPrice(ask);
						option->SetBidPrice(bid);
						option->SetVolume(retvol);
						option->SetOpenInterest(retopenInt);

						LOG(INFO) << " DailyFuturesOptionValue " << symbol << " with trade date " << traddate << " constructed with " \
							<< "maturity date" << matDate \
							<< "strike price" << strike \
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