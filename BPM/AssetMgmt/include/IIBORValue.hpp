/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_IIBORVALUE_H_
#define _DERIVATIVE_IIBORVALUE_H_

#include <string>
#include <memory>

#include "Global.hpp"
#include "IIBOR.hpp"

namespace derivative
{
	/// interface exposes value of  Interbank offered rate
	/// at a given time. 
	class IIBORValue : public virtual IObject
	{
	public:

		enum {TYPEID = INTERFACE_IBORVALUE_TYPE};

		static Name ConstructName(const std::string& currCode, Maturity::MaturityType& maturity, const dd::date& date)
		{
			Name nm(TYPEID, std::hash<std::string>()(currCode + std::to_string(maturity) + \
				dd::to_simple_string(date)));
			nm.AppendKey(string("currency"), boost::any_cast<string>(currCode));
			nm.AppendKey(string("maturity"), boost::any_cast<int>(static_cast<int>(maturity)));
			nm.AppendKey(string("date"), boost::any_cast<dd::date>(date));
			return nm;
		}

		inline static void GetKeys(const Name& nm, std::string& currCode, Maturity::MaturityType& maturity, dd::date& date)
		{
			Name::KeyMapType keys = nm.GetKeyMap();
			auto i = keys.find("currency");
			currCode = boost::any_cast<std::string>(i->second);
			auto j = keys.find("maturity");
			maturity = static_cast<Maturity::MaturityType>(boost::any_cast<int>(j->second));
			auto k = keys.find("date");
			date = boost::any_cast<dd::date>(k->second);
		}

		/// return last reported (known) rate
		virtual double GetLastRate() const = 0;

		//// Return the reported date 
		virtual dd::date   GetReportedDate() const = 0;

		/// Get the associated rate
		virtual std::shared_ptr<IIBOR> GetRate() const = 0;	

		virtual void SetLastRate(double rate) = 0;

		virtual void SetReportedDate(const dd::date& d) = 0;

		virtual void convert( istringstream  &input) = 0;

		virtual void SetRate(const std::shared_ptr<IIBOR>& ir) = 0;
	};

	istringstream& operator >> (istringstream& input, std::shared_ptr<IIBORValue>& val)
	{
		val->convert(input);
		return input;
	}
}

/* namespace derivative */

#endif /* _IDERIVATIVE_IIBORVALUE_H_ */