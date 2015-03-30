/*
Copyright (c) 2015, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_RESTJSONREQUESTINTERCEPTOR_H_
#define _DERIVATIVE_RESTJSONREQUESTINTERCEPTOR_H_

#pragma once

#include "cpprest/basic_types.h"
#ifdef _MS_WINDOWS
#define NOMINMAX
#include <Windows.h>
#else
# include <sys/time.h>
#endif

#include "cpprest/json.h"
#include "cpprest/http_listener.h"
#include "cpprest/uri.h"
#include "cpprest/asyncrt_utils.h"

#include "Name.hpp"
#include "IMake.hpp"
#include "IRESTJSONRequestInterceptor.hpp"

#if defined _WIN32 || defined __CYGWIN__
#ifdef WEBSERVICE_EXPORTS
#ifdef __GNUC__
#define WEBSERVICE_DLL_API __attribute__ ((dllexport))
#else
#define WEBSERVICE_DLL_API __declspec(dllexport)
#endif
#else
#ifdef __GNUC__
#define WEBSERVICE_DLL_API __attribute__ ((dllimport))
#else
#define WEBSERVICE_DLL_API __declspec(dllimport)
#endif
#endif
#define WEBSERVICE_DLL_LOCAL
#else
#if __GNUC__ >= 4
#define WEBSERVICE_DLL_API __attribute__ ((visibility ("default")))
#define WEBSERVICE_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define WEBSERVICE_DLL_API
#define WEBSERVICE_DLL_LOCAL
#endif
#endif

using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;

namespace derivative
{
	class WEBSERVICE_DLL_API RESTJSONRequestInterceptor : virtual public IRESTJSONRequestInterceptor,
		virtual public IMake

	{
	public:

		enum { TYPEID = CLASS_RESTINTERCEPTOR_TYPE };

		virtual const Name& GetName()
		{
			return m_name;
		}

		RESTJSONRequestInterceptor(const Exemplar &ex);

		RESTJSONRequestInterceptor(const Name& nm);

		/// virtual destructor
		virtual ~RESTJSONRequestInterceptor();

		/// IMake functions
		virtual std::shared_ptr<IMake> Make(const Name &nm);

		virtual std::shared_ptr<IMake> Make(const Name &nm, const std::deque<boost::any>& agrs);

		/// start the service implements from IRequestInterceptor
		virtual void StartInterceptor();

		/// shutdown the service implements from IRequestInterceptor
		virtual void Shutdown();

	private:

		/// disallow copy and assignment
		DISALLOW_COPY_AND_ASSIGN(RESTJSONRequestInterceptor);

		/// A GET of the dealer resource produces a list of existing tables.
		void handle_get(http_request message);

		/// A POST of the dealer resource creates a new table and returns a resource for
		/// that table.
		void handle_put(http_request message);

		/// A PUT to a table resource makes a card request (hit / stay).
		void handle_post(http_request message);

		/// A DELETE of the player resource leaves the table.
		void handle_delete(http_request message);

	private:

		Name m_name;

		http_listener m_listener;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_RESTJSONREQUESTINTERCEPTOR_H_ */
