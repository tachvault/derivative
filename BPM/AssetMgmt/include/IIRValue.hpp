/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_IIRVALUE_H_
#define _DERIVATIVE_IIRVALUE_H_

#include <string>
#include <memory>

#include "Global.hpp"
#include "Maturity.hpp"

namespace derivative
{
	class Country;
	class IIR;

	/// IInterestRateValue interface exposes value of  interest rate
	/// at a given time. 
	class IIRValue : public virtual IObject
	{

	public:

		enum {TYPEID = INTERFACE_IRVALUE_TYPE};

		inline static Name ConstructName(const std::string& cntryCode, Maturity::MaturityType& maturity, \
			const dd::date& date)
		{
			Name nm(TYPEID, std::hash<std::string>()(cntryCode + std::to_string(maturity) + \
				dd::to_simple_string(date)));
			nm.AppendKey(string("country"), boost::any_cast<string>(cntryCode));
			nm.AppendKey(string("maturity"), boost::any_cast<int>(static_cast<int>(maturity)));
			nm.AppendKey(string("date"), boost::any_cast<dd::date>(date));
			return nm;
		}

		inline static void GetKeys(const Name& nm, std::string& cntryCode, Maturity::MaturityType& maturity, \
			dd::date& date)
		{
			Name::KeyMapType keys = nm.GetKeyMap();
			auto i = keys.find("country");
			cntryCode = boost::any_cast<std::string>(i->second);
			auto j = keys.find("maturity");
			maturity = static_cast<Maturity::MaturityType>(boost::any_cast<int>(j->second));
			auto k = keys.find("date");
			date = boost::any_cast<dd::date>(k->second);			
		}

		/// return last reported (known) rate
		virtual double GetLastRate() const = 0;

		virtual std::shared_ptr<IIR> GetRate() const = 0;

		//// Return the reported date 
		virtual dd::date   GetReportedDate() const = 0;		

		virtual void SetReportedDate(const dd::date& d) = 0;

		virtual void SetLastRate(double rate) = 0;

		virtual void SetRate(const std::shared_ptr<IIR>& ir) = 0;

		virtual void convert( istringstream  &input) = 0;
	};
}


/* namespace derivative */

#endif /* _IDERIVATIVE_IIRVALUE_H_ */