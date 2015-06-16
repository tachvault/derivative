/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial versions: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
Initial versions: Copyright (c) 2008, Frances Y. Kuo and Stephen Joe
*/

#ifndef _DERIVATIVE_QFQUASIRANDOM_H_
#define _DERIVATIVE_QFQUASIRANDOM_H_

#include <mutex>
#include <boost/shared_ptr.hpp>
#include <boost/math/distributions/normal.hpp>
#include <blitz/array.h>

#include "ClassType.hpp"
#include "SpinLock.hpp"

#if defined _WIN32 || defined __CYGWIN__
  #ifdef SERVICEUTIL_EXPORTS
    #ifdef __GNUC__
      #define SERVICE_UTIL_DLL_API __attribute__ ((dllexport))
    #else
      #define SERVICE_UTIL_DLL_API __declspec(dllexport)
    #endif
  #else
    #ifdef __GNUC__
      #define SERVICE_UTIL_DLL_API __attribute__ ((dllimport))
    #else
      #define SERVICE_UTIL_DLL_API __declspec(dllimport)
    #endif
  #endif
  #define ESERVICE_UTIL_DLL_LOCAL
#else
  #if __GNUC__ >= 4
    #define SERVICE_UTIL_DLL_API __attribute__ ((visibility ("default")))
    #define SERVICE_UTIL_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define SERVICE_UTIL_DLL_API
    #define SERVICE_UTIL_DLL_LOCAL
  #endif
#endif

namespace derivative 
{
	using blitz::Array;
	using blitz::firstDim;
	using blitz::secondDim;

	const char default_direction_number_file[] =  "new-joe-kuo-6.21201.dat";

	class SERVICE_UTIL_DLL_API Sobol
	{

	public:

		enum {TYPEID = CLASS_SOBOL_TYPE};

		Sobol(int dimension,size_t maximum_number_of_points,const char* direction_number_file = default_direction_number_file);
		Array<double,1>& random();
		Array<double,1>& random(const Array<double,1>& shift);
		void reset();
		inline size_t number_of_points() const { return maxN; };
		inline void set_number_of_points(size_t maximum_number_of_points,const char* direction_number_file = default_direction_number_file) 
		{ 
			maxN = maximum_number_of_points; initialise(direction_number_file); 
		};

	private:
		Array<double,1>                current_point;
		Array<unsigned,1>                   currentX;
		std::shared_ptr<Array<unsigned,2> >      V;
		size_t                                  maxN;
		double                               scaling;
		unsigned                            currentC;
		unsigned                            currenti;
		int                                      dim;
		void initialise(const char* direction_number_file = default_direction_number_file);

	};

	class SERVICE_UTIL_DLL_API SobolArray 
	{

	public:

		enum {TYPEID = CLASS_SOBOLARRAY_TYPE};

		inline SobolArray(int rows,int cols,size_t maximum_number_of_points,const char* direction_number_file = default_direction_number_file) 
			: contents(rows,cols),rng(rows*cols,maximum_number_of_points,direction_number_file) 
		{ };
		/// Returns an array of draws from Sobol sequence.
		inline Array<double,2>& random();

	private:
		Array<double,2>    contents;
		Sobol                   rng;
	};

	inline Array<double,2>& SobolArray::random()
	{
		int i,j,k;
		Array<double,1> rnd(rng.random());
		k = 0;
		for (i=0;i<contents.extent(blitz::firstDim);i++) 
		{
			for (j=0;j<contents.extent(blitz::secondDim);j++) 
			{
				contents(i,j) = rnd(k);
				k++; 
			}
		}
		return contents;
	}

	class SERVICE_UTIL_DLL_API SobolArrayNormal 
	{	
	public:

		enum {TYPEID = CLASS_SOBOLARRAYNORMAL_TYPEID};

		inline SobolArrayNormal(int rows,int cols,size_t maximum_number_of_points,const char* direction_number_file = default_direction_number_file) 
			: contents(rows,cols),rng(rows*cols,maximum_number_of_points,direction_number_file),n(rows*cols),epsilon(1e-64)
		{  };
		/// Returns an array of draws from Sobol sequence.
		inline Array<double,2>& random();
		inline void random_reference(Array<double, 2>& cnt);
		inline Array<double,2>& random(Array<double,2>& shift);
		inline void reset() { rng.reset();
		};
		inline size_t number_of_points() const 
		{
			return rng.number_of_points(); 
		};
		inline void set_number_of_points(size_t maximum_number_of_points,const char* direction_number_file = default_direction_number_file) 
		{ 
			rng.set_number_of_points(maximum_number_of_points,direction_number_file);
		};

	private:
		Array<double,2>    contents;
		Sobol                   rng;
		boost::math::normal  normal;
		int                       n;
		const double        epsilon;
		mutable SpinLock m_lock;
	};

	inline Array<double,2>& SobolArrayNormal::random()
	{
		int i,j,k;
		Array<double,1> rnd(rng.random());
		k = 0;
		for (i=0;i<contents.extent(blitz::firstDim);i++)
		{
			for (j=0;j<contents.extent(blitz::secondDim);j++)
			{
				double x = rnd(k);
				if (x==0) x = epsilon;
				contents(i,j) = boost::math::quantile(normal,x);
				k++; 
			}
		}
		return contents;
	}

	void SobolArrayNormal::random_reference(Array<double, 2>& cnt)
	{
		std::lock_guard<SpinLock> lock(m_lock);
		if ((contents.extent(firstDim) != cnt.extent(firstDim)) || (contents.extent(secondDim) != cnt.extent(secondDim)))
		{
			cnt.resize(contents.extent(firstDim), contents.extent(secondDim));
		}

		int i, j, k;
		Array<double, 1> rnd(rng.random());
		k = 0;
		for (i = 0; i<contents.extent(blitz::firstDim); i++)
		{
			for (j = 0; j<contents.extent(blitz::secondDim); j++)
			{
				double x = rnd(k);
				if (x == 0) x = epsilon;
				cnt(i, j) = boost::math::quantile(normal, x);
				k++;
			}
		}
	}

	inline Array<double,2>& SobolArrayNormal::random(Array<double,2>& shift)
	{
		int i,j,k;
		Array<double,1> tmp(shift.dataFirst(),n,blitz::neverDeleteData);
		Array<double,1> rnd(rng.random(tmp));
		k = 0;
		for (i=0;i<contents.extent(blitz::firstDim);i++)
		{
			for (j=0;j<contents.extent(blitz::secondDim);j++)
			{
				double x = rnd(k);
				if (x==0) x = epsilon;
				contents(i,j) = boost::math::quantile(normal,x);
				k++;
			}
		}
		return contents;
	}

}
/* namespace derivative */

#endif /* _DERIVATIVE_QFQUASIRANDOM_H_ */
