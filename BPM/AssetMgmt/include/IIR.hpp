/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_IIR_H_
#define _DERIVATIVE_IIR_H_

#include "IObject.hpp"
#include "Global.hpp"
#include "Maturity.hpp"

namespace derivative
{
	class Country;

	/// Interface for central bank/treasury risk free rate class
	class IIR: public virtual IObject
	{

	public:

		enum {TYPEID = INTERFACE_IR_TYPE};

		inline static Name ConstructName(const std::string& cntryCode, Maturity::MaturityType& maturity)
		{
			Name nm(TYPEID, std::hash<std::string>()(cntryCode + std::to_string(maturity)));
			nm.AppendKey(string("country"), boost::any_cast<string>(cntryCode));
			nm.AppendKey(string("maturity"), boost::any_cast<int>(static_cast<int>(maturity)));
			return nm;
		}

		inline static void GetKeys(const Name& nm, std::string& cntryCode, \
			Maturity::MaturityType& maturity)
		{
			Name::KeyMapType keys = nm.GetKeyMap();
			auto i = keys.find("country");
			cntryCode = boost::any_cast<std::string>(i->second);
			auto j = keys.find("maturity");
			maturity = static_cast<Maturity::MaturityType>(boost::any_cast<int>(j->second));
		}

		/// get the domestic country
		virtual const Country& GetCountry() const = 0;

		/// get the maturity type
		virtual Maturity::MaturityType GetMaturityType() const = 0; 

		/// return type of rate. Influenced by Xignite rate API.
		virtual const std::string& GetRateType() const = 0;

		/// get the domestic country
		virtual void SetCountry(const Country& cntry) = 0;

		virtual void SetMaturityType(Maturity::MaturityType type)  = 0; 

		virtual void SetRateType(const std::string& type) = 0;

	protected:

		/// you should know the derived type if you are deleting.
		virtual ~IIR() 
		{
		}  
	};

} /* namespace derivative */

#endif /* _DERIVATIVE_IIR_H_ */
