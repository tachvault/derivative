/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/
#include "StringForm.hpp"

namespace derivative
{
	Bound_StringForm::Bound_StringForm(const StringForm& ff,double v) 
	{ 
		int wdt = ff.wdt;
		if (v>=0.0) 
		{
			s << ' ';
			wdt--; 
		}
		s.precision(ff.prc);
		s.setf(ff.fmt,std::ios_base::floatfield);
		s.setf(std::ios_base::left);
		s.width(wdt);
		s.fill(ff.fl);
		s << v;
	}

	Bound_StringForm::Bound_StringForm(const StringForm& ff,const char* c) 
	{ 
		int i;
		if (ff.str_sch == StringForm::CENTER) 
		{
			const char* p = c;
			int len = 0;
			while (*p++) len++;
			int rest = ff.wdt - len;
			int  add = rest % 2;
			rest /= 2;
			for (i=0;i<rest;i++) s << ff.fl;
			s << c;
			for (i=0;i<rest+add;i++) s << ff.fl; 
		}
		else 
		{
			s.width(ff.wdt);
			s.fill(ff.fl);
			s.setf((ff.str_sch==StringForm::RIGHT) ? std::ios_base::right : std::ios_base::left,std::ios_base::adjustfield);
			s << c; 
		}
	}

	std::ostream& operator<<(std::ostream& os,const Bound_StringForm& bf)
	{
		return os << bf.s.str().c_str();
	}
}
