/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_FUTURESVALUE_H_
#define _DERIVATIVE_FUTURESVALUE_H_

#include <string>
#include <memory>

#include "IMake.hpp"
#include "IFuturesValue.hpp"
#include "Name.hpp"
#include "IFutures.hpp"
#include "ConstVol.hpp"

namespace derivative
{
	/// Class FuturesValue represents current value of a futures
	/// It is constructed as a Named type. That is, only only one
	/// shared object per futures would be shared among all client(s)
	/// threads. Callers will have the ability to refresh from external source.
	class FuturesValue : virtual public IFuturesValue,
		virtual public IMake
	{

	public:

		enum {TYPEID = CLASS_FUTURESVALUE_TYPE};

		/// Constructor with Exemplar for the Creator Futures value object
		FuturesValue (const Exemplar &ex);

		/// construct FuturesValue from a given Name
		/// used in IMake::Make(..)
		/// The object ID would be the same as that of underlying futures.
		FuturesValue (const Name& nm);

		virtual ~FuturesValue()
		{}

		shared_ptr<IMake> Make (const Name &nm);

		shared_ptr<IMake> Make (const Name &nm, const deque<boost::any>& agrs);

		/// construct FuturesValue from input stream.		
		virtual void convert( istringstream  &input);

		const Name& GetName()
		{
			return m_name;
		}

		double GetHighPrice() const
		{
			return m_priceHigh;
		}

		double GetLowPrice() const
		{
			return m_priceLow;
		}

		double GetPriceOpen() const
		{
			return m_priceOpen;
		}

		double GetPriceLast() const
		{
			return m_priceClose;
		}

		double GetTradePrice() const
		{
			return m_priceTrade;
		}
		
		virtual double GetSettledPrice() const
		{
			return m_settledPrice;
		}

		virtual dd::date GetTradeDate() const
		{
			return m_tradeDate;
		}

		virtual dd::date GetDeliveryDate() const
		{
			return m_deliveryDate;
		}

		//// Return the volume of the futures.
		virtual int GetVolume() const
		{
			return m_volume;
		}	

		virtual int GetOpenInterest() const
		{
			return m_openInterest;
		}

		virtual std::shared_ptr<IAsset> GetAsset() const
		{
			return m_futures;
		}

		/// return futures.
		virtual shared_ptr<IFutures> GetFutures() const
		{
			return m_futures;
		}

		virtual void SetHighPrice(double high)
		{
			m_priceHigh = high;
		}

		virtual void SetLowPrice(double low)
		{
			m_priceLow = low;
		}

		virtual void SetPriceOpen(double price)
		{
			m_priceOpen = price;
		}

		virtual void SetLastPrice(double price)
		{
			m_priceClose = price;
		}

		virtual void SetTradePrice(double price)
		{
			m_priceTrade = price;
		}

		virtual void SetSettledPrice(double settle)
		{
			m_settledPrice = settle;
		}

		virtual void SetTradeDate(const dd::date& d)
		{
			m_tradeDate = d;
		}

		virtual void SetDeliveryDate(const dd::date& d)
		{
			m_deliveryDate = d;
		}

		virtual void SetVolume(int vol)
		{
			m_volume = vol;
		}	

		virtual void SetOpenInterest(int openInt)
		{
			m_openInterest = openInt;
		}

		void SetFutures(std::shared_ptr<IFutures> futures)
		{
			m_futures = futures;
		}

		virtual double GetDivYield() const
		{
			return 0.0;
		}
				
	private:

		double m_priceHigh;

		double m_priceLow;

		double   m_priceOpen;

		double    m_priceClose;

		double m_priceTrade;

		dd::date m_tradeDate;

		dd::date m_deliveryDate;

		int    m_volume;

		int    m_openInterest;

		double m_settledPrice;

		/// Name(TYPEID, std::hash<std::string>()(symbol)) 
		/// Key[0] => "symbol"
		Name m_name;

		std::shared_ptr<IFutures> m_futures;
	};
}


/* namespace derivative */

#endif /* _DERIVATIVE_FUTURESVALUE_H_ */