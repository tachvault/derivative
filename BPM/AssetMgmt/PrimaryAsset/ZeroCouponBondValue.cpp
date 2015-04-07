/*
Copyright (c) 2013-2014 Nathan Muruganantha. All rights reserved.
*/

#include "ZeroCouponBondValue.hpp"
#include "GroupRegister.hpp"
#include "EntityMgrUtil.hpp"
#include "RESTConnectionUtil.hpp"
#include "DException.hpp"

namespace derivative
{
	GROUP_REGISTER(ZeroCouponBondValue);
	ALIAS_REGISTER(ZeroCouponBondValue,IAssetValue);
	ALIAS_REGISTER(ZeroCouponBondValue,IBondValue);
	ALIAS_REGISTER(ZeroCouponBondValue,IZeroCouponBondValue);

	ZeroCouponBondValue::ZeroCouponBondValue (const Exemplar &ex)
		:m_name(TYPEID),m_quotedPrice(0),m_tradePrice(0),m_yield(0)
	{		
	}

	ZeroCouponBondValue::ZeroCouponBondValue (const Name& nm)
		:m_name(nm),m_quotedPrice(0),m_tradePrice(0),m_yield(0),
		m_processedDate(dd::day_clock::local_day())
	{
		std::lock_guard<SpinLock> lock(m_lock);
		m_cashFlow = std::make_shared<IIRDataSrc::cashFlowSetType>();
	}

	std::shared_ptr<IMake> ZeroCouponBondValue::Make(const Name &nm)
	{
		std::lock_guard<SpinLock> lock(m_lock);

		/// Construct ZeroCouponBond from given name
		/// The caller required to register the constructed with object with EntityManager
		std::shared_ptr<ZeroCouponBondValue> val = make_shared<ZeroCouponBondValue>(nm);
		
		/// return constructed object if no exception is thrown
		return val;
	}

	std::shared_ptr<IMake> ZeroCouponBondValue::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("Invalid factory method call");
	}

	void ZeroCouponBondValue::convert( istringstream  &input)
	{ 

	}

	void ZeroCouponBondValue::generateCashFlow()
	{
		std::lock_guard<SpinLock> lock(m_lock);
		dd::date_duration dur = m_maturityDate - m_tradeDate;
		m_cashFlow->insert(std::make_pair(0, 0.0));
		m_cashFlow->insert(make_pair(dur.days(), 1.0));
	}
}