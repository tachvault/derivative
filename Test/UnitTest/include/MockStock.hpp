/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_MOCKSTOCK_H_
#define _DERIVATIVE_MOCKSTOCK_H_

#include "GroupRegister.hpp"
#include "IStock.hpp"
#include "Name.hpp"
#include "Currency.hpp"
#include "Exchange.hpp"

namespace derivative
{
	/// IStock provides public interface for Stock class.
	class MockStock : public IStock		
	{
	public:

		enum {TYPEID = CLASS_MOCKSTOCK_TYPE};
		
		MockStock(const Name& nm)
			:m_name(nm)
		{}

		const Name& GetName()
		{
			return m_name;
		}

		const std::string& GetSymbol() const
		{
			return m_symbol;
		}

		void SetName(const Name& nm)
		{
			m_name = nm;
		}

		void SetSymbol(const std::string& sym)
		{
			m_symbol = sym;
		}

		const std::string& GetDescription() const
		{
			throw std::logic_error("not supported");
		}

		virtual void SetVolatility(double vol)
		{
			throw std::logic_error("not supported");
		}

		const Currency& GetDomesticCurrency() const
		{
			throw std::logic_error("Not supported");
		}

		const Exchange& GetExchange() const
		{
			throw std::logic_error("Not supported");
		}

		const Country& GetCountry() const
		{
			throw std::logic_error("Not supported");
		}

		virtual double GetImpliedVol() const
		{
			return m_impliedVol;
		}

		virtual double GetHistVol() const
		{
			throw std::logic_error("Not supported");
		}
		
		void SetDomesticCurrency(const Currency& cur)
		{
			throw std::logic_error("Not supported");
		}

		void SetDescription(const std::string& des)
		{
			throw std::logic_error("Not supported");
		}

		void SetExchange(const Exchange& exchange)
		{
			throw std::logic_error("Not supported");
		}

		void SetCountry(const Country& cntry)
		{
			throw std::logic_error("Not supported");
		}

		virtual void SetImpliedVol(double vol)
		{
			m_impliedVol = vol;
		}

		virtual void SetHistVol(double vol)
		{
			throw std::logic_error("Not supported");
		}
						
	private:
		
		Name m_name;

		std::string m_symbol;

		double m_impliedVol;
	};

	ALIAS_REGISTER(MockStock, IAsset);
	ALIAS_REGISTER(MockStock, IPrimitiveSecurity);
	ALIAS_REGISTER(MockStock, IStock);
}

/* namespace derivative */

#endif /* _DERIVATIVE_STOCK_H_ */
