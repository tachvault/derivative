/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_IIBOR_H_
#define _DERIVATIVE_IIBOR_H_

#include "IObject.hpp"
#include "Global.hpp"
#include "Currency.hpp"
#include "Maturity.hpp"


namespace derivative
{
	/// Interface for Interbank Offered Rate such as LIBOR rate
	/// callers modules (such as pricing engine) should
	/// be able to use the interface without knowing the 
	/// details of SpotRate concrete class itself
	class IIBOR: public virtual IObject
	{

	public:

		enum {TYPEID = INTERFACE_IBOR_TYPE};

		static Name ConstructName(const std::string& currCode, Maturity::MaturityType& maturity)
		{
			Name nm(TYPEID, std::hash<std::string>()(currCode + std::to_string(maturity)));
			nm.AppendKey(string("currency"), boost::any_cast<string>(currCode));
			nm.AppendKey(string("maturity"), boost::any_cast<int>(static_cast<int>(maturity)));
			return nm;
		}

		inline static void GetKeys(const Name& nm, std::string& currCode, Maturity::MaturityType& maturity)
		{
			Name::KeyMapType keys = nm.GetKeyMap();
			auto i = keys.find("currency");
			currCode = boost::any_cast<std::string>(i->second);
			auto j = keys.find("maturity");
			maturity = static_cast<Maturity::MaturityType>(boost::any_cast<int>(j->second));
		}

		/// get the domestic currency
		virtual const Currency& GetCurrency() const = 0;

		/// get the maturity type
		virtual Maturity::MaturityType GetMaturityType() const = 0; 

		/// return type of rate. Influenced by Xignite rate API.
		virtual const std::string& GetRateType() const = 0;

		virtual void SetRateType(const std::string& type) = 0;

		/// get the domestic currency
		virtual void SetCurrency(const Currency& curr) = 0;

		virtual void SetMaturityType(Maturity::MaturityType type)  = 0; 

	protected:

		/// you should know the derived type if you are deleting.
		virtual ~IIBOR() 
		{
		}  
	};

} /* namespace derivative */

#endif /* _DERIVATIVE_IIBOR_H_ */
