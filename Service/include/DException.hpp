/*
Copyright (c) 2013, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_DEXCEPTION_H_
#define _DERIVATIVE_DEXCEPTION_H_

#include <exception>
#include <string>

using std::exception;

namespace derivative
{
	/// Specialized exception class for EntityManager component
	class  RegistryException : public std::logic_error
	{

	public:
		RegistryException(const std::string& msg)
			: std::logic_error(msg) 
		{
		}
	};

	/// Specialized exception class for Data Source component
	class  DataSourceException : public std::logic_error
	{

	public:
		DataSourceException(const std::string& msg)
			: std::logic_error(msg) 
		{
		}
	};

	/// Specialized exception class for Financial Utility component
	class  FinUtilException : public std::logic_error
	{

	public:
		FinUtilException(const std::string& msg)
			: std::logic_error(msg) 
		{
		}
	};

	/// Specialized exception class for MySQL Data Source component
	class  MySQLSrcException : public std::logic_error
	{

	public:
		MySQLSrcException(const std::string& msg)
			: std::logic_error(msg) 
		{
		}
	};

	/// Specialized exception class for Yahoo Data Source component
	class  YahooSrcException : public std::logic_error
	{

	public:
		YahooSrcException(const std::string& msg)
			:logic_error(msg) 
		{
		}
	};

	/// Specialized exception class for Xignite Data Source component
	class  XigniteSrcException : public std::logic_error
	{

	public:
		XigniteSrcException(const std::string& msg)
			:logic_error(msg)
		{
		}
	};

	class QFmath_error : public std::logic_error 
	{
	public:
		inline QFmath_error(const std::string& what_arg) : std::logic_error(what_arg) { };
	};

	class DegeneratePolynomial : public QFmath_error
	{
	public:
		inline DegeneratePolynomial() : QFmath_error("Degenerate polynomial") { };
	};

} /* namespace derivative */

#endif /*_DERIVATIVE_EXCEPTION_H_ */
