/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include "ExchangeRate.hpp"
#include "GroupRegister.hpp"
#include "EntityMgrUtil.hpp"

namespace derivative
{
	GROUP_REGISTER(ExchangeRate);
	ALIAS_REGISTER(ExchangeRate,IAsset);
	ALIAS_REGISTER(ExchangeRate,IExchangeRate);
	
	ExchangeRate::ExchangeRate (const Exemplar &ex)
		:m_name(TYPEID)
	{
		LOG(INFO) << "Exemplar object created " << endl;
	}

	ExchangeRate::ExchangeRate (const Name& nm)
		:m_name(nm)
	{
		LOG(INFO) << "Exchange object created for " << m_name << endl; 
	}

	ExchangeRate::~ExchangeRate ()
	{
		LOG(INFO) << "Destructor is called for " << m_name << endl; 
	}
	
	ExchangeRate::ExchangeRate(const Currency& domestic, const Currency& foreign)
		:m_domesticCurrency(domestic),
		m_foreignCurrency(foreign),
		m_name(TYPEID, std::hash<std::string>()(domestic.GetCode() + foreign.GetCode()))
	{	
		LOG(INFO) << "Exchange object created for " << m_name << endl;
	}

	std::shared_ptr<IMake> ExchangeRate::Make(const Name &nm)
	{
		/// Construct ExchangeRate from given name
		/// The caller required to register the constructed with object with EntityManager
		std::shared_ptr<ExchangeRate> exchangeRate = make_shared<ExchangeRate>(nm);
		
		/// return constructed object if no exception is thrown
		return exchangeRate;
	}

	std::shared_ptr<IMake> ExchangeRate::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("Invalid factory method call");
	}

} /* namespace derivative */