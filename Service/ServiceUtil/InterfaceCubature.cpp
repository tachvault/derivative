/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#include "InterfaceCubature.hpp"
#include "cubature.h"

namespace derivative
{
	std::function<void (const Array<double,1>&,Array<double,1>&)> global_function_for_cubature;

	void integrand_f(unsigned ndim, const double *x, void *, unsigned fdim, double *fval)
	{
		int i;
		Array<double,1> in(ndim);
		for (i=0;i<ndim;i++) in(i) = x[i];
		Array<double,1> out(fval, blitz::shape(fdim), blitz::neverDeleteData);
		global_function_for_cubature(in,out);
	}

	int cubature(std::function<void (const Array<double,1>&,Array<double,1>&)> f,
		const Array<double,1>& xmin, 
		const Array<double,1>& xmax, 
		unsigned maxEval, 
		double reqAbsError, 
		double reqRelError, 
		Array<double,1>& val, 
		Array<double,1>& err)
	{
		global_function_for_cubature = f;
		int retval = adapt_integrate(val.extent(firstDim),integrand_f,NULL,xmin.extent(firstDim),xmin.data(),xmax.data(),
			maxEval,reqAbsError,reqRelError,val.data(),err.data());
		return retval;
	}
}
