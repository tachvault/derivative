/// Copyright (C) Nathan Muruganantha 2013 - 2014

#ifndef _DERIVATIVE_VOLATILITYSMILE_H_
#define _DERIVATIVE_VOLATILITYSMILE_H_

#include <memory>
#include <string>
#include <algorithm>
#include <blitz/array.h>

#include "ClassType.hpp"
#include "Global.hpp"
 
#if defined _WIN32 || defined __CYGWIN__
#ifdef ASSET_PROPERTIES_EXPORTS
#ifdef __GNUC__
#define ASSET_PROPERTIES_API __attribute__ ((dllexport))
#else
#define ASSET_PROPERTIES_API __declspec(dllexport)
#endif
#else
#ifdef __GNUC__
#define ASSET_PROPERTIES_API __attribute__ ((dllimport))
#else
#define ASSET_PROPERTIES_API __declspec(dllimport)
#endif
#endif
#define ASSET_PROPERTIES_LOCAL
#else
#if __GNUC__ >= 4
#define ASSET_PROPERTIES_API __attribute__ ((visibility ("default")))
#define ASSET_PROPERTIES_LOCAL  __attribute__ ((visibility ("hidden")))
#else
#define ASSET_PROPERTIES_API
#define ASSET_PROPERTIES_LOCAL
#endif
#endif

namespace derivative
{
	using blitz::Array;

    class ASSET_PROPERTIES_API VolatilitySmile
	{
      public:
       	  
		enum {TYPEID = CLASS_VOLATILITYSMILE_TYPE};

		VolatilitySmile(const Array<int, 1>& K, const Array<double, 1>& vol);

		///destructor
		~VolatilitySmile()
		{
		}

		const Array<int, 1>& GetStrikes() const
		{
			return m_strikes;
		}

		const Array<double, 1>& GetVols() const
		{
			return m_vols;
		}
		
	private:
		
		const Array<int, 1>& m_strikes;

		const Array<double, 1>& m_vols;

		/// disallow the copy constructor and operator= functions
		DISALLOW_COPY_AND_ASSIGN(VolatilitySmile);
    };	
}

/* namespace derivative */
#endif /* _DERIVATIVE_VOLATILITYSMILE_H_ */