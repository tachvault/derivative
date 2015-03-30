/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_IOBJECT_H_
#define _DERIVATIVE_IOBJECT_H_

#include <memory>

#if defined ENTITYMGMT_EXPORTS
#define ENTITY_MGMT_DLL_API __declspec(dllexport)
#else
#define ENTITY_MGMT_DLL_API __declspec(dllimport)
#endif

namespace derivative
{
	/// forward declare Name
	class Name;

	class ENTITY_MGMT_DLL_API IObject
	{

	public:	

		// Return name
		virtual const Name& GetName() = 0;

	};

} /* namespace derivative */

#endif /* _DERIVATIVE_IOBJECT_H_ */
