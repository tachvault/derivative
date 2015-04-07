/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#include "Currency.hpp"
#include "Country.hpp"

#include "ZeroCouponBond.hpp"
#include "GroupRegister.hpp"
#include "EntityMgrUtil.hpp"

namespace derivative
{
	GROUP_REGISTER(ZeroCouponBond);
	ALIAS_REGISTER(ZeroCouponBond,IAsset);
	ALIAS_REGISTER(ZeroCouponBond,IBond);
	ALIAS_REGISTER(ZeroCouponBond,IPrimitiveSecurity);
	ALIAS_REGISTER(ZeroCouponBond,IZeroCouponBond);

	ZeroCouponBond::ZeroCouponBond (const Exemplar &ex)
		:m_name(TYPEID)
	{
		LOG(INFO) << "Exemplar Constructor is called" << endl;
	}

	ZeroCouponBond::ZeroCouponBond (const Name& nm)
		:m_name(nm)
	{
		LOG(INFO) << "Constructor is called for " << m_name << endl;
	}

	ZeroCouponBond::~ZeroCouponBond()
	{
		LOG(INFO) << "Destructor is called for " << m_name << endl;
	}

	ZeroCouponBond::ZeroCouponBond(const std::string& symbol, const std::string& description, const Currency& currency, \
		const Country& cntry, double val)
		:m_symbol(symbol),
		m_description(description),
		m_currency(currency),
		m_faceValue(val),
		m_country(cntry),
		m_name(TYPEID, std::hash<std::string>()(symbol))
	{		
	}

	std::shared_ptr<IMake> ZeroCouponBond::Make(const Name &nm)
	{
		std::lock_guard<SpinLock> lock(m_lock);

		/// Construct ZeroCouponBond from given name
		/// The caller required to register the constructed with object with EntityManager
		std::shared_ptr<ZeroCouponBond> bond = make_shared<ZeroCouponBond>(nm);
		
		/// return constructed object if no exception is thrown
		return bond;
	}

	std::shared_ptr<IMake> ZeroCouponBond::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("Invalid factory method call");
	}

} /* namespace derivative */