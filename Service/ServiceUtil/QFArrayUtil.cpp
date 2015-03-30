/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/
#include <algorithm>    // std::max
#include "QFArrayUtil.hpp"

#undef max

namespace derivative
{

	double const_func(const Array<double,1>& x)
	{
		return 1.0;
	}

	/** Assuming that T is a vector of doubles sorted in ascending order, find the index i such that
	T(i) < xi <= T(i+1). */
	int find_segment(double xi,const Array<double,1>& T) 
	{
		if (xi<T(0)-std::max(1e-7,std::abs(T(0)*1e-7))) throw std::out_of_range("cannot extrapolate");
		for (int i=1;i<T.extent(firstDim);i++) if (xi<=T(i)+std::max(1e-7,std::abs(T(i)*1e-7))) return i-1;
		throw std::out_of_range("cannot extrapolate");
	}

	void CSVwrite(std::ostream& os,const Array<double,1>& v)
	{
		int i;
		for (i=0;i<v.extent(firstDim)-1;i++) os << v(i) << ',';
		os << v(v.extent(firstDim)-1) << std::endl;
	}

	void CSVwrite(std::ostream& os,const Array<double,2>& A)
	{
		int i,j;
		for (i=0;i<A.extent(firstDim);i++)
		{
			for (j=0;j<A.extent(secondDim)-1;j++) os << A(i,j) << ',';
			os << A(i,A.extent(secondDim)-1) << std::endl; 
		}
	}
}

#define max(a,b)  ((a > b) ? a : b)

