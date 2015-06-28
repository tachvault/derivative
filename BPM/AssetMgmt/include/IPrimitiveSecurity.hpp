/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_IPRIMITIVESECURITY_H_
#define _DERIVATIVE_IPRIMITIVESECURITY_H_

#include "IAsset.hpp"

namespace derivative
{
	/// forward declare currency;
	class Currency;
	class Exchange;

	/// PrimitiveSecurity generalizes any financial rudumentory securities such as
	/// stocks, bonds, money market funds etc.
	class  IPrimitiveSecurity : virtual public IAsset
	{

	public:

		enum {TYPEID = INTERFACE_PRIMITIVESECURITY_TYPE};

		/// return the domestic currency for the asset
		virtual const Currency& GetDomesticCurrency() const = 0;

		virtual const std::string& GetDescription() const = 0;
		
		virtual const Exchange& GetExchange() const = 0;

		/// Setter methods. Used by DAOs.
		/// set the domestic currency for the asset
		virtual void SetDomesticCurrency(const Currency&  curr) = 0;

		virtual void SetDescription(const std::string&  des) = 0;

	protected:

		/// you should know the derived type if you are deleting.
		virtual ~IPrimitiveSecurity() 
		{
		}  
	};

} /* namespace derivative */

#endif /* _DERIVATIVE_IPRIMITIVEASSET_H_ */
