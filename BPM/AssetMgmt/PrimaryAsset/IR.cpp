/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#include "IR.hpp"
#include "GroupRegister.hpp"
#include "EntityMgrUtil.hpp"

namespace derivative
{
	GROUP_REGISTER(IR);
	ALIAS_REGISTER(IR,IIR);

	IR::IR (const Exemplar &ex)
		:m_name(TYPEID)
	{
		LOG(INFO) << "Exemplar object created " << endl;
	}

	IR::IR (const Name& nm)
		:m_name(nm)
	{
		LOG(INFO) << "IR object created for " << m_name << endl; 
	}

	IR::~IR ()
	{
		LOG(INFO) << "Destructor is called for " << m_name << endl; 
	}
	
	IR::IR(const Country& domestic, Maturity::MaturityType type)
		:m_country(domestic),
		m_maturityType(type),
		m_name(TYPEID, std::hash<std::string>()(to_string(static_cast<int>(type)) + domestic.GetCode()))
	{	
		LOG(INFO) << "IR object created for " << m_name << endl;
	}

	std::shared_ptr<IMake> IR::Make(const Name &nm)
	{
		std::lock_guard<SpinLock> lock(m_lock);

		/// Construct IR from given name
		/// The caller required to register the constructed with object with EntityManager
		std::shared_ptr<IR> ir = make_shared<IR>(nm);
		
		/// return constructed object if no exception is thrown
		return ir;
	}

	std::shared_ptr<IMake> IR::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("Invalid factory method call");
	}

} /* namespace derivative */