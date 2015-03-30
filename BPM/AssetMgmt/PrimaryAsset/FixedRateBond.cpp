/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#include "Currency.hpp"
#include "Country.hpp"

#include "FixedRateBond.hpp"
#include "GroupRegister.hpp"
#include "EntityMgrUtil.hpp"

namespace derivative
{
	GROUP_REGISTER(FixedRateBond);
	ALIAS_REGISTER(FixedRateBond,IAsset);
	ALIAS_REGISTER(FixedRateBond,IBond);
	ALIAS_REGISTER(FixedRateBond,IPrimitiveSecurity);
	ALIAS_REGISTER(FixedRateBond,IFixedRateBond);

	FixedRateBond::FixedRateBond (const Exemplar &ex)
		:m_name(TYPEID)
	{
		LOG(INFO) << "Exemplar Constructor is called" << endl;
	}

	FixedRateBond::FixedRateBond (const Name& nm)
		:m_name(nm)
	{
		LOG(INFO) << "Constructor is called for " << m_name << endl;
	}

	FixedRateBond::~FixedRateBond()
	{
		LOG(INFO) << "Destructor is called for " << m_name << endl;
	}

	FixedRateBond::FixedRateBond(const std::string& symbol, const std::string& description, const Currency& curr, \
			const Country& cntry, double val, CouponPeriodType period, double couponRate)
	   :m_symbol(symbol),
		m_description(description),
		m_currency(curr),
		m_faceValue(val),
		m_country(cntry),
		m_couponPeriod(period),
		m_couponRate(couponRate),
		m_name(TYPEID, std::hash<std::string>()(symbol))
	{		
	}

	std::shared_ptr<IMake> FixedRateBond::Make(const Name &nm)
	{
		/// Construct FixedRateBond from given name and register with EntityManager
		std::shared_ptr<FixedRateBond> bond = make_shared<FixedRateBond>(nm);
		EntityMgrUtil::registerObject(nm, bond);		
		LOG(INFO) << " FixedRateBond  " << nm << " is constructed and registered with EntityManager" << endl;

		/// return constructed object if no exception is thrown
		return bond;
	}

	std::shared_ptr<IMake> FixedRateBond::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("Invalid factory method call");
	}

} /* namespace derivative */