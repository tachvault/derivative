/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_IEXCHANGERATE_H_
#define _DERIVATIVE_IEXCHANGERATE_H_

#include "IAsset.hpp"
#include "Global.hpp"
#include "Currency.hpp"

namespace derivative
{
	/// Interface for ExchangeRate class
	/// client modules (such as pricing engine) should
	/// be able to use the interface without knowing the 
	/// details of ExchangeRate concrete class itself
	class IExchangeRate: public virtual IAsset
	{

	public:

		enum {TYPEID = INTERFACE_EXCHANGERATE_TYPE};

		static Name ConstructName(const std::string& domestic,const std::string& foreign)
		{
			Name nm(TYPEID, std::hash<std::string>()(domestic + foreign));
			nm.AppendKey(string("domestic"), boost::any_cast<string>(domestic));
			nm.AppendKey(string("foreign"), boost::any_cast<string>(foreign));
			return nm;
		}

		inline static void GetKeys(const Name& nm, std::string& domestic, std::string& foreign)
		{
			Name::KeyMapType keys = nm.GetKeyMap();
			auto i = keys.find("domestic");
			domestic = boost::any_cast<std::string>(i->second);
			auto j = keys.find("foreign");
			foreign = boost::any_cast<std::string>(j->second);
		}

		/// get the domestic currency
		virtual const Currency& GetDomesticCurrency() const = 0;

		/// get the foreign currency
		virtual const Currency& GetForeignCurrency() const = 0;

		/// get the domestic currency
		virtual void SetDomesticCurrency(const Currency& curr) = 0;

		/// get the foreign currency
		virtual void SetForeignCurrency(const Currency&  curr)  = 0;

	protected:

		/// you should know the derived type if you are deleting.
		virtual ~IExchangeRate() 
		{
		}  
	};

} /* namespace derivative */

#endif /* _DERIVATIVE_IEXCHANGERATE_H_ */
