/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#include "Currency.hpp"
#include "Exchange.hpp"

#include "Stock.hpp"
#include "GroupRegister.hpp"
#include "EntityMgrUtil.hpp"

namespace derivative
{
	GROUP_REGISTER(Stock);
	ALIAS_REGISTER(Stock,IAsset);
	ALIAS_REGISTER(Stock,IPrimitiveSecurity);
	ALIAS_REGISTER(Stock,IStock);

	Stock::Stock (const Exemplar &ex)
		:m_name(TYPEID)
	{
		LOG(INFO) << "Exemplar Constructor is called" << endl;
	}

	Stock::Stock (const Name& nm)
		:m_name(nm)
	{
		LOG(INFO) << "Constructor is called for " << m_name << endl;
	}

	Stock::~Stock()
	{
		LOG(INFO) << "Destructor is called for " << m_name << endl;
	}

	Stock::Stock(const std::string& symbol, const std::string& description, const Currency& currency, \
		   const Exchange& ex, const Country& cntry)
	   :m_symbol(symbol),
		m_description(description),
		m_currency(currency),
		m_exchange(ex),
		m_country(cntry),
		m_impliedVol(0.0),
		m_histVol(0.0),
		m_name(TYPEID, std::hash<std::string>()(symbol))
	{		
	}

	std::shared_ptr<IMake> Stock::Make(const Name &nm)
	{
		std::lock_guard<SpinLock> lock(m_lock);
		/// Construct Stock from given name
		/// The caller required to register the constructed with object with EntityManager
		std::shared_ptr<Stock> stock = make_shared<Stock>(nm);
		/// return constructed object if no exception is thrown
		return stock;
	}

	std::shared_ptr<IMake> Stock::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("Invalid factory method call");
	}

} /* namespace derivative */