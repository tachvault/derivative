/*
Copyright (c) 2013-2014 Nathan Muruganantha. All rights reserved.
*/

#include "IRValue.hpp"
#include "GroupRegister.hpp"
#include "EntityMgrUtil.hpp"
#include "DException.hpp"
#include "DayCount.hpp"
#include "IIR.hpp"

namespace derivative
{
	GROUP_REGISTER(IRValue);
	ALIAS_REGISTER(IRValue,IIRValue);

	IRValue::IRValue (const Exemplar &ex)
		:m_name(TYPEID),m_rate(0)
	{
	}

	IRValue::IRValue (const Name& nm)
		:m_name(nm),m_rate(0)
	{
	}

	std::shared_ptr<IMake> IRValue::Make(const Name &nm)
	{
		std::lock_guard<SpinLock> lock(m_lock);

		/// Construct IRValue from given name
		/// The caller required to register the constructed with object with EntityManager
		std::shared_ptr<IRValue> val = make_shared<IRValue>(nm);
		
		/// return constructed object if no exception is thrown
		return val;
	}

	std::shared_ptr<IMake> IRValue::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("Invalid factory method call");
	}

	void IRValue::convert( istringstream  &input)
	{ 
		std::lock_guard<SpinLock> lock(m_lock);

		std::string elem;
		if (std::getline(input, elem,','))  m_rate = atof(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem,','))  m_date = boost::gregorian::from_string(elem); else throw YahooSrcException("Invalid data");	  
		return;            
	}	
}