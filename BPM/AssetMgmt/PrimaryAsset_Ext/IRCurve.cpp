/*
Copyright (c) 2013-2014 Nathan Muruganantha. All rights reserved.
*/

#include <string>
#include <deque>
#include <vector>
#include <deque>
#include <future>

#include "IRCurve.hpp"
#include "GroupRegister.hpp"
#include "DException.hpp"
#include "EntityManager.hpp"
#include "EntityMgrUtil.hpp"

#include "CountryHolder.hpp"
#include "Country.hpp"
#include "IZeroCouponBondValue.hpp"
#include "IFixedRateBondValue.hpp"
#include "IIRDataSrc.hpp"
#include "TSLinear.hpp"
#include "IIR.hpp"
#include "IIRValue.hpp"
#include "IIBOR.hpp"
#include "IIBORValue.hpp"
#include "PrimaryAssetUtil.hpp"

namespace derivative
{
	template <typename T1, typename T2>
	void FindRateValues(const Name& nm, const std::string& code, std::vector<std::shared_ptr<T1> >& rates)
	{
		try
		{
			std::vector<std::shared_ptr<IObject> > objs;
			EntityMgrUtil::findObjects(nm, objs);
			std::vector<std::future<std::shared_ptr<IObject>>> futures;
			auto src = XIGNITE;
			for (std::shared_ptr<IObject> obj : objs)
			{
				try
				{
					std::shared_ptr<T2> rate = dynamic_pointer_cast<T2>(obj);
					LOG(INFO) << " Rate: for " << nm << ", " << rate->GetRateType() << ", " \
						<< rate->GetMaturityType() << endl;
					/// now we the rate, get the Value with last reported date
					Maturity::MaturityType mat = rate->GetMaturityType();
					dd::date repDate;
					Name valueName = T1::ConstructName(code, mat, repDate);
					futures.push_back(std::move(std::async(std::launch::async, &EntityMgrUtil::findObject, valueName, src)));
				}
				catch (std::logic_error& e)
				{
					LOG(ERROR) << " Cannot use " << obj->GetName() << " old rate " << endl;
				}				
			}

			for (auto &f : futures)
			{
				std::shared_ptr<T1> rateValue = dynamic_pointer_cast<T1>(f.get());
				rates.push_back(rateValue);
				LOG(INFO) << "Rate Value: " << rateValue->GetRate()->GetRateType() << "," << rateValue->GetReportedDate() << ", " << rateValue->GetLastRate() << endl;
			}

			/// sort the data by maturity date.
			sort(rates.begin(), rates.end(),
				[](std::shared_ptr<T1>& a, std::shared_ptr<T1>& b) -> bool
			{
				return static_cast<Maturity::MaturityType>(a->GetRate()->GetMaturityType()) < \
					static_cast<Maturity::MaturityType>(b->GetRate()->GetMaturityType());
			});
		}
		catch (RegistryException& e)
		{
			LOG(WARNING) << " RegistryException thrown " << e.what() << endl;
			throw e;
		}
		catch (...)
		{
			LOG(WARNING) << " Unknown Exception thrown " << endl;
			throw;
		}
	}

	//GROUP_REGISTER(IRCurve);

	IRCurve::IRCurve(const Exemplar &ex)
		:m_name(TYPEID)
	{
	}

	IRCurve::IRCurve(const Name& nm)
		: m_name(nm)
	{
	}

	/// construct IRCurve from country and process date
	IRCurve::IRCurve(const Country& cntry, const dd::date& processDate, IRCurve::DataSourceType src)
		: m_name(ConstructName(cntry.GetCode(), processDate, src)),
		m_country(cntry),
		m_processDate(processDate),
		m_src(src)
	{
	}

	void IRCurve::BuildIRCurve()
	{
		if (m_src == BOND)
		{
			BuildIRCurveFromBond();
		}
		else if (m_src == LIBOR)
		{
			BuildIRCurveFromIIBOR();
		}
		else if (m_src == YIELD)
		{
			BuildIRCurveFromIR();
		}
	}

	void IRCurve::BuildIRCurveFromBond()
	{
		/// get all the ZCB/coupon bonds corresponding to this country
		std::vector<std::shared_ptr<IZeroCouponBondValue> > zcbs;
		std::vector<std::shared_ptr<IFixedRateBondValue> > cbonds;
		std::vector<std::shared_ptr<IIRDataSrc> > bonds;

		/// get all the ZCB/coupon bonds corresponding to this country
		/// the trade date would the processed date of this IRCurve.
		FindZCBBondValues(zcbs);
		FindCouponBondValues(cbonds);

		if (zcbs.empty() && cbonds.empty()) throw std::domain_error("No zero coupon bonds found");

		/// Now get the cashflow map from each bond. Make a container
		/// to store the pointer to the cash flows.
		std::vector<std::shared_ptr<IIRDataSrc::cashFlowSetType> > CashFlows;
		std::vector<DeterministicCashflow> DeterministicCashflows(zcbs.size() + cbonds.size());

		/// generate cash flows and add them to the container vector cashFlows.
		/// get the timelines from each cash flow and make a unique vector.
		std::shared_ptr<IIRDataSrc::cashFlowSetType> allTimeLineSet = std::make_shared<IIRDataSrc::cashFlowSetType>();
		for (auto zcb : zcbs)
		{
			std::shared_ptr<IIRDataSrc> bond = dynamic_pointer_cast<IIRDataSrc>(zcb);
			bonds.push_back(bond);
			bond->generateCashFlow();
			std::shared_ptr<IIRDataSrc::cashFlowSetType> cfMap = bond->getCashFlowMap();
			for (auto& elem : *cfMap)
			{
				if (allTimeLineSet->find(elem.first) == allTimeLineSet->end())
				{
					allTimeLineSet->insert(IIRDataSrc::cashflowType(elem.first, 0));
				}
			}
		}
		for (auto cb : cbonds)
		{
			std::shared_ptr<IIRDataSrc> bond = dynamic_pointer_cast<IIRDataSrc>(cb);
			bonds.push_back(bond);
			bond->generateCashFlow();
			std::shared_ptr<IIRDataSrc::cashFlowSetType> cfMap = bond->getCashFlowMap();
			for (auto& elem : *cfMap)
			{
				if (allTimeLineSet->find(elem.first) == allTimeLineSet->end())
				{
					allTimeLineSet->insert(IIRDataSrc::cashflowType(elem.first, 0));
				}
			}
		}

		int maxtimeLine = allTimeLineSet->rbegin()->first;

		/// Now it is time to create unique cash flow objects.
		/// make union of cash flow and allTimeLineSet
		std::vector<std::shared_ptr<DeterministicCashflow> > deterministicCashflows;
		for (auto bond : bonds)
		{
			/// get the cash flow map of the bond
			std::shared_ptr<IIRDataSrc::cashFlowSetType> cfMap = dynamic_pointer_cast<IIRDataSrc>(bond)->getCashFlowMap();
			/// get the last element.
			auto last = cfMap->rbegin()->first;
			/// find the iterator corresponding to the last value from AllTimeLineSet
			auto iter = allTimeLineSet->find(last);
			/// now copy all elements from allTimeLineSet to cgMap if the elements are not in upto last
			for (auto iter1 = allTimeLineSet->begin(); iter1 != iter; ++iter1)
			{
				if (cfMap->find(iter1->first) == cfMap->end())
				{
					cfMap->insert(IIRDataSrc::cashflowType(iter1->first, 0.0));
				}
			}

			/// declare arrays
			Array<double, 1> timeline(cfMap->size());
			Array<double, 1> cashflow(cfMap->size());
			int i = 0;
			for (auto& elem : *cfMap)
			{
				timeline(i) = double(elem.first) / 365;
				cashflow(i) = elem.second;
				++i;
			}

			std::shared_ptr<IBondValue> bondVal = dynamic_pointer_cast<IBondValue>(bond);
			std::shared_ptr<IBond> bondObj = dynamic_pointer_cast<IBond>((bondVal)->GetAsset());
			std::shared_ptr<DeterministicCashflow> deterministicCashflow = std::make_shared<DeterministicCashflow>(timeline, cashflow, \
				bondVal->GetTradePrice() / bondObj->GetFaceValue());

			LOG(INFO) << "Bond " << bondObj->GetName() << " has cash flows  with size " \
				<< timeline.size() << "-" << cashflow.size() << endl;
			LOG(INFO) << timeline << endl;
			LOG(INFO) << cashflow << endl;
			LOG(INFO) << "Bond value " << bondVal->GetTradePrice() / bondObj->GetFaceValue() << endl;

			deterministicCashflows.push_back(deterministicCashflow);
		}

		Array<double, 1> alltimeline(allTimeLineSet->size());
		Array<double, 1> allcashflow(allTimeLineSet->size());
		alltimeline = 0.0;
		allcashflow = 0.0;

		LOG(INFO) << "Initialize term structure with time line and cash flow sizes " << alltimeline.size() << endl;
		LOG(INFO) << alltimeline << endl;
		LOG(INFO) << allcashflow << endl;

		/// construct a termstructure with number of maturies 
		/// equal to number of valid LIBOR rates.
		m_termStructure.reset(new TSLinear(alltimeline, allcashflow));

		m_termStructure->bootstrap(deterministicCashflows);
	}


	void IRCurve::BuildIRCurveFromIIBOR()
	{
		/// get all the zero coupon bonds corresponding to the 
		std::vector<std::shared_ptr<IIBORValue> > rates;

		FindLiborRateValues(rates);
		if (rates.empty()) throw std::domain_error("No LIBOR rates found");

		/// construct a termstructure with number of maturies 
		/// equal to number of valid LIBOR rates.
		std::map<double, double> rateMap;
		Array<double, 1> _timeline(rates.size() + 1);
		Array<double, 1> _zcbs(rates.size() + 1);
		_timeline = 0.0;
		_zcbs = 1.0;

		/// Now extract rates from LIBORs	
		for (std::shared_ptr<IIBORValue> rate : rates)
		{
			dd::date maturityDate = Maturity::getNextDate(m_processDate, rate->GetRate()->GetMaturityType());
			/// throw exception if process date is later than the maturity date
			if (m_processDate >= maturityDate)
			{
				LOG(ERROR) << " Process Date " << m_processDate \
					<< " is greater than the maturity date " << maturityDate << endl;
				throw std::logic_error("Old rate for the process date");
			}

			auto daysRemain = (maturityDate - m_processDate).days();

			/// Calculate the discount factor
			auto ra = rate->GetLastRate();
			DayCount::DayCountType dType = DayCount::DayCountType::actual_365;
			auto tT = DayCount::getPeriod(m_processDate, maturityDate, dType);
			rateMap.insert(make_pair(tT, rate->GetLastRate()));
			LOG(INFO) << " Insert T, rate (" << tT << "," << rate->GetLastRate() << endl;
		}

		int i = 1;
		for (auto &pr : rateMap)
		{
			double B_tT = PrimaryUtil::getSimpleRateToDF(pr.second, pr.first, 1);
			_timeline(i) = pr.first;
			_zcbs(i) = (i > 1) ? B_tT / _zcbs(i - 1) : B_tT;
			++i;
		}

		LOG(INFO) << " Time line " << _timeline << endl;
		LOG(INFO) << " Forward ZCBs " << _zcbs << endl;

		/// construct a termstructure with number of maturies 
		/// equal to number of valid LIBOR rates.
		m_termStructure.reset(new TSLinear(_timeline, _zcbs));
	}

	void IRCurve::BuildIRCurveFromIR()
	{
		/// get all the zero coupon bonds corresponding to the 
		std::vector<std::shared_ptr<IIRValue> > rates;

		FindIRRateValues(rates);

		if (rates.empty()) throw std::domain_error("No IR rates found");

		/// construct a termstructure with number of maturies 
		/// equal to number of valid interest rates.
		std::map<double, double> rateMap;
		Array<double, 1> _timeline(rates.size() + 1);
		Array<double, 1> _zcbs(rates.size() + 1);
		_timeline = 0.0;
		_zcbs = 1.0;

		/// Now extract rates from IRss	
		for (std::shared_ptr<IIRValue> rate : rates)
		{
			dd::date maturityDate = Maturity::getNextDate(m_processDate, rate->GetRate()->GetMaturityType());
			/// throw exception if process date is later than the maturity date
			if (m_processDate >= maturityDate)
			{
				LOG(ERROR) << " Process Date " << m_processDate \
					<< " is greater than the maturity date " << maturityDate << endl;
				throw std::logic_error("Old rate for the process date");
			}

			auto daysRemain = (maturityDate - m_processDate).days();

			/// Calculate the discount factor. It is mentioned in US treasury website that
			/// day count convention is actual/365.
			DayCount::DayCountType dType = DayCount::DayCountType::actual_365;
			auto tT = DayCount::getPeriod(m_processDate, maturityDate, dType);

			rateMap.insert(make_pair(tT, rate->GetLastRate()));
			LOG(INFO) << " Insert T, rate (" << tT << "," << rate->GetLastRate() << endl;
		}

		int i = 1;
		for (auto &pr : rateMap)
		{
			double B_tT = PrimaryUtil::getSimpleRateToDF(pr.second, pr.first, 2);
			_timeline(i) = pr.first;
			_zcbs(i) = (i > 1) ? B_tT / _zcbs(i - 1) : B_tT;
			++i;
		}

		LOG(INFO) << " Time line " << _timeline << endl;
		LOG(INFO) << " Z(0,T) " << _zcbs << endl;

		/// construct a termstructure with number of maturies 
		/// equal to number of valid interest rates.
		m_termStructure.reset(new TSLinear(_timeline, _zcbs));
	}

	void IRCurve::FindZCBBondValues(std::vector<std::shared_ptr<IZeroCouponBondValue> >& zcbs)
	{
		/// Get all the zero coupon bonds with symbol starting with this country code and
		/// tradedate as this processed date 
		Name nm = IZeroCouponBondValue::ConstructName(m_country.GetCode(), m_processDate);

		std::vector<std::shared_ptr<IObject> > bonds;
		/// find all the bonds with different maturity types having reported date on m_processDate
		EntityMgrUtil::findObjects(nm, bonds);
		for (std::shared_ptr<IObject> obj : bonds)
		{
			std::shared_ptr<IZeroCouponBondValue> bondVal = dynamic_pointer_cast<IZeroCouponBondValue>(obj);
			zcbs.push_back(bondVal);
		}
	}

	void IRCurve::FindCouponBondValues(std::vector<std::shared_ptr<IFixedRateBondValue> >& cbonds)
	{
		/// Get all the zero coupon bonds with symbol starting with this country code and
		/// tradedate as this processed date 
		Name nm = IFixedRateBondValue::ConstructName(m_country.GetCode(), m_processDate);

		std::vector<std::shared_ptr<IObject> > bonds;
		/// find all the bonds with different maturity types having reported date on m_processDate
		EntityMgrUtil::findObjects(nm, bonds);
		for (std::shared_ptr<IObject> obj : bonds)
		{
			std::shared_ptr<IFixedRateBondValue> bondVal = dynamic_pointer_cast<IFixedRateBondValue>(obj);
			cbonds.push_back(bondVal);
		}
	}

	void IRCurve::FindLiborRateValues(std::vector<std::shared_ptr<IIBORValue> >& rates)
	{
		/// get the currency for the country for this term structure
		auto curr = m_country.GetCurrency().GetCode();

		/// This is dummy variable. The MYSQLDAO for Libor knows it.
		/// we want to get all the latest rates for curr.
		Maturity::MaturityType maturity = Maturity::MaturityType::M1;
		Name nm = IIBOR::ConstructName(curr, maturity);

		FindRateValues<IIBORValue, IIBOR>(nm, curr, rates);
	}

	void IRCurve::FindIRRateValues(std::vector<std::shared_ptr<IIRValue> >& rates)
	{
		/// get the country code for this term structure and get all
		/// the fed rates for the given country 
		auto cntry = m_country.GetCode();
		/// This is dummy variable. The MYSQLDAO for IR knows it.
		/// we want to get all the rates for country as of processed date.
		Maturity::MaturityType maturity = Maturity::MaturityType::M1;
		Name nm = IIR::ConstructName(cntry, maturity);

		FindRateValues<IIRValue, IIR>(nm, cntry, rates);
	}

	std::shared_ptr<IRCurve> BuildIRCurve(const IRCurve::DataSourceType src, const std::string& cntryCode, const dd::date& edate)
	{
		/// register the exemplar here until the bug on visual C++ is fixed.
		GroupRegister IRCurveGrp(IRCurve::TYPEID, std::make_shared<IRCurve>(Exemplar()));

		Name curName = IRCurve::ConstructName(cntryCode, edate, src);
		/// check if this object is already registered with EntityManager

		std::shared_ptr<IObject> obj;
		EntityManager& entMgr = EntityManager::getInstance();
		/// check if the object is already registered with EntityManager
		try
		{
			obj = entMgr.findObject(curName);
		}
		catch (RegistryException& e)
		{
			LOG(INFO) << e.what();
		}
		/// if the object is found then register return
		if (obj)
		{
			std::shared_ptr<IRCurve> curve = dynamic_pointer_cast<IRCurve>(obj);
			return curve;
		}

		/// if not in registry then build the object, register and return.		
		CountryHolder& handler = CountryHolder::getInstance();
		const Country& cntry = handler.GetCountry(cntryCode);

		std::shared_ptr<IRCurve>  irCurve = std::make_shared<IRCurve>(cntry, edate, src);

		try
		{
			/// Build the curve from the given data source
			irCurve->BuildIRCurve();

			/// if sucessfully built (without any exception)
			/// register with EntityManager
			entMgr.registerObject(irCurve->GetName(), irCurve);
		}
		catch (RegistryException& e)
		{
			LOG(ERROR) << "Throw exception.. The object not in registry and unable to register or build" << endl;
			LOG(ERROR) << e.what();
			throw e;
		}
		return irCurve;
	}
}