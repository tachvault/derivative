/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_DETERMINISTICVOLMEDIATOR_H_
#define _DERIVATIVE_DETERMINISTICVOLMEDIATOR_H_

#include "ConstVol.hpp"
#include "ExponentialVol.hpp"

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
	class ASSET_PROPERTIES_API DeterministicVolMediator
	{
	public:

		enum { TYPEID = CLASS_DETERMINISTICVOLMEDIATOR_TYPE};

		enum type { FLAT, EXPONENTIAL };
		
		static double volproduct_DeterministicAssetVolDiff(double t,double dt,
			const DeterministicAssetVol& xv,
			const DeterministicAssetVol& v1,
			const DeterministicAssetVol& v2);  
		
		static double volproduct_ConstVol(double t,double dt,const Array<double,1>& lvl,const DeterministicAssetVol& v2);  

		static double volproduct_ConstVol(double t, double dt, const Array<double, 1>& lvl, const DeterministicAssetVol& v2, Array<double, 1>& temp);
		
		static double volproduct_ExponentialVol(double t,double dt,
			const Array<double,1>& lvl,
			const Array<double,1>& decay,
			const ExponentialVol& v1,
			const DeterministicAssetVol& v2);
		
		static double bondvolproduct_ConstVol(double t,double dt,double bondmat,const Array<double,1>& lvl,const DeterministicAssetVol& xv);
		
		static double bondvolproduct_ExponentialVol(double t,double dt,double bondmat,const Array<double,1>& lvl,const Array<double,1>& decay,const ExponentialVol& this_vol,const DeterministicAssetVol& xv);
		
		static double bondbondvolproduct_ConstVol(double t,double dt,double T1,double T2,const Array<double,1>& lvl,const ConstVol& v1,const DeterministicAssetVol& xv);
		
		static double bondbondvolproduct_ExponentialVol(double t,double dt,double T1,double T2,const Array<double,1>& lvl,const Array<double,1>& decay,const ExponentialVol& this_vol,const DeterministicAssetVol& xv);
		
		static Array<double,1> z_bondintegral_ExponentialVol(double t,double dt,double T,const Array<double,1>& lvl,const Array<double,1>& decay,const ExponentialVol& v1,const DeterministicAssetVol& xmodel);
		
		static Array<double,1> z_volintegral_ExponentialVol(double t,double dt,double T,const Array<double,1>& lvl,const Array<double,1>& decay,const ExponentialVol& v1,const DeterministicAssetVol& fxvol);
		
		static double FwdFXexponential_ExponentialVol(double t,double dt,const Array<double,1>& dWj,const Array<double,1>& dZj0,const Array<double,1>& dZjk,const Array<double,1>& nu,const Array<double,1>& a,\
			const ExponentialVol* this_vol, const std::shared_ptr<DeterministicAssetVol>& xv, const std::shared_ptr<DeterministicAssetVol>& fxvol);
		
		static double covar_ExponentialVol(int j,double t,double dt,double lvl,double decay,const ExponentialVol& this_vol,const DeterministicAssetVol& xv);
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_DETERMINISTICVOLMEDIATOR_H_ */