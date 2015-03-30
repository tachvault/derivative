/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_IASSET_H_
#define _DERIVATIVE_IASSET_H_

#include <string>
#include "ClassType.hpp"
#include "IObject.hpp"

namespace derivative
{
	/// Base interface for any assets
	class  IAsset: virtual public IObject
	{
	public:

		enum {TYPEID = INTERFACE_ASSET_TYPE};

		/// return the symbol of the asset
		/// Ex: Apple stock AAPL
		virtual const std::string& GetSymbol() const = 0;
		
		virtual double GetImpliedVol() const = 0;

		virtual double GetHistVol() const = 0;

		virtual void SetSymbol(const std::string& sym) = 0;

	protected:

		/// you should know the derived type if you are deleting.
		virtual ~IAsset() 
		{
		}  
	};

} /* namespace derivative */

#endif /* _DERIVATIVE_IASSET_H_ */
