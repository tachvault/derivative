/*
Copyright (C) Nathan Muruganantha 2013 - 2014
*/

#include "PrimaryAssetUtil.hpp"
#include "IDailyEquityOptionValue.hpp"
#include "IStockValue.hpp"
#include "IExchangeRate.hpp"
#include "IExchangeRateValue.hpp"
#include "IIRValue.hpp"
#include "IIR.hpp"
#include "EntityMgrUtil.hpp"
#include "DException.hpp"
#include "EntityManager.hpp"
#include "IRCurve.hpp"
#include "IFutures.hpp"
#include "IFuturesValue.hpp"
#include "Exchange.hpp"
#include "ExchangeExt.hpp"

namespace derivative
{
	namespace PrimaryUtil
	{
		double getSimpleRateToDF(double rate, double tenor, int frequency)
		{
			double df = 1.0 / std::pow((1 + rate / frequency), frequency*tenor);
			return df;
		}

		double getDFToSimpleRate(double df, double tenor, int frequency)
		{
			double rate = frequency*(std::pow(1.0 / df, 1.0 / (frequency*tenor)) - 1);
			return rate;
		}

		double getCompoundRateToDF(double rate, double tenor)
		{
			double df = std::exp(-rate*tenor);
			return df;
		}

		double getDFToCompoundRate(double df, double tenor)
		{
			double rate = -std::log(df) / tenor;
			return rate;
		}

		std::shared_ptr<IStockValue> getStockValue(const std::string& symbol)
		{
			/// in milliseconds
			static long long g_accessDur = 30000;

			std::shared_ptr<IStock> stock;
			std::shared_ptr<IStockValue> stockVal;

			/// find the stock from MySQL database
			Name stockName = IStock::ConstructName(symbol);
			stock = dynamic_pointer_cast<IStock>(EntityMgrUtil::findObject(stockName));

			/// find current stock value from YAHOO data source
			Name stockValName = IStockValue::ConstructName(symbol);
			stockVal = dynamic_pointer_cast<IStockValue>(EntityMgrUtil::findObject(stockValName, YAHOO));

			pt::ptime now = pt::second_clock::local_time();
			pt::time_duration diff = now - stockVal->GetAccessTime();
			if (diff.total_milliseconds() > g_accessDur)
			{
				EntityMgrUtil::refreshObject(stockVal, YAHOO);
			}
			stockVal->SetStock(stock);

			return stockVal;
		}

		std::shared_ptr<IFuturesValue> getFuturesValue(const std::string& symbol, const dd::date& tdate, const dd::date& deliverydate)
		{
			/// in milliseconds
			static long long g_accessDur = 30000;
			std::shared_ptr<IFutures> futures;
			std::shared_ptr<IFuturesValue> futuresVal;

			/// find the futures from MySQL database
			Name futName = IFutures::ConstructName(symbol);
			futures = dynamic_pointer_cast<IFutures>(EntityMgrUtil::findObject(futName));

			/// find current futuresvalue from MySQL data source again
			Name futValName = IFuturesValue::ConstructName(symbol, tdate, deliverydate);
			futuresVal = dynamic_pointer_cast<IFuturesValue>(EntityMgrUtil::findObject(futValName, MYSQL));

			pt::ptime now = pt::second_clock::local_time();
			pt::time_duration diff = now - futuresVal->GetAccessTime();
			if (diff.total_milliseconds() > g_accessDur)
			{
				EntityMgrUtil::refreshObject(futuresVal, MYSQL);
			}
			futuresVal->SetFutures(futures);
			return futuresVal;
		}

		std::shared_ptr<IExchangeRateValue> getExchangeRateValue(const std::string& domestic, const std::string& foreign)
		{
			std::shared_ptr<IExchangeRate> exchangeRate;
			std::shared_ptr<IExchangeRateValue> exchangeRateVal;

			/// in milliseconds
			static long long g_accessDur = 1200000;

			/// find the exchange rate from MySQL
			Name exchangeRateName = IExchangeRate::ConstructName(domestic, foreign);
			exchangeRate = dynamic_pointer_cast<IExchangeRate>(EntityMgrUtil::findObject(exchangeRateName));

			/// find current exchange rate value from YAHOO data source
			Name exchangeRateValName = IExchangeRateValue::ConstructName(domestic, foreign);
			exchangeRateVal = dynamic_pointer_cast<IExchangeRateValue>(EntityMgrUtil::findObject(exchangeRateValName, YAHOO));

			pt::ptime now = pt::second_clock::local_time();
			pt::time_duration diff = now - exchangeRateVal->GetAccessTime();
			if (diff.total_milliseconds() > g_accessDur)
			{
				EntityMgrUtil::refreshObject(exchangeRateVal, YAHOO);
			}
			exchangeRateVal->SetExchangeRate(exchangeRate);
			return exchangeRateVal;
		}

		/// Test finding  Interest rate objects.
		std::shared_ptr<IIRValue> FindInterestRate(const std::string& cntry, Maturity::MaturityType& maturity, dd::date& issue)
		{
			Name nm = IIRValue::ConstructName(cntry, maturity, issue);

			bool foundFlag = true;
			std::shared_ptr<IIRValue> ir;
			try
			{
				ir = dynamic_pointer_cast<IIRValue>(EntityMgrUtil::findObject(nm));
				cout << ir->GetLastRate() << ir->GetReportedDate() << ir->GetRate()->GetCountry().GetCode() << ir->GetRate()->GetMaturityType() << endl;
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

			return ir;
		}

		/// find simple interest rate
		double FindInterestRate(const std::string& cntry, double tenor, IRCurve::DataSourceType src)
		{
			/// Test for EntityManager singleton
			EntityManager& entMgr = EntityManager::getInstance();
			dd::date today = dd::day_clock::local_day();
			std::shared_ptr<IRCurve> irCurve = BuildIRCurve(src, cntry, today);
			const TermStructure& term = *(irCurve->GetTermStructure());

			double Bt = term(tenor);
			return PrimaryUtil::getDFToSimpleRate(Bt, tenor, 1);
		}

		std::string GetTickerSymbol(unsigned short src, std::shared_ptr<IStock> stock)
		{
			/// get the exchange corresponding to the ticker symbol
			ExchangeExt& symMap = ExchangeExt::getInstance();
			auto exchange = stock->GetExchange();

			/// using the exchange, get the extension
			auto ext = symMap.GetExchangeExt(src, exchange.GetExchangeName());

			/// now construct the complete symbol used by the given Data source
			if (ext.empty())
			{
				return stock->GetSymbol();
			}

			std::string symbol = stock->GetSymbol() + std::string(".") + ext;
			return symbol;
		}
	}
}