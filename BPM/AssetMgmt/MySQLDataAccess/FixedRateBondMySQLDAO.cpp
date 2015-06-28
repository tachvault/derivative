/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include <memory>
#include "Name.hpp"
#include "FixedRateBondMySQLDAO.hpp"
#include "GroupRegister.hpp"
#include "MySqlConnection.hpp"
#include "EntityManager.hpp"
#include "EntityMgrUtil.hpp"
#include "IDataSource.hpp"
#include "CountryHolder.hpp"
#include "CurrencyHolder.hpp"
#include "PrimaryAssetUtil.hpp"
#

namespace derivative
{
	GROUP_REGISTER(FixedRateBondMySQLDAO);
	DAO_REGISTER(IFixedRateBond, MYSQL, FixedRateBondMySQLDAO);

	const int FixedRateBondMySQLDAO::MaxCount = 100;
	std::shared_ptr<IMake> FixedRateBondMySQLDAO::Make(const Name &nm)
	{
		/// Construct FixedRateBondMySQLDAO from given name and register with EntityManager
		std::shared_ptr<FixedRateBondMySQLDAO> dao = make_shared<FixedRateBondMySQLDAO>(nm);
		dao = std::dynamic_pointer_cast<FixedRateBondMySQLDAO>(EntityMgrUtil::registerObject(nm, dao));
		LOG(INFO) << " FixedRateBondMySQLDAO  " << nm << " is constructed and registered with EntityManager" << endl;

		/// return constructed object if no exception is thrown
		return dao;
	}

	const std::shared_ptr<IObject> FixedRateBondMySQLDAO::find(const Name& nm)
	{
		LOG(INFO) << "FixedRateBondMySQLDAO::find(..) is called for " << nm << endl;
		/// If we are here means, FixedRateBondValue object with the name nm is
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
		findFixedRateBond(nm);

		if (!m_rate)
		{
			throw MySQLSrcException("Rate not found");
		}					
		/// now return m_rate
		return m_rate;
	}

	void FixedRateBondMySQLDAO::findFixedRateBond(const Name& name)
	{
		try 
		{
			std::auto_ptr<sql::Statement> stmt(m_con->createStatement());	
			stmt->execute("CALL get_FixedRateBond()");
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
					double couponRate = res->getDouble("couponRate");
					IFixedRateBond::CategoryType category = static_cast<IFixedRateBond::CategoryType>(res->getInt("category"));
					DayCount::DayCountType dayCount = static_cast<DayCount::DayCountType>(res->getInt("dayCount"));
					IFixedRateBond::CouponPeriodType couponPeriod = static_cast<IFixedRateBond::CouponPeriodType>(res->getInt("couponPeriod"));
					
					Name nm = IFixedRateBond::ConstructName(symbol);
					std::shared_ptr<IFixedRateBond> rate = PrimaryUtil::ConstructEntity<IFixedRateBond>(nm);
					rate->SetSymbol(symbol);
					rate->SetDescription(description);
					rate->SetDayCount(dayCount);
					rate->SetCountry(cntry);
					rate->SetDomesticCurrency(curr);
					rate->SetFaceValue(faceValue);
					rate->SetCategory(category);
					rate->SetCouponPeriod(couponPeriod);
					rate->SetCouponRate(couponRate);
					rate = dynamic_pointer_cast<IFixedRateBond>(EntityMgrUtil::registerObject(rate->GetName(), rate));

					LOG(INFO) << " New FixedRateBond object created for << description " << endl;

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