/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include <memory>
#include "Name.hpp"
#include "ZeroCouponBondMySQLDAO.hpp"
#include "GroupRegister.hpp"
#include "MySqlConnection.hpp"
#include "EntityManager.hpp"
#include "EntityMgrUtil.hpp"
#include "IDataSource.hpp"
#include "CountryHolder.hpp"
#include "CurrencyHolder.hpp"
#include "PrimaryAssetUtil.hpp"

namespace derivative
{
	GROUP_REGISTER(ZeroCouponBondMySQLDAO);
	DAO_REGISTER(IZeroCouponBond, MYSQL, ZeroCouponBondMySQLDAO);

	const int ZeroCouponBondMySQLDAO::MaxCount = 100;
	std::shared_ptr<IMake> ZeroCouponBondMySQLDAO::Make(const Name &nm)
	{
		/// Construct ZeroCouponBondMySQLDAO from given name and register with EntityManager
		std::shared_ptr<ZeroCouponBondMySQLDAO> dao = make_shared<ZeroCouponBondMySQLDAO>(nm);
		dao = dynamic_pointer_cast<ZeroCouponBondMySQLDAO>(EntityMgrUtil::registerObject(nm, dao));
		LOG(INFO) << " ZeroCouponBondMySQLDAO  " << nm << " is constructed and registered with EntityManager" << endl;

		/// return constructed object if no exception is thrown
		return dao;
	}

	const std::shared_ptr<IObject> ZeroCouponBondMySQLDAO::find(const Name& nm)
	{
		LOG(INFO) << "ZeroCouponBondMySQLDAO::find(..) is called for " << nm << endl;
		/// If we are here means, ZeroCouponBondValue object with the name nm is
		/// not in the registry. That's we should fetch the 
		/// object from the data data source.

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

		/// Populate the interestRate specific attributes
		findZeroCouponBond(nm);

		if (!m_rate)
		{
			throw MySQLSrcException("Rate not found");
		}					
		/// now return m_rate
		return m_rate;
	}

	void ZeroCouponBondMySQLDAO::findZeroCouponBond(const Name& name)
	{
		try 
		{
			std::auto_ptr<sql::Statement> stmt(m_con->createStatement());	
			stmt->execute("CALL get_ZeroCouponBond()");
			do 
			{
				std::unique_ptr<sql::ResultSet> res;
				res.reset(stmt->getResultSet());
				while (res->next())
				{
					std::string symbol = res->getString("symbol").asStdString();
					size_t id = res->getInt("bond_id");
					std::string description = res->getString("description").asStdString();
					std::string code = res->getString("countryCode").asStdString();

					/// get country for the given code
					CountryHolder& cntryHolder = CountryHolder::getInstance();
					const Country& cntry = cntryHolder.GetCountry(code);

					/// get currency
					std::string currCode = res->getString("currencyCode").asStdString();
					CurrencyHolder& currHolder = CurrencyHolder::getInstance();
					const Currency& curr = currHolder.GetCurrency(currCode);

					double faceValue = res->getDouble("faceValue");
					IZeroCouponBond::CategoryType category = static_cast<IZeroCouponBond::CategoryType>(res->getInt("category"));
					DayCount::DayCountType dayCount = static_cast<DayCount::DayCountType>(res->getInt("dayCount"));

					Name nm = IZeroCouponBond::ConstructName(symbol);
					std::shared_ptr<IZeroCouponBond> rate = PrimaryUtil::ConstructEntity<IZeroCouponBond>(nm);
					rate->SetSymbol(symbol);
					rate->SetDescription(description);
					rate->SetDayCount(dayCount);
					rate->SetCountry(cntry);
					rate->SetDomesticCurrency(curr);
					rate->SetFaceValue(faceValue);
					rate->SetCategory(category);
					rate = dynamic_pointer_cast<IZeroCouponBond>(EntityMgrUtil::registerObject(rate->GetName(), rate));

					LOG(INFO) << " New ZeroCouponBond object created for << description " << endl;

					if (id == name.GetObjId())
					{
						m_rate = rate;
					}
				}
			}
			while (stmt->getMoreResults());
		}
		catch (sql::SQLException &e) 
		{
			LOG(ERROR) << " Error loading country data " << endl;
			LOG(ERROR) << "# ERR: " << e.what();
			throw e;
		};		
	};
	
} /* namespace derivative */