/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_STOCK_H_
#define _DERIVATIVE_STOCK_H_

#include "IMake.hpp"
#include "IStock.hpp"
#include "Name.hpp"
#include "Currency.hpp"
#include "Exchange.hpp"

namespace derivative
{
	/// IStock provides public interface for Stock class.
	class Stock : virtual public IStock,
		virtual public IMake
	{
	public:

		enum {TYPEID = CLASS_STOCK_TYPE};

		/// Constructor with Exemplar for the Creator Stock object
		Stock (const Exemplar &ex);

		/// construct Stock from a given Name
		/// used in IMake::Make(..)
		Stock (const Name& nm);

		virtual ~Stock();

		/// constructor.
		Stock(const std::string& symbol, const std::string& description, const Currency& currency, \
			const Exchange& ex, const Country& cntry);

		virtual std::shared_ptr<IMake> Make (const Name &nm);

		virtual std::shared_ptr<IMake> Make (const Name &nm, const std::deque<boost::any>& agrs);

		const Name& GetName()
		{
			return m_name;
		}

		const std::string& GetSymbol() const
		{
			return m_symbol;
		}

		const std::string& GetDescription() const
		{
			return m_description;
		}

		const Currency& GetDomesticCurrency() const
		{
			return m_currency;
		}

		const Exchange& GetExchange() const
		{
			return m_exchange;
		}

		const Country& GetCountry() const
		{
			return m_country;
		}

		virtual double GetImpliedVol() const
		{
			return m_impliedVol;
		}

		virtual double GetHistVol() const
		{
			return m_histVol;
		}

		virtual void SetName(const Name& nm)
		{
			throw std::logic_error("Not supported");
		}

		void SetSymbol(const std::string& sym)
		{
			m_symbol = sym;
		}

		void SetDomesticCurrency(const Currency& cur)
		{
			m_currency = cur;
		}

		void SetDescription(const std::string& des)
		{
			m_description = des;
		}

		void SetExchange(const Exchange& exchange)
		{
			m_exchange = exchange;
		}

		void SetCountry(const Country& cntry)
		{
			m_country = cntry;
		}

		virtual void SetImpliedVol(double vol)
		{
			m_impliedVol = vol;
		}

		virtual void SetHistVol(double vol)
		{
			m_histVol = vol;
		}

	private:

		/// Name(TYPEID, std::hash<std::string>()(ticker symbol))
		/// Key[0] => "symbol"
		Name m_name;

		std::string m_symbol;

		Currency m_currency;

		std::string m_description;

		/// the primary exchange that the stock is traded
		Exchange m_exchange; 

		Country m_country;

		double m_impliedVol;

		double m_histVol;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_STOCK_H_ */
