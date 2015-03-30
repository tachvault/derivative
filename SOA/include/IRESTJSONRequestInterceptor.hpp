/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_IRESTJSONREQUESTINTERCEPTOR_H_
#define _DERIVATIVE_IRESTJSONREQUESTINTERCEPTOR_H_
#pragma once

#include "IRequestInterceptor.hpp"
#include "IObject.hpp"
#include "Name.hpp"
#include "Global.hpp"

namespace derivative
{
	/// IRequestInterceptor interface for handlers that intercept
	/// service request from clients. (Ex: REST, TCP, SOAP).
	class IRESTJSONRequestInterceptor : virtual public IObject,
		                            virtual public IRequestInterceptor
	{
	public:

		enum { TYPEID = INTERFACE_RESTJSONREQUESTINTERCEPTOR_TYPE };

		static Name ConstructName(const std::string& address, unsigned int port, const std::string& path)
		{
			Name nm(TYPEID, std::hash<std::string>()(address + to_string(port) + path));
			nm.AppendKey(string("address"), boost::any_cast<string>(address));
			nm.AppendKey(string("port"), boost::any_cast<unsigned int>(port));
			nm.AppendKey(string("path"), boost::any_cast<string>(path));
			return nm;
		}

		inline static void GetKeys(const Name& nm, std::string& address, unsigned int& port, std::string& path)
		{
			Name::KeyMapType keys = nm.GetKeyMap();
			auto i = keys.find("address");
			address = boost::any_cast<std::string>(i->second);
			auto j = keys.find("port");
			port = boost::any_cast<unsigned int>(j->second);
			auto k = keys.find("path");
			path = boost::any_cast<string>(k->second);
		}

	protected:

		/// you should know the derived type if you are deleting.
		virtual ~IRESTJSONRequestInterceptor() 
		{
		}  
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_IRESTJSONREQUESTINTERCEPTOR_H_ */
