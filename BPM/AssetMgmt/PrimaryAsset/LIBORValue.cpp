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
		/// Construct LIBORValue from given name and register with EntityManager
		std::shared_ptr<LIBORValue> val = make_shared<LIBORValue>(nm);
		EntityMgrUtil::registerObject(nm, val);		
		LOG(INFO) << " LIBOR  " << nm << " is constructed and registered with EntityManager" << endl;

		/// return constructed object if no exception is thrown
		return val;
	}

	std::shared_ptr<IMake> LIBORValue::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("Invalid factory method call");
	}

	void LIBORValue::convert( istringstream  &input)
	{ 
		std::string elem;
		if (std::getline(input, elem,','))  m_rate = atof(elem.c_str()); else throw YahooSrcException("Invalid data");
		if (std::getline(input, elem,','))  m_date = boost::gregorian::from_string(elem); else throw YahooSrcException("Invalid data");	  
		return;            
	}
}