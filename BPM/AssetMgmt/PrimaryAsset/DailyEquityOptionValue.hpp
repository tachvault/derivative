/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_DAILYEQUITYOPTIONVALUE_H_
#define _DERIVATIVE_DAILYEQUITYOPTIONVALUE_H_

#define _VARIADIC_MAX 10 

#include <string>

#include "IMake.hpp"
#include "IDailyEquityOptionValue.hpp"
#include "Name.hpp"

namespace derivative
{
	class IStock;
	/// Class DailyEquityOptionValue represents daily value of a option
	/// snapshot.
	class DailyEquityOptionValue : virtual public IDailyEquityOptionValue,
		virtual public IMake
	{

	public:

		enum {TYPEID = CLASS_DAILYEQUITYOPTIONVALUE_TYPE};

		/// Constructor with Exemplar for the Creator Option value object
		DailyEquityOptionValue (const Exemplar &ex);

		/// construct DailyEquityOptionValue from all its attributes
		/// used in IMake::Make(..)
		DailyEquityOptionValue (const Name& nm);

		DailyEquityOptionValue::DailyEquityOptionValue (const Name& nm, double price, double strike, const OptionType& optType, \
		const dd::date& maturityDate, double askingPrice, double bidPrice, int volume, \
		int openInt, const dd::date& tradeDate);

		std::shared_ptr<IMake> Make (const Name &nm);

		std::shared_ptr<IMake> Make (const Name &nm, const std::deque<boost::any>& agrs);

		/// construct DailyEquityOptionValue from input stream.		
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

		virtual double GetStrikePrice() const
		{
			return m_strikePrice;
		}

		virtual double GetAskingPrice() const
		{
			return m_askingPrice;
		}

		virtual double GetBidPrice() const
		{
			return m_bidPrice;
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
			return m_stock;
		}
		
		/// return option.
		std::shared_ptr<const IStock> GetStock() const
		{
			return m_stock;
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

		void SetMaturityDate(const boost::gregorian::date& d)
		{
			m_maturityDate = d;
		}

		void SetOptionType(const OptionType& optType)
		{
			m_optType = optType;
		}

		virtual void SetAskingPrice(double price)
		{
			m_askingPrice = price;
		}

		/// set day's closing price
		virtual void SetBidPrice(double price)
		{
			m_bidPrice  = price;
		}

		virtual void SetTradePrice(double price)
		{
			m_tradePrice = price;
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

		void SetOption(std::shared_ptr<IStock> stock)
		{
			m_stock = stock;
		}

	private:

		OptionType m_optType;

		/// current price for the day
		double m_tradePrice;

		double m_strikePrice;

		/// asking price. 
		double m_askingPrice;

		/// The bid price.
		double   m_bidPrice;

		/// volume. 
		int  m_volume;

		/// open interest 
		double m_openInt;

		/// The date this option value.
		dd::date m_tradeDate;

		/// The maturity date of this option value.
		dd::date m_maturityDate;

		/// Name(DailyEquityOptionValue::TYPEID, std::hash<std::string>() \
		///  (string(symbol + m_tradedate)))
		/// Key[0] => "symbol"
		/// key[1] => "option type"
		/// key[2] => "trade date"
		/// key[3] => "strike"
		Name m_name;

		std::shared_ptr<IStock> m_stock;
	};
}


/* namespace derivative */

#endif /* _DERIVATIVE_DAILYEQUITYOPTIONVALUE_H_ */