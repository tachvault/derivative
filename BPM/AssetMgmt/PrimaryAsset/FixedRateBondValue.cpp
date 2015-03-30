/*
Copyright (c) 2013-2014 Nathan Muruganantha. All rights reserved.
*/

#include "FixedRateBondValue.hpp"
#include "GroupRegister.hpp"
#include "EntityMgrUtil.hpp"
#include "RESTConnectionUtil.hpp"
#include "DException.hpp"

namespace derivative
{

	GROUP_REGISTER(FixedRateBondValue);
	ALIAS_REGISTER(FixedRateBondValue,IAssetValue);
	ALIAS_REGISTER(FixedRateBondValue,IBondValue);
	ALIAS_REGISTER(FixedRateBondValue,IFixedRateBondValue);

	FixedRateBondValue::FixedRateBondValue (const Exemplar &ex)
		:m_name(TYPEID),m_quotedPrice(0),m_tradePrice(0),m_yield(0)
	{
	}

	FixedRateBondValue::FixedRateBondValue (const Name& nm)
		:m_name(nm),m_quotedPrice(0),m_tradePrice(0),m_yield(0),
		m_processedDate(dd::day_clock::local_day())
	{
		m_cashFlow = std::make_shared<IIRDataSrc::cashFlowSetType>();
	}

	std::shared_ptr<IMake> FixedRateBondValue::Make(const Name &nm)
	{
		/// Construct FixedRateBond from given name and register with EntityManager
		std::shared_ptr<FixedRateBondValue> FixedRateBondVal = make_shared<FixedRateBondValue>(nm);
		EntityMgrUtil::registerObject(nm, FixedRateBondVal);		
		LOG(INFO) << " FixedRateBond  " << nm << " is constructed and registered with EntityManager" << endl;

		/// return constructed object if no exception is thrown
		return FixedRateBondVal;
	}

	std::shared_ptr<IMake> FixedRateBondValue::Make(const Name &nm, const std::deque<boost::any>& agrs)
	{
		throw std::logic_error("Invalid factory method call");
	}

	void FixedRateBondValue::convert( istringstream  &input)
	{ 

	}

	void FixedRateBondValue::generateCashFlow()
	{
		auto period = static_cast<int>(m_bond->GetCouponPeriod());
		auto frequency = 12.0/period;
		if (!period) throw std::logic_error("This is not a coupon bond");

		/// determine number of cash flows
		dd::month_iterator m_itr(m_tradeDate, period);
		auto startDate = *m_itr;	

		dd::month_iterator temp_iter(startDate, period);
		int numCashFlows = 0;
		while (*temp_iter <= m_maturityDate)
		{
			numCashFlows++;
			++temp_iter;
		}

		/// Now add the timeline and cashFlow
		/// determine number of cash flows
		temp_iter = dd::month_iterator(startDate, period);
		int index = 0;
		while (*temp_iter < m_maturityDate)
		{
			m_cashFlow->insert(make_pair((*temp_iter - m_tradeDate).days(), m_bond->GetCouponRate()/frequency));
			++temp_iter;
		}

		/// add the last cash flow (last coupon plus face value)
		m_cashFlow->insert(make_pair((m_maturityDate - m_tradeDate).days(), 1 + m_bond->GetCouponRate()/frequency));
	}
}