/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include "LIBOR.hpp"
#include "GroupRegister.hpp"
#include "EntityMgrUtil.hpp"

namespace derivative
{
	GROUP_REGISTER(LIBOR);
	ALIAS_REGISTER(LIBOR,IIBOR);

	LIBOR::LIBOR (const Exemplar &ex)
		:m_name(TYPEID)
	{
		LOG(INFO) << "Exemplar object created " << endl;
	}

	LIBOR::LIBOR (const Name& nm)
		:m_name(nm)
	{
		LOG(INFO) << "Exchange object created for " << m_name << endl; 
	}

	LIBOR::~LIBOR ()
	{
		LOG(INFO) << "Destructor is called for " << m_name << endl; 
	}
	
	LIBOR::LIBOR(const Currency& domestic, Maturity::MaturityType type)
		:m_currency(domestic),
		m_maturityType(type),
		m_name(TYPEID, std::hash<std::string>()(to_string(static_cast<int>(type)) + domestic.GetCode()))
	{	
		LOG(INFO) << "LIBOR object created for " << m_name << ", " << domestic << ", " << type << endl;
	}

	std::shared_ptr<IMake> LIBOR::Make(const Name &nm)
	{
		std::lock_guard<SpinLock> lock(m_lock);

		/// Construct LIBOR from given name
		/// The caller required to register the constructed with object with EntityManager
		std::shared_ptr<LIBOR> libor = make_shared<LIBOR>(nm);
		
		/// return constructed object if no exception is thrown
		return libor;
	}

	std::shared_ptr<IMake> LIBOR::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("Invalid factory method call");
	}

} /* namespace derivative */