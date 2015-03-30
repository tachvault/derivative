/*
Copyright (c) 2013-2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_IASSETVALUE_H_
#define _DERIVATIVE_IASSETVALUE_H_

#include "ClassType.hpp"
#include "Global.hpp"
#include "IObject.hpp"

namespace derivative
{
	class DeterministicAssetVol;
	class IAsset;

	/// Base interface for any asset value
	class  IAssetValue : virtual public IObject
	{

	public:

		enum {TYPEID = INTERFACE_ASSETVALUE_TYPE};

		/// Get last reported price
		virtual double GetTradePrice() const = 0;

		virtual double GetDivYield() const = 0;

		virtual std::shared_ptr<IAsset> GetAsset() const = 0;

	protected:

		/// you should know the derived type if you are deleting.
		virtual ~IAssetValue() 
		{
		}  
	};

} /* namespace derivative */

#endif /* _DERIVATIVE_IASSETVALUE_H_ */
