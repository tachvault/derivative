/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_INTERFACECUBATURE_H_
#define _DERIVATIVE_INTERFACECUBATURE_H_

#include "MSWarnings.hpp"
#include <boost/function.hpp>
#include <blitz/array.h>

#if defined SERVICEUTIL_EXPORTS
#define SERVICE_UTIL_DLL_API __declspec(dllexport)
#else
#define SERVICE_UTIL_DLL_API __declspec(dllimport)
#endif

namespace derivative
{ 
	using blitz::Array;
	using blitz::firstDim;

	int SERVICE_UTIL_DLL_API cubature(std::function<void (const Array<double,1>&,Array<double,1>&)> f,
		const Array<double,1>& xmin, 
		const Array<double,1>& xmax, 
		unsigned maxEval, 
		double reqAbsError, 
		double reqRelError, 
		Array<double,1>& val, 
		Array<double,1>& err);
}

/* namespace derivative */

#endif /* _DERIVATIVE_INTERFACECLAPACK_H_ */

