/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_IRVALUE_H_
#define _DERIVATIVE_IRVALUE_H_

#include <string>
#include <memory>

#include "IMake.hpp"
#include "IIRValue.hpp"
#include "IIRDataSrc.hpp"

namespace derivative
{
	/// Class IRValue represents current value of a country's
	/// central bank  interest rate given the maturity type
	class IRValue : virtual public IIRValue,
		virtual public IMake
	{

	public:

		enum {TYPEID = CLASS_IRVALUE_TYPE};

		/// Constructor with Exemplar for the Creator IRValue value object
		IRValue (const Exemplar &ex);

		/// construct IRValue from a given Name
		/// used in IMake::Make(..)
		/// The object ID would be the same as that of underlying interest rate.
		IRValue (const Name& nm);

		shared_ptr<IMake> Make (const Name &nm);

		shared_ptr<IMake> Make (const Name &nm, const deque<boost::any>& agrs = std::deque<boost::any>());

		/// construct InterestRateValue from input stream.		
		virtual void convert( istringstream  &input);

		const Name& GetName()
		{
			return m_name;
		}

		/// return last reported rate
		virtual double GetLastRate() const
		{
			return m_rate;
		}

		/// Return the date this stock was last traded.
		virtual dd::date   GetReportedDate() const
		{
			return m_date;
		}

		virtual void SetLastRate(double rate)
		{
			m_rate = rate;
		}

		virtual void SetReportedDate(const dd::date& d)
		{
			m_date = d;
		}

		virtual std::shared_ptr<IIR> GetRate() const
		{
			return m_IR;
		}

		virtual void SetRate(const std::shared_ptr<IIR>& ir)
		{
			m_IR = ir;
		}

	private:

		double m_rate;

		dd::date m_date;

		/// Name(TYPEID, std::hash<std::string>()(country + std::to_string(maturity) + to_string(date)))
		/// Key[0] => "country code"
		/// Key[1] => "maturity type"
		/// Key[2] => "date"
		Name m_name;

		std::shared_ptr<IIR> m_IR;
	};
}


/* namespace derivative */

#endif /* _DERIVATIVE_IRVALUE_H_ */