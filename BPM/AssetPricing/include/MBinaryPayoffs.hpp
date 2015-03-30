/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_MBINARYPAYOFFS_H_
#define _DERIVATIVE_MBINARYPAYOFFS_H_

#include "MBinary.hpp"
#include "GaussMarkovWorld.hpp"

#if defined PRICINGENGINE_EXPORTS
#define PRICINGENGINE_DLL_API __declspec(dllexport)
#else
#define PRICINGENGINE_DLL_API __declspec(dllimport)
#endif

namespace derivative
{
	class PRICINGENGINE_DLL_API AssetBinary : public MBinaryPayoff
	{
	public:
		inline AssetBinary(std::shared_ptr<BlackScholesAssetAdapter>& asset,double strike,double now,double mat,const TermStructure& xts);
		inline AssetBinary(const GaussMarkovWorld& world,double strike,double now,double mat,int reportable_asset_index,int callput = 1);
	};

	class BondBinary : public MBinaryPayoff 
	{
	public:
		inline BondBinary(std::shared_ptr<BlackScholesAssetAdapter>& asset,double strike,double now,double mat,const TermStructure& xts);
		inline BondBinary(const GaussMarkovWorld& world,double strike,double now,double mat,int reportable_asset_index,int callput = 1);
	};

	/** European option on a foreign asset, struck in foreign currency (but price expressed in domestic currency).
	Exchange rate is interpreted as a BlackScholesAsset paying a continuous dividend yield. 
	If interest rates are stochastic, volatilities of the asset and the exchange rate must be the volatilities of the forward prices. */
	class ForeignOption : public MBinaryPayoff 
	{
	public:
		inline ForeignOption(std::shared_ptr<BlackScholesAssetAdapter>& asset, \
			std::shared_ptr<BlackScholesAssetAdapter>& fx, \
			double strike, \
			double now, \
			double mat, \
			const TermStructure& xts, \
			bool assetpart = true, \
			int callput = 1);
		inline ForeignOption(const GaussMarkovWorld& world,
			double strike,
			double now,
			double mat,
			int reportable_asset_index,
			int reportable_FX_index,
			bool assetpart = true,
			int callput = 1);
	};

	inline ForeignOption::ForeignOption(std::shared_ptr<BlackScholesAssetAdapter>& asset, std::shared_ptr<BlackScholesAssetAdapter>& fx,\
		double strike,double now,double mat,const TermStructure& xts,bool assetpart,int callput)
		: MBinaryPayoff(xts,2,2,1) 
	{      
		underlying.push_back(asset);
		underlying.push_back(fx);
		timeline = now, mat;
		index    = 0, 1,
			1, 1;      
		alpha    = 1.0;
		if (!assetpart) alpha(0) = 0.0;
		S        = callput;
		A        = 1.0, 0.0;
		a        = strike;
	}

	inline ForeignOption::ForeignOption(const GaussMarkovWorld& world,double strike,double now,double mat,int reportable_asset_index,int reportable_FX_index,bool assetpart,int callput)
		: MBinaryPayoff(*world.get_economies()[0]->initialTS,2,2,1) 
	{      
		timeline = now, mat;
		index    = reportable_asset_index, 1,
			reportable_FX_index, 1;      
		alpha    = 1.0;
		if (!assetpart) alpha(0) = 0.0;
		S        = callput;
		A        = 1.0, 0.0;
		a        = strike;
	}

	inline AssetBinary::AssetBinary(std::shared_ptr<BlackScholesAssetAdapter>& asset, \
		double strike,double now,double mat,const TermStructure& xts)
		: MBinaryPayoff(xts,2,1,1) 
	{      
		underlying.push_back(asset);
		timeline = now, mat;
		index    = 0, 1;      
		alpha    = 1.0;
		S        = 1;
		A        = 1.0;
		a        = strike;
	}

	inline AssetBinary::AssetBinary(const GaussMarkovWorld& world,double strike,double now,double mat,int reportable_asset_index,int callput)
		: MBinaryPayoff(*world.get_economies()[0]->initialTS,2,1,1) 
	{      
		timeline = now, mat;
		index    = reportable_asset_index, 1;      
		alpha    = 1.0;
		S        = callput;
		A        = 1.0;
		a        = strike;
	}

	inline BondBinary::BondBinary(std::shared_ptr<BlackScholesAssetAdapter>& asset,double strike,double now,double mat,const TermStructure& xts)
		: MBinaryPayoff(xts,2,1,1) 
	{      
		underlying.push_back(asset);
		timeline = now, mat;
		index    = 0, 1;      
		alpha    = 0.0;
		S        = 1;
		A        = 1.0;
		a        = strike;
	}

	inline BondBinary::BondBinary(const GaussMarkovWorld& world,double strike,double now,double mat,int reportable_asset_index,int callput)
		: MBinaryPayoff(*world.get_economies()[0]->initialTS,2,1,1) 
	{      
		timeline = now, mat;
		index    = reportable_asset_index, 1;      
		alpha    = 0.0;
		S        = callput;
		A        = 1.0;
		a        = strike;
	}

	class AssetProductBinary : public MBinaryPayoff
	{
	public:
		inline AssetProductBinary(const GaussMarkovWorld& world,double strike,double now,double mat,const Array<int,1>& reportable_asset_index);
		inline AssetProductBinary(const GaussMarkovWorld& world,double strike,double now,double mat,const Array<int,1>& reportable_asset_index,const Array<double,1>& xalpha);
	};

	inline AssetProductBinary::AssetProductBinary(const GaussMarkovWorld& world,double strike,double now,double mat,const Array<int,1>& reportable_asset_index)
		: MBinaryPayoff(*world.get_economies()[0]->initialTS,2,reportable_asset_index.extent(firstDim),1) 
	{      
		int i;
		timeline = now, mat;
		for (i=0;i<reportable_asset_index.extent(firstDim);i++)
		{
			index(0,i)  = reportable_asset_index(i);      
			index(1,i)  = 1; 
		}  
		alpha    = 1.0;
		S        = 1;
		A        = 1.0;
		a        = strike;
	}

	inline AssetProductBinary::AssetProductBinary(const GaussMarkovWorld& world,double strike,double now,double mat,const Array<int,1>& reportable_asset_index,const Array<double,1>& xalpha)
		: MBinaryPayoff(*world.get_economies()[0]->initialTS,2,reportable_asset_index.extent(firstDim),1) 
	{      
		int i;
		timeline = now, mat;
		for (i=0;i<reportable_asset_index.extent(firstDim);i++)
		{
			index(0,i)  = reportable_asset_index(i);      
			index(1,i)  = 1; 
		}  
		alpha    = xalpha;
		S        = 1;
		A        = 1.0;
		a        = strike;
	}

	class BondProductBinary : public MBinaryPayoff
	{
	public:
		inline BondProductBinary(const GaussMarkovWorld& world,double strike,double now,double mat,const Array<int,1>& reportable_asset_index);
	};

	inline BondProductBinary::BondProductBinary(const GaussMarkovWorld& world,double strike,double now,double mat,const Array<int,1>& reportable_asset_index)
		: MBinaryPayoff(*world.get_economies()[0]->initialTS,2,reportable_asset_index.extent(firstDim),1) 
	{      
		int i;
		timeline = now, mat;
		for (i=0;i<reportable_asset_index.extent(firstDim);i++) 
		{
			index(0,i)  = reportable_asset_index(i);      
			index(1,i)  = 1; 
		}  
		alpha    = 0.0;
		S        = 1;
		A        = 1.0;
		a        = strike;
	}

} /* namespace derivative */

#endif /* _DERIVATIVE_MBINARYPAYOFFS_H_ */


