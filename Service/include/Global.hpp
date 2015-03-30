/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_GLOBAL_H_
#define _DERIVATIVE_GLOBAL_H_

#include <gflags/gflags.h>
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#include <iostream>

#include "boost/date_time/posix_time/posix_time.hpp" 
#include "boost/date_time/gregorian/gregorian.hpp"

#include "ClassType.hpp"

#if defined ENTITYMGMT_EXPORTS
#define ENTITY_MGMT_DLL_API __declspec(dllexport)
#else
#define ENTITY_MGMT_DLL_API __declspec(dllimport)
#endif

// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

/// define namespace alias
namespace dd = boost::gregorian;
namespace pt = boost::posix_time;

/// import frequently used std types
using std::cout;
using std::endl;
using std::istringstream;
using std::string;
using std::to_string;
using std::size_t;
using std::shared_ptr;
using std::make_shared;
using std::unique_ptr;
using std::make_pair;
using std::dynamic_pointer_cast;

using std::vector;
using std::deque;
using std::map;


/// type definitions
typedef unsigned short grpType;
typedef unsigned short msgType;
typedef unsigned short ushort;

namespace derivative
{
	class ENTITY_MGMT_DLL_API Exemplar
	{
	public:

		enum {TYPEID = CLASS_EXEMPLAR_TYPE};
	};

} /* namespace derivative */

#endif