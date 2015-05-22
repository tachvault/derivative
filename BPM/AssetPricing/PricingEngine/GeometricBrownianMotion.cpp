/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#include "GeometricBrownianMotion.hpp"
#include "QFArrayUtil.hpp"
#include "IAssetValue.hpp"
#include "IAsset.hpp"

namespace derivative
{
	GeometricBrownianMotion::GeometricBrownianMotion(std::vector<std::shared_ptr<BlackScholesAssetAdapter> >& xunderlying)
		: underlying(xunderlying),T(NULL),timeline_(NULL),asset_values(xunderlying.size(),2),time_mapping(NULL),
		dW(xunderlying[0]->GetVolatilityFunction().factors()),vol_lvl(xunderlying[0]->GetVolatilityFunction().factors())
	{
		underlying.resize(xunderlying.size());
		std::copy(xunderlying.begin(), xunderlying.end(), underlying.begin());
	}

	GeometricBrownianMotion::~GeometricBrownianMotion()
	{
		if (T) delete T;
		if (timeline_) delete timeline_;
		if (time_mapping) delete time_mapping;
	}

	/// clone this object
	std::shared_ptr<GeometricBrownianMotion> GeometricBrownianMotion::Clone()
	{
		std::vector<std::shared_ptr<BlackScholesAssetAdapter> > new_underlying;
		new_underlying.reserve(underlying.size());
		for (auto &bs : underlying)
		{
			std::shared_ptr<BlackScholesAssetAdapter> new_bs = std::make_shared<BlackScholesAssetAdapter>(*bs);
			new_underlying.push_back(new_bs);
		}

		std::shared_ptr<GeometricBrownianMotion> bwm = std::make_shared<GeometricBrownianMotion>(new_underlying);
		bwm->T = new Array<double, 1>(T->copy());
		bwm->timeline_ = new Array<double, 1>(timeline_->copy());
		bwm->dW = dW.copy();
		bwm->vol_lvl = vol_lvl.copy();
		bwm->asset_values = asset_values.copy();
		bwm->time_mapping = new Array<int, 1>(time_mapping->copy());

		return bwm;
	}

	/// Query the dimension of the process.
	int GeometricBrownianMotion::dimension() const
	{
		return underlying.size();
	}

	/// Generate a realisation of the process under the martingale measure associated with deterministic bond prices.
	void GeometricBrownianMotion::operator()(Array<double,2>& underlying_values,const Array<double,2>& x,const TermStructure& ts)
	{
		int i,j;
		if (!T) throw std::logic_error("Timeline not set in GeometricBrownianMotion::operator()");
		for (j=1; j<asset_values.extent(secondDim); j++)
		{   // Loop over time line
			double fwd = ts((*T)(j-1))/ts((*T)(j));
			dW = x(Range::all(),j-1) * std::sqrt((*T)(j)-(*T)(j-1));
			for (i=0; i<asset_values.extent(firstDim); i++) { // Loop over assets
				asset_values(i,j) = asset_values(i,j-1) * fwd * underlying[i]->GetDivDiscount((*T)(j-1),(*T)(j)) * underlying[i]->DoleansExp((*T)(j-1),(*T)(j),dW);
			}
		}
		// set return values
		for (i=0; i<underlying_values.extent(firstDim); i++)
		{
			for (j=0; j<underlying_values.extent(secondDim); j++) underlying_values(i,j) = asset_values(i,(*time_mapping)(j));
		}
	}

	/// Generate a realisation of the process under the martingale measure associated with a given numeraire asset.
	void GeometricBrownianMotion::operator()(Array<double,2>& underlying_values,Array<double,1>& numeraire_values,const Array<double,2>& x,const TermStructure& ts,int numeraire_index)
	{
		int i,j;
		if (!T) throw std::logic_error("Timeline not set in GeometricBrownianMotion::operator()");
		if (numeraire_index>=0)
		{
			// set return values
			for (j=1; j<asset_values.extent(secondDim); j++)
			{   // Loop over time line
				double fwd = ts((*T)(j-1))/ts((*T)(j));
				dW = x(Range::all(),j-1) * std::sqrt((*T)(j)-(*T)(j-1));
				add_drift((*T)(j-1),(*T)(j),numeraire_index);
				for (i=0; i<asset_values.extent(firstDim); i++)
				{   // Loop over assets
					asset_values(i,j) = asset_values(i,j-1) * fwd * underlying[i]->GetDivDiscount((*T)(j-1),(*T)(j)) * underlying[i]->DoleansExp((*T)(j-1),(*T)(j),dW);
				}
			}
			// set return values
			for (i=0; i<underlying_values.extent(firstDim); i++)
			{
				for (j=0; j<underlying_values.extent(secondDim); j++) underlying_values(i,j) = asset_values(i,(*time_mapping)(j));
			}
			// set numeraire values
			// Note that numeraire values must be adjusted for reinvestment of dividends.
			for (j=0; j<underlying_values.extent(secondDim); j++)
				numeraire_values(j) = underlying_values(numeraire_index,j) / underlying[numeraire_index]->GetDivDiscount((*timeline_)(0),(*timeline_)(j));
		}
		else (*this)(underlying_values,x,ts);
	}

	void GeometricBrownianMotion::add_drift(double tstart,double tend,int numeraire_index)
	{
		if (numeraire_index>=0)
		{
			if (!((underlying[numeraire_index]->GetVolatilityFunction()).get_volatility_level(tstart,tend,vol_lvl)))
				throw std::logic_error("Unable to get volatility levels in GeometricBrownianMotion::add_drift()");
			vol_lvl *= tend - tstart;
			dW += vol_lvl;
		}
	}

	/// Set process timeline.
	bool GeometricBrownianMotion::set_timeline(const Array<double,1>& timeline)
	{
		int i,j;
		/* check timeline conformity with volatility of underlying assets
		all volatilities must be constant on each time segment */
		Array<double,1> segments(timeline.copy());
		for (i=0; i<underlying.size(); i++)
		{
			Array<double,1> tmp_segments = unique_merge(segments,(underlying[i]->GetVolatilityFunction()).segments(timeline(0),timeline(timeline.extent(firstDim)-1)-timeline(0)));
			segments.resize(tmp_segments.extent(firstDim));
			segments = tmp_segments;
		}
		if (T) delete T;
		T = new Array<double,1>(segments.copy());
		if (timeline_) delete timeline_;
		timeline_ = new Array<double,1>(timeline.copy());
		// map timeline indices to segments
		if (time_mapping) delete time_mapping;
		time_mapping = new Array<int,1>(timeline.extent(firstDim));
		j = 0;
		for (i=0; i<segments.extent(firstDim); i++)
		{
			if (timeline(j)==segments(i))
			{
				(*time_mapping)(j) = i;
				j++;
			}
		}
		// resize scratch arrays
		if (asset_values.extent(secondDim)!=T->extent(firstDim)) asset_values.resize(underlying.size(),T->extent(firstDim));
		for (i=0; i<underlying.size(); i++) asset_values(i,0) = underlying[i]->GetAssetValue()->GetTradePrice();
		return T;
	}
}

