/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#include "Currency.hpp"
#include "Exchange.hpp"
#include "Futures.hpp"
#include "GroupRegister.hpp"
#include "EntityMgrUtil.hpp"

namespace derivative
{
	GROUP_REGISTER(Futures);
	ALIAS_REGISTER(Futures,IAsset);
	ALIAS_REGISTER(Futures,IPrimitiveSecurity);
	ALIAS_REGISTER(Futures,IFutures);

	Futures::Futures (const Exemplar &ex)
		:m_name(TYPEID)
	{
		LOG(INFO) << "Exemplar Constructor is called" << endl;
	}

	Futures::Futures (const Name& nm)
		:m_name(nm)
	{
		LOG(INFO) << "Constructor is called for " << m_name << endl;
	}

	Futures::~Futures()
	{
		LOG(INFO) << "Destructor is called for " << m_name << endl;
	}

	Futures::Futures(const std::string& symbol, const std::string& description,  const Exchange& ex)
	   :m_symbol(symbol),
		m_description(description),
		m_exchange(ex),
		m_impliedVol(0.0),
		m_histVol(0.0),
		m_name(TYPEID, std::hash<std::string>()(symbol))
	{		
	}

	std::shared_ptr<IMake> Futures::Make(const Name &nm)
	{
		std::lock_guard<SpinLock> lock(m_lock);

		/// Construct Futures from given name
		/// The caller required to register the constructed with object with EntityManager
		std::shared_ptr<Futures> futures = make_shared<Futures>(nm);
		
		/// return constructed object if no exception is thrown
		return futures;
	}

	std::shared_ptr<IMake> Futures::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("Invalid factory method call");
	}

} /* namespace derivative */