/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#include "QFUtil.hpp"

namespace derivative
{

	bool verify_compare(double x,double y,double eps)
	{
		double ax = (x>0.0) ? x : -x;
		if (ax>eps) eps *= ax;
		x -= y;
		x = (x>0.0) ? x : -x;
		return (x<eps);       
	}

	void load_file(std::string& s, std::istream& is)
	{
		s.erase();
		if(is.bad()) return;
		char c;
		while(is.get(c)) 
		{
			if(s.capacity() == s.size())  s.reserve(s.capacity() * 3);
			s.append(1, c); 
		}
	}
}
