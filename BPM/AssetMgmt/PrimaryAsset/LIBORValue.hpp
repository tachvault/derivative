/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_LIBORVALUE_H_
#define _DERIVATIVE_LIBORVALUE_H_

#include <string>
#include <memory>

#include "IMake.hpp"
#include "IIBORValue.hpp"
#include "IIRDataSrc.hpp"

namespace derivative
{
	class DeterministicAssetVol;

	/// Class LIBORValue represents current value of an
	/// LIBOR rate given the maturity and currency
	class LIBORValue : virtual public IIBORValue,
		virtual public IMake
	{

	public:

		enum {TYPEID = CLASS_LIBORVALUE_TYPE};

		/// Constructor with Exemplar for the Creator LIBOR value object
		LIBORValue (const Exemplar &ex);

		/// construct LIBORValue from a given Name
		/// used in IMake::Make(..)
		/// The object ID would be the same as that of underlying stock.
		LIBORValue (const Name& nm);

		shared_ptr<IMake> Make (const Name &nm);

		shared_ptr<IMake> Make (const Name &nm, const deque<boost::any>& agrs = std::deque<boost::any>());

		/// construct LIBORValue from input stream.		
		virtual void convert( istringstream  &input);

		const Name& GetName()
		{
			return m_name;
		}

		/// return last reported rate
		double GetLastRate() const
		{
			return m_rate;
		}

		/// Return the date this stock was last traded.
		dd::date   GetReportedDate() const
		{
			return m_date;
		}

		virtual double GetDivYield() const
		{
			throw std::logic_error("Invalid call");
		}

		void SetLastRate(double rate)
		{
			m_rate = rate;
		}

		void SetReportedDate(const dd::date& d)
		{
			m_date = d;
		}

		virtual std::shared_ptr<IIBOR> GetRate() const
		{
			return m_libor;
		}

		virtual void SetRate(const std::shared_ptr<IIBOR>& ir)
		{
			m_libor = ir;
		}
		
	private:

		double m_rate;

		dd::date m_date;

		/// Name(TYPEID, std::hash<std::string>()(to_string(maturity type) + \
		/// currency + to_string(date)))
		/// Key[0] => "maturity type"
		/// Key[1] => "currency"
		/// Key[2] => "date"
		Name m_name;

		std::shared_ptr<IIBOR> m_libor;
	};
}


/* namespace derivative */

#endif /* _DERIVATIVE_LIBORVALUE_H_ */