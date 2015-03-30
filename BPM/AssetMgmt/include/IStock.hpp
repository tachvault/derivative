/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_ISTOCK_H_
#define _DERIVATIVE_ISTOCK_H_

#include <memory>
#include "IPrimitiveSecurity.hpp"
#include "Country.hpp"

namespace derivative
{
	class Exchange;

	/// Interface for Stock class
	/// client modules (such as pricing engine) should
	/// be able to use the interface without knowing the 
	/// details of Stock concrete class itself
	class IStock: public virtual IPrimitiveSecurity
	{

	public:

		enum {TYPEID = INTERFACE_STOCK_TYPE};

		static Name ConstructName(const std::string& symbol)
		{
			Name nm(TYPEID, std::hash<std::string>()(symbol));
			nm.AppendKey(string("symbol"), boost::any_cast<string>(symbol));
			return nm;
		}

		inline static void GetKeys(const Name& nm, std::string& symbol)
		{
			Name::KeyMapType keys = nm.GetKeyMap();
			auto i = keys.find("symbol");
			symbol = boost::any_cast<std::string>(i->second);
		}
		
		virtual void SetName(const Name& nm) = 0;

		virtual const Exchange& GetExchange() const  = 0;

		virtual const Country& GetCountry() const  = 0;

		virtual void SetExchange(const Exchange& ex) = 0;

		virtual void SetCountry(const Country& cntry) = 0;

		virtual void SetImpliedVol(double vol) = 0;

		virtual void SetHistVol(double vol) = 0;

	protected:

		/// you should know the derived type if you are deleting.
		virtual ~IStock() 
		{
		}  
	};

} /* namespace derivative */

#endif /* _DERIVATIVE_ISTOCK_H_ */
