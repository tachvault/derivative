/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#include "CSV2Array.hpp"

namespace derivative
{
	std::string make_string(const char* str)
	{
		std::string result(str);
		return result;
	}

	/// Strip leading and trailing spaces.
	std::string make_string_strip_spaces(const char* str)
	{
		int len = std::strlen(str);
		char* newstr = new char[len+1];
		int i = 0;
		while ((str[i]==' ')&&(str[i]!=0)) i++;
		int j = len-1;
		while ((j>i)&&(str[j]==' ')) j--;
		int k = 0;
		for(;i<=j;i++) 
		{ 
			newstr[k] = str[i];
			k++; 
		}
		newstr[k] = 0;
		std::string result(newstr);
		delete[] newstr;
		return result;
	}
}
