/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_QFVISITOR_H_
#define _DERIVATIVE_QFVISITOR_H_

#include "MSWarnings.hpp"
#include <blitz/array.h>

#if defined _WIN32 || defined __CYGWIN__
#ifdef PRIMARYASSET_EXT_EXPORTS
#ifdef __GNUC__
#define PRIMARYASSET_EXT_API __attribute__ ((dllexport))
#else
#define PRIMARYASSET_EXT_API __declspec(dllexport)
#define EXPIMP_TEMPLATE
#endif
#else
#ifdef __GNUC__
#define PRIMARYASSET_EXT_API __attribute__ ((dllimport))
#else
#define PRIMARYASSET_EXT_API __declspec(dllimport)
#define EXPIMP_TEMPLATE extern
#endif
#endif
#define PRIMARYASSET_EXT_LOCAL
#else
#if __GNUC__ >= 4
#define PRIMARYASSET_EXT_API __attribute__ ((visibility ("default")))
#define PRIMARYASSET_EXT_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define PRIMARYASSET_EXT_API
#define PRIMARYASSET_EXT_LOCAL
#endif
#endif

namespace derivative 
{
	using blitz::Array;
	using blitz::firstDim;

	class QFVisitor;
	class QFNestedVisitor;

	class PRIMARYASSET_EXT_API QFVisitable {
	public:
		virtual void accept(QFVisitor& visitor) const = 0;
	};

	class PRIMARYASSET_EXT_API QFNestedVisitable {
	public:
		virtual void accept(QFNestedVisitor& visitor) const = 0;
		virtual const std::string& name() const = 0;
	};

	class PRIMARYASSET_EXT_API QFVisitor {
	public:
		virtual void visit(const QFVisitable& visitable) = 0;
	};

	class PRIMARYASSET_EXT_API QFNestedVisitor {
	public:
		virtual void visit(const QFNestedVisitable& visitable) = 0;    
		virtual void visit(const std::string& name,const QFNestedVisitable& value) = 0; 
		// Visit functions for recognised data types.
		virtual void visit(double x) = 0;    
		virtual void visit(size_t x) = 0;    
		virtual void visit(const Array<double,1>& x) = 0;    
		virtual void visit(const Array<double,2>& x) = 0;    
		virtual void visit(const std::string& name,double value) = 0; 
		virtual void visit(const std::string& name,size_t value) = 0; 
		virtual void visit(const std::string& name,const Array<double,1>& value) = 0; 
		virtual void visit(const std::string& name,const Array<double,2>& value) = 0; 
	};

}

/* namespace derivative */

#endif /* _DERIVATIVE_QFVISITOR_H_ */
