/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_DAILYFUTURESOPTIONVALUE_H_
#define _DERIVATIVE_DAILYFUTURESOPTIONVALUE_H_

#define _VARIADIC_MAX 10 

#include <string>

#include "IMake.hpp"
#include "IDailyFuturesOptionValue.hpp"
#include "IFutures.hpp"
#include "Name.hpp"

namespace derivative
{
	class IFutures;
	/// Class DailyFuturesOptionValue represents daily value of a option
	/// snapshot.
	class DailyFuturesOptionValue : virtual public IDailyFuturesOptionValue,
		virtual public IMake
	{

	public:

		enum {TYPEID = CLASS_DAILYFUTURESOPTIONVALUE_TYPE};

		/// Constructor with Exemplar for the Creator Option value object
		DailyFuturesOptionValue (const Exemplar &ex);

		/// construct DailyFuturesOptionValue from all its attributes
		/// used in IMake::Make(..)
		DailyFuturesOptionValue (const Name& nm);

			DailyFuturesOptionValue(const Name& nm, double price, double strike, const OptionType& optType, \
			const dd::date& maturityDate, double highPrice, double lowPrice, double settle, int volume, \
			int openInt, const dd::date& tradeDate);

		std::shared_ptr<IMake> Make (const Name &nm);

		std::shared_ptr<IMake> Make (const Name &nm, const std::deque<boost::any>& agrs);

		/// construct DailyFuturesOptionValue from input stream.		
		virtual void convert( istringstream  &input);

		const Name& GetName()
		{
			return m_name;
		}

		void SetName(const Name& nm)
		{
			m_name = nm;
		}

		OptionType GetOptionType() const 
		{
			return m_optType;
		}

		//// Return the date this option was last traded.
		dd::date   GetTradeDate() const
		{
			return m_tradeDate;
		}

		dd::date  GetMaturityDate() const
		{
			return m_maturityDate;
		}

		virtual dd::date GetDeliveryDate() const
		{
			return m_deliveryDate;
		}


		virtual double GetStrikePrice() const
		{
			return m_strikePrice;
		}

		virtual double GetHighPrice() const
		{
			return m_highPrice;
		}

		virtual double GetLowPrice() const
		{
			return m_lowPrice;
		}

		virtual int GetVolume() const
		{
			return m_volume;
		}

		virtual int GetOpenInterest() const
		{
			return m_openInt;
		}

		virtual double GetDivYield() const
		{
			return 0;
		}
		
		/// return option.
		std::shared_ptr<IAsset> GetAsset() const
		{
			return m_futures;
		}
		
		/// return option.
		std::shared_ptr<const IFutures> GetFutures() const
		{
			return m_futures;
		}

		void SetTradeDate(const boost::gregorian::date& d)
		{
			m_tradeDate = d;
		}
			
		/// no last reported value for historical data
		double GetTradePrice() const
		{
			return m_tradePrice;
		}

		/// no settled price
		double GetSettledPrice() const
		{
			return m_settledPrice;
		}

		void SetMaturityDate(const boost::gregorian::date& d)
		{
			m_maturityDate = d;
		}

		void SetDeliveryDate(const boost::gregorian::date& d)
		{
			m_deliveryDate = d;
		}

		void SetOptionType(const OptionType& optType)
		{
			m_optType = optType;
		}

		virtual void SetHighPrice(double price)
		{
			m_highPrice = price;
		}

		/// set day's closing price
		virtual void SetLowPrice(double price)
		{
			m_lowPrice  = price;
		}

		virtual void SetTradePrice(double price)
		{
			m_tradePrice = price;
		}
		
		virtual void SetSettledPrice(double price)
		{
			m_settledPrice = price;
		}

		virtual void SetStrikePrice(double price)
		{
			m_strikePrice = price;
		}

		virtual void SetVolume(int vol)
		{
			m_volume = vol;
		}

		virtual void SetOpenInterest(int openInt)
		{
			m_openInt = openInt;
		}
		
		void SetOption(std::shared_ptr<IFutures> futures)
		{
			m_futures = futures;
		}

	private:

		OptionType m_optType;

		/// current price for the day
		double m_tradePrice;

		/// settled for the day
		double m_settledPrice;

		double m_strikePrice;

		/// high price. 
		double m_highPrice;

		/// The low price.
		double   m_lowPrice;
		
		/// volume. 
		int  m_volume;

		/// open interest 
		double m_openInt;

		/// The date this option value.
		dd::date m_tradeDate;

		/// The maturity date of this option value.
		dd::date m_maturityDate;

		/// delivery date of the undelying futures
		dd::date m_deliveryDate;

		/// Name(DailyFuturesOptionValue::TYPEID, std::hash<std::string>() \
		///  (string(symbol + m_tradedate)))
		/// Key[0] => "symbol"
		/// key[1] => "option type"
		/// key[2] => "trade date"
		/// key[3] => "strike"
		Name m_name;

		std::shared_ptr<IFutures> m_futures;
	};
}


/* namespace derivative */

#endif /* _DERIVATIVE_DAILYFUTURESOPTIONVALUE_H_ */