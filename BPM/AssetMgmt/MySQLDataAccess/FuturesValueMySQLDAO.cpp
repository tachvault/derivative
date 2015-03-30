/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/


#include <memory>

#include "Name.hpp"
#include "FuturesValueMySQLDAO.hpp"
#include "Currency.hpp"
#include "GroupRegister.hpp"
#include "EntityManager.hpp"
#include "EntityMgrUtil.hpp"
#include "IDataSource.hpp"
#include "IFuturesValue.hpp"
#include "IFutures.hpp"
#include "EntityMgrUtil.hpp"
#include "PrimaryAssetUtil.hpp"
#include "RESTConnectionUtil.hpp"
namespace derivative
{
	GROUP_REGISTER(FuturesValueMySQLDAO);
	DAO_REGISTER(IFuturesValue, MYSQL, FuturesValueMySQLDAO);

	std::shared_ptr<IMake> FuturesValueMySQLDAO::Make(const Name &nm)
	{
		/// Construct FuturesValueMySQLDAO from given name and register with EntityManager
		std::shared_ptr<FuturesValueMySQLDAO> dao = make_shared<FuturesValueMySQLDAO>(nm);
		EntityMgrUtil::registerObject(nm, dao);
		LOG(INFO) << " FuturesValueMySQLDAO  " << nm << " is constructed and registered with EntityManager" << endl;

		/// return constructed object if no exception is thrown
		return dao;
	}

	std::shared_ptr<IMake> FuturesValueMySQLDAO::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("This function is not applicable to FuturesValueMySQLDAO");
	}

	const std::shared_ptr<IObject> FuturesValueMySQLDAO::find(const Name& nm)
	{
		LOG(INFO) << "FuturesValueMySQLDAO::find(..) is called for " << nm << endl;
		/// If we are here means, FuturesValue object with the name nm is
		/// not in the registry. That's we should fetch the 
		/// object from MySQL.

		/// constructEntity could throw exception.
		/// let the caller of this function handle
		m_futuresVal = PrimaryUtil::ConstructEntity<IFuturesValue>(nm);

		/// Now associate FuturesVal with futures.
		/// find futures with EntityUtil.
		/// find an object that is in the registry
		/// get the processed date.
		std::string symbol;
		dd::date tradeDate;
		dd::date deliveryDate;
		IFuturesValue::GetKeys(nm, symbol, tradeDate, deliveryDate);
		Name futuresName(IFutures::TYPEID, std::hash<std::string>()(symbol));

		try
		{
			/// This call should find the named object in
			/// registry. If not found then it should fetch
			/// the futures data from database, construct futures
			/// and register with entity manager.
			m_futuresVal->SetFutures(dynamic_pointer_cast<IFutures>(EntityMgrUtil::findObject(futuresName)));
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

		/// Once we have the m_futuresVal skeleton, it is time to populate the
		/// fields from the futuresVal fetched from MySQL.
		try
		{

			/// Make a connection to the MySQL if not connected
			/// log the SQLException error if thrown
			/// and retrow the exception so that the client
			/// can catch and take required action
			if (!m_con)
			{
				MySQLConnection::InitConnection(m_driver, m_con);
			}

			std::unique_ptr<sql::PreparedStatement> pstmt;
			std::unique_ptr<sql::ResultSet> res;

			std::string date = dd::to_iso_extended_string(tradeDate);
			sql::SQLString tdate(date.c_str());

			date = dd::to_iso_extended_string(deliveryDate);
			sql::SQLString ddate(date.c_str());

			/// contract_date, open_price, high, low, last_price, settle, volume, open_int
			pstmt.reset(m_con->prepareStatement("CALL get_futuresValueByfuturesId(?, ?, ?, @outtdate, @open, @high, @low, @lastPrice, @settle, @volume, @openInt, @count)"));
			pstmt->setUInt64(1, std::hash<std::string>()(symbol));
			pstmt->setDateTime(2, tdate);
			pstmt->setDateTime(3, ddate);
			pstmt->execute();

			pstmt.reset(m_con->prepareStatement("SELECT @outtdate AS _tdate, @open AS _open, @high AS _high, @low AS _low, @lastPrice AS _last_price, @settle AS _settle, @volume AS _volume, @openInt AS _openInt, @count AS _count"));
			res.reset(pstmt->executeQuery());
			while (res->next())
			{
				auto count = res->getInt("_count");
				if (count == 0) throw std::domain_error("No futures value found");

				auto tdateStr = res->getString("_tdate").asStdString();
				auto tradeDate = dd::from_simple_string(tdateStr);

				m_futuresVal->SetTradeDate(tradeDate);
				m_futuresVal->SetDeliveryDate(deliveryDate);

				m_futuresVal->SetPriceOpen(res->getDouble("_open"));
				m_futuresVal->SetHighPrice(res->getDouble("_high"));
				m_futuresVal->SetLowPrice(res->getDouble("_low"));
				m_futuresVal->SetLastPrice(res->getDouble("_last_price"));
				m_futuresVal->SetSettledPrice(res->getDouble("_settle"));
				m_futuresVal->SetVolume(res->getInt("_volume"));
				m_futuresVal->SetOpenInterest(res->getInt("_openInt"));

				/// set the settle price as the trade price
				m_futuresVal->SetTradePrice(res->getDouble("_settle"));

				LOG(INFO) << " Futures object  " << m_futuresVal->GetName() << " constructed for " << symbol << endl;
			}
		}
		catch (sql::SQLException &e)
		{
			LOG(ERROR) << " No futures data found in source for " << m_name << endl;
			LOG(ERROR) << "# ERR: " << e.what();
			throw e;
		}
		/// now return m_futuresVal
		return m_futuresVal;
	}

} /* namespace derivative */