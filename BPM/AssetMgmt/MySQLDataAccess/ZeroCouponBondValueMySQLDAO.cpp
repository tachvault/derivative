/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include <memory>
#include "Name.hpp"
#include "ZeroCouponBondValueMySQLDAO.hpp"
#include "GroupRegister.hpp"
#include "MySqlConnection.hpp"
#include "EntityManager.hpp"
#include "EntityMgrUtil.hpp"
#include "IDataSource.hpp"
#include "CountryHolder.hpp"
#include "IZeroCouponBond.hpp"
#include "PrimaryAssetUtil.hpp"

namespace derivative
{
	GROUP_REGISTER(ZeroCouponBondValueMySQLDAO);
	DAO_REGISTER(IZeroCouponBondValue, MYSQL, ZeroCouponBondValueMySQLDAO);

	std::shared_ptr<IMake> ZeroCouponBondValueMySQLDAO::Make(const Name &nm)
	{
		/// Construct ZeroCouponBondValueMySQLDAO from given name and register with EntityManager
		std::shared_ptr<ZeroCouponBondValueMySQLDAO> dao = make_shared<ZeroCouponBondValueMySQLDAO>(nm);
		EntityMgrUtil::registerObject(nm, dao);
		LOG(INFO) << " ZeroCouponBondValueMySQLDAO  " << nm << " is constructed and registered with EntityManager" << endl;

		/// return constructed object if no exception is thrown
		return dao;
	}

	std::shared_ptr<IZeroCouponBondValue> ZeroCouponBondValueMySQLDAO::constructEntity(const Name& nm)
	{
		/// First get the ZeroCouponBondValue exemplar object from the registry
		/// Get the EntityManager instance
		EntityManager& entMgr= EntityManager::getInstance();	

		/// get the concrete types for the given alias name
		std::vector<Name> names;
		entMgr.findAlias(nm, names);
		if (names.empty())
		{
			LOG(ERROR) << " No concrete name found for " << nm << endl;
			throw DataSourceException("ZeroCouponBondValue not bound with correct alias");
		}

		Name entityName((*names.begin()).GetGrpId(), nm.GetObjId());

		/// copy the keys from Alias Name to concrete name
		entityName.SetKeys(nm.GetKeyMap());

		/// get the examplar object for the ZeroCouponBondValue
		/// Exemplar objects should be initialized
		/// during global initialization time.
		std::shared_ptr<IObject> exemplar = entMgr.findObject(Name(entityName.GetGrpId()));

		/// Now we have the exampler object. 
		/// Make the exemplar ZeroCouponBondValue to construct ZeroCouponBondValue for the given name
		std::shared_ptr<IZeroCouponBondValue> value;
		try
		{
			value = dynamic_pointer_cast<IZeroCouponBondValue>(dynamic_pointer_cast<IMake>(exemplar)->Make(entityName));
		}
		catch(RegistryException& e)
		{
			LOG(WARNING) << " Object already found in registry " << e.what() << endl;
			return value;
		}
		/// we don't want to catch other exceptions here.. Let the client code handle that.

		/// if the bond is not in the registry and constructed sucessfully
		/// then we need to find and associate ZeroCouponBond with ZeroCouponBondValue
		std::string symbol;
		dd::date tradeDate;
		IZeroCouponBondValue::GetKeys(nm, symbol, tradeDate);
		Name ZeroCouponBondName = IZeroCouponBond::ConstructName(symbol);
		try
		{
			/// get the ZeroCouponBond from registry or from data source
			/// set the ZeroCouponBond object with ZeroCouponBondValue
			value->SetZeroCouponBond(dynamic_pointer_cast<IZeroCouponBond>(EntityMgrUtil::findObject(ZeroCouponBondName)));
		}
		catch(RegistryException& e)
		{
			LOG(WARNING) << " RegistryException thrown " << e.what() << endl;
			throw e;
		}
		catch(...)
		{
			LOG(WARNING) << " Unknown Exception thrown " << endl;
			throw;
		}
		return value;
	}

	const std::shared_ptr<IObject> ZeroCouponBondValueMySQLDAO::find(const Name& nm)
	{
		LOG(INFO) << "ZeroCouponBondValueMySQLDAO::find(..) is called for " << nm << endl;
		/// If we are here means, ZeroCouponBondValue object with the name nm is
		/// not in the registry. That's we should fetch the 
		/// object from the data data source.

		/// constructEntity could throw exception.
		/// let the caller of this function handle
		std::shared_ptr<IZeroCouponBondValue> value = constructEntity(nm);

		/// Once we have the m_stock skeleton, it is time to populate the
		/// fields from the stock fetched from the database

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
		catch(sql::SQLException &e) 
		{
			LOG(ERROR) << " MySQL throws exception while connecting to the database " << nm << endl;
			LOG(ERROR) << "# ERR: " << e.what();
			throw e;
		}	

		/// Populate the stock specific attributes
		return findBond(value);
	}

	void ZeroCouponBondValueMySQLDAO::find(const Name& nm, std::vector<std::shared_ptr<IObject> > & entities)
	{
		LOG(INFO) << "ZeroCouponBondValueMySQLDAO::find(..) is called for " << nm << endl;

		/// If we are here means, ZeroCouponBondValue object with trade date in nm are
		/// not in the registry. That's we should fetch the 
		/// object from the data data source.
		try
		{
			if (!m_con)
			{
				MySQLConnection::InitConnection(m_driver, m_con);
			}
		}
		catch(sql::SQLException &e) 
		{
			LOG(ERROR) << " MySQL throws exception while connecting to the database " << nm << endl;
			LOG(ERROR) << "# ERR: " << e.what();
			throw e;
		}	

		/// Populate the bond value specific attributes
		findZeroCouponBondValue(nm, entities);
	}

	std::shared_ptr<IObject> ZeroCouponBondValueMySQLDAO::findBond(std::shared_ptr<IZeroCouponBondValue>& bond)
	{
		try
		{
			std::unique_ptr<sql::PreparedStatement> pstmt;
			std::unique_ptr<sql::ResultSet> res;

			/// get the processed date.
			std::string symbol;
			dd::date processedDate;
			IZeroCouponBondValue::GetKeys(bond->GetName(), symbol, processedDate);
			std::string date = dd::to_iso_extended_string(processedDate);
			sql::SQLString processdate(date.c_str());

			pstmt.reset(m_con->prepareStatement ("CALL get_ZeroCouponBondValue(?, ?, @out_tradedate,\
												 @out_tradedPrice, @out_quotedPrice, @out_maturitydate, @out_yield)"));

			pstmt->setString(1, symbol.c_str());
			pstmt->setDateTime(2, processdate);
			pstmt->execute();

			pstmt.reset(m_con->prepareStatement("SELECT @out_tradedate AS tdate, @out_tradedPrice AS tPrice, \
												@out_quotedPrice AS qPrice, @out_maturitydate AS mdate, @out_yield AS yield"));			
			res.reset(pstmt->executeQuery());
			while (res->next())
			{
				/// trade date
				auto stdate = res->getString("tdate").asStdString();
				if (stdate.empty())
				{
					/// unbind Libor rate value object and set it null.
					EntityManager& entMgr= EntityManager::getInstance();
					entMgr.unbind(dynamic_pointer_cast<IObject>(bond));
					bond = nullptr;					
					throw std::domain_error("No liborratevalue data found");
				}

				auto tDate = boost::any_cast<dd::date>(dd::from_simple_string(stdate));

				/// trade price
				auto tPrice = res->getDouble("tPrice");

				/// quoted price
				auto qPrice = res->getDouble("qPrice");

				/// maturity date
				auto smdate = res->getString("mdate");
				auto mDate = boost::any_cast<dd::date>(dd::from_simple_string(smdate));

				/// make sure the trade date is less than the maturity date
				if (tDate > mDate) throw std::logic_error("Invalid trade and or maturity dates"); 

				// yield
				auto yield = res->getDouble("yield");

				bond->SetProcessedDate(dd::day_clock::local_day());
				bond->SetMaturityDate(mDate);
				bond->SetTradeDate(tDate);
				bond->SetYield(yield);
				bond->SetTradePrice(tPrice);
				bond->SetQuotedPrice(qPrice);
			    LOG(INFO) << " ZeroCouponBondValue " << symbol << " with trade date " << tDate << " constructed with " \
				          << "maturity date" << mDate << "trade date" << tDate << endl;
				LOG(INFO) << "trade price date" << tPrice << "quoted price date" << qPrice 	<< "yield " << yield <<	endl;

				return bond;
			}
		}
		catch (sql::SQLException &e) 
		{
			LOG(ERROR) << " No ZCB data found in source for " << m_name << endl;
			LOG(ERROR) << "# ERR: " << e.what();
			throw e;
		}
	}

	void ZeroCouponBondValueMySQLDAO::findZeroCouponBondValue(const Name& nm, \
		std::vector<std::shared_ptr<IObject> > & entities)
	{
		dd::date tdate;
		std::string cntryCode;
		IZeroCouponBondValue::GetKeys(nm, cntryCode, tdate);

		try
		{
			EntityManager& entMgr= EntityManager::getInstance();

			std::unique_ptr<sql::Statement> stmt;
			std::unique_ptr<sql::ResultSet> res;

			stmt.reset(m_con->createStatement());
			std::string query = std::string("select symbol from zerocouponbond where countrycode like \"") + cntryCode + "%\"";
			res.reset(stmt->executeQuery(query));

			while(res->next())
			{
				auto symbol = res->getString(1);
				Name bondName = IZeroCouponBondValue::ConstructName(symbol, tdate);
				try
				{
					/// try getting if from EntityManager. If not in entitity manager
					/// then fetch from database
					std::shared_ptr<IObject> obj = entMgr.findObject(bondName);	
					entities.push_back(obj);

					/// if found then skip the steps of getting from the database.
					continue;
				}
				catch(RegistryException& e)
				{
					LOG(INFO) << bondName << " not found in registry. fetch from data store " << endl;
				}

				/// no object with same type or child type found
				/// in registry. We have to fetch from database		
				try
				{						
					LOG(INFO) << " Attempt to fetch from data store " << endl;
					std::shared_ptr<IObject>obj = find(bondName);
					entities.push_back(obj);
				}
				catch (sql::SQLException &e) 
				{
					LOG(ERROR) << " No data found in source for " << bondName << endl;
					LOG(ERROR) << "# ERR: " << e.what();
					throw e;
				}	
				catch(std::domain_error& e)
				{
				}		
			}
		}
		catch (sql::SQLException &e) 
		{
			LOG(ERROR) << " No data found in source for " << nm << endl;
			LOG(ERROR) << "# ERR: " << e.what();
			throw e;
		}
	};	
} /* namespace derivative */