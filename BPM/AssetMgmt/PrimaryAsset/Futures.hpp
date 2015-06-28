/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_FUTURES_H_
#define _DERIVATIVE_FUTURES_H_

#include "IMake.hpp"
#include "IFutures.hpp"
#include "Name.hpp"
#include "Currency.hpp"
#include "Exchange.hpp"

namespace derivative
{
	/// IFutures provides public interface for Futures class.
	class Futures : virtual public IFutures,
		virtual public IMake
	{
	public:

		enum {TYPEID = CLASS_FUTURES_TYPE};

		/// Constructor with Exemplar for the Creator Futures object
		Futures (const Exemplar &ex);

		/// construct Futures from a given Name
		/// used in IMake::Make(..)
		Futures (const Name& nm);

		virtual ~Futures();

		/// constructor.
		Futures(const std::string& symbol, const std::string& description, const Exchange& ex);

		virtual std::shared_ptr<IMake> Make (const Name &nm);

		virtual std::shared_ptr<IMake> Make (const Name &nm, const std::deque<boost::any>& agrs);

		const Name& GetName()
		{
			std::lock_guard<SpinLock> lock(m_lock);
			return m_name;
		}

		const std::string& GetSymbol() const
		{
			std::lock_guard<SpinLock> lock(m_lock);
			return m_symbol;
		}

		const std::string& GetDescription() const
		{
			std::lock_guard<SpinLock> lock(m_lock);
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
			std::lock_guard<SpinLock> lock(m_lock);
			m_symbol = sym;
		}

		void SetDomesticCurrency(const Currency& cur)
		{
			m_currency = cur;
		}

		void SetDescription(const std::string& des)
		{
			std::lock_guard<SpinLock> lock(m_lock);
			m_description = des;
		}

		void SetExchange(const Exchange& exchange)
		{
			m_exchange = exchange;
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

		/// the primary exchange that the Futures is traded
		Exchange m_exchange; 

		std::atomic<double> m_impliedVol;

		std::atomic<double> m_histVol;

		mutable SpinLock m_lock;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_FUTURES_H_ */
