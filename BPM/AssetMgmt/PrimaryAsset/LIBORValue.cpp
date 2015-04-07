/*
Copyright (c) 2013-2014 Nathan Muruganantha. All rights reserved.
*/

#include "LIBORValue.hpp"
#include "GroupRegister.hpp"
#include "EntityMgrUtil.hpp"
#include "DException.hpp"
#include "DayCount.hpp"
#include "Maturity.hpp"

namespace derivative
{
	GROUP_REGISTER(LIBORValue);
	ALIAS_REGISTER(LIBORValue,IIBORValue);

	LIBORValue::LIBORValue (const Exemplar &ex)
		:m_name(TYPEID),m_rate(0)
	{
	}

	LIBORValue::LIBORValue (const Name& nm)
		:m_name(nm),m_rate(0)
	{
	}

	std::shared_ptr<IMake> LIBORValue::Make(const Name &nm)
	{
		std::lock_guard<SpinLock> lock(m_lock);

		/// Construct LIBORValue from given name
		/// The caller required to register the constructed with object with EntityManager
		std::shared_ptr<LIBORValue> val = make_shared<LIBORValue>(nm);
		
		/// return constructed object if no exception is thrown
		return val;
	}

	std::shared_ptr<IMake> LIBORValue::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("Invalid factory method call");
	}

	void LIBORValue::convert( istringstream  &input)
	{ 
		std::lock_guard<SpinLock> lock(m_lock);
		std::string elem;
		if (std::getline(input, elem,','))  m_rate = atof(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem,','))  m_date = boost::gregorian::from_string(elem); else throw YahooSrcException("Invalid data");	  
		return;            
	}
}