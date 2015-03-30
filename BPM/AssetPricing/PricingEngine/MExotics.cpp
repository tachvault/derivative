/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#include "MExotics.hpp"

namespace derivative
{
	using namespace derivative::exotics;

	Margrabe::Margrabe(const std::shared_ptr<BlackScholesAssetAdapter>& S1,const std::shared_ptr<BlackScholesAssetAdapter>& S2, \
		double t,double T,double xK1,double xK2,const TermStructure& xts)
		: K1(xK1),K2(xK2),ts(xts),part1(xts,2,2,1),part2(xts,2,2,1),M1(nullptr),M2(nullptr)
	{
		part1.underlying.push_back(S1);
		part1.underlying.push_back(S2);
		part2.underlying.push_back(S1);
		part2.underlying.push_back(S2);
		part1.timeline = t, T;
		part2.timeline = t, T;
		part1.index = 0, 1,
			1, 1; 
		part2.index = 0, 1,
			1, 1;      
		part1.alpha = 1.0, 0.0;
		part2.alpha = 0.0, 1.0;
		part1.S = 1;
		part2.S = 1;
		part1.A = 1.0, -1.0;
		part2.A = 1.0, -1.0;
		part1.a = K2/K1;
		part2.a = K2/K1;
		M1 = std::unique_ptr<MBinary>(new MBinary(part1));
		M2 = std::unique_ptr<MBinary>(new MBinary(part2));
	}

	Margrabe::~Margrabe()
	{  
	}

	StandardOption::StandardOption(const std::shared_ptr<BlackScholesAssetAdapter>& S,double t,double T,double xK,const TermStructure& xts,int xsign)
		: K(xK),ts(xts),part1(xts,2,1,1),part2(xts,2,1,1),M1(nullptr),M2(nullptr),sign(xsign)
	{
		part1.underlying.push_back(S);
		part2.underlying.push_back(S);
		part1.timeline = t, T;
		part2.timeline = t, T;
		part1.index = 0,
			1; 
		part2.index = 0,
			1;      
		part1.alpha = 1.0;
		part2.alpha = 0.0;
		part1.S = sign;
		part2.S = sign;
		part1.A = 1.0;
		part2.A = 1.0;
		part1.a = K;
		part2.a = K;
		M1 = std::unique_ptr<MBinary>(new MBinary(part1));
		M2 = std::unique_ptr<MBinary>(new MBinary(part2));
	}

	StandardOption::~StandardOption()
	{
	}

	PowerOption::PowerOption(const std::shared_ptr<BlackScholesAssetAdapter>& S,double alpha,double t,double T,double xK,const TermStructure& xts)
		: K(xK),ts(xts),part1(xts,2,1,1),part2(xts,2,1,1),M1(nullptr),M2(nullptr)
	{
		part1.underlying.push_back(S);
		part2.underlying.push_back(S);
		part1.timeline = t, T;
		part2.timeline = t, T;
		part1.index = 0,
			1; 
		part2.index = 0,
			1;      
		part1.alpha = alpha;
		part2.alpha = 0.0;
		part1.S = 1;
		part2.S = 1;
		part1.A = alpha;
		part2.A = alpha;
		part1.a = K;
		part2.a = K;
		M1 = std::unique_ptr<MBinary>(new MBinary(part1));
		M2 = std::unique_ptr<MBinary>(new MBinary(part2));
	}

	PowerOption::~PowerOption()
	{
	}

	std::shared_ptr<MCPayoffList> PowerOption::get_payoff() const
	{
		std::shared_ptr<MCPayoffList> result(new MCPayoffList());
		std::shared_ptr<MBinaryPayoff> M1payoff = M1->get_payoff();
		std::shared_ptr<MBinaryPayoff> M2payoff = M2->get_payoff();
		result->push_back(M1payoff);
		result->push_back(M2payoff,-K);
		return result;
	}

	DiscreteGeometricMeanFixedStrike::DiscreteGeometricMeanFixedStrike(const std::shared_ptr<BlackScholesAssetAdapter>& S,const Array<double,1>& T,double xK,const TermStructure& xts,
		int number_of_observations_in_existing_average,double existing_average)
		: ts(xts),K(xK),geo(xts,T.extent(firstDim),T.extent(firstDim)-1,1),
		BB(xts,T.extent(firstDim),T.extent(firstDim)-1,1),Mgeo(nullptr),MBB(nullptr)
	{
		firstIndex  idx;
		secondIndex jdx;
		int n = T.extent(firstDim)-1 + number_of_observations_in_existing_average;
		geo.underlying.push_back(S);
		BB.underlying.push_back(S);
		geo.timeline = T;
		BB.timeline  = T;
		geo.index = idx * (jdx+1);
		BB.index  = idx * (jdx+1);
		geo.alpha = 1.0/double(n);
		BB.alpha  = 0.0;
		geo.S     = 1;
		BB.S      = 1;
		geo.A     = 1.0/double(n);
		BB.A      = 1.0/double(n);
		factor    = std::pow(existing_average,number_of_observations_in_existing_average/double(n));
		geo.a     = K/factor;
		BB.a      = K/factor;
		Mgeo = std::unique_ptr<MBinary>(new MBinary(geo));
		MBB = std::unique_ptr<MBinary>(new MBinary(BB));
	}

	DiscreteGeometricMeanFixedStrike::DiscreteGeometricMeanFixedStrike(const GaussMarkovWorld& world,const Array<double,1>& T,double xK,int reportable_asset_index,int number_of_observations_in_existing_average,double existing_average)
		: ts(*world.get_economies()[0]->initialTS),K(xK),geo(*world.get_economies()[0]->initialTS,T.extent(firstDim),T.extent(firstDim)-1,1),
		BB(*world.get_economies()[0]->initialTS,T.extent(firstDim),T.extent(firstDim)-1,1),Mgeo(nullptr),MBB(nullptr)
	{
		int i;
		int n = T.extent(firstDim)-1 + number_of_observations_in_existing_average;
		geo.timeline = T;
		BB.timeline  = T;
		for (i=0;i<T.extent(firstDim)-1;i++) {
			geo.index(0,i) = BB.index(0,i)  = reportable_asset_index;
			geo.index(1,i) = BB.index(1,i)  = i+1; }
		geo.alpha = 1.0/double(n);
		BB.alpha  = 0.0;
		geo.S     = 1;
		BB.S      = 1;
		geo.A     = 1.0/double(n);
		BB.A      = 1.0/double(n);
		factor    = std::pow(existing_average,number_of_observations_in_existing_average/double(n));
		geo.a     = K/factor;
		BB.a      = K/factor;
		Mgeo = std::unique_ptr<MBinary>(new MBinary(world,geo));
		MBB = std::unique_ptr<MBinary>(new MBinary(world,BB));
	}

	DiscreteGeometricMeanFixedStrike::~DiscreteGeometricMeanFixedStrike()
	{
	}

	DiscreteGeometricMeanFloatingStrike::DiscreteGeometricMeanFloatingStrike(const std::shared_ptr<BlackScholesAssetAdapter>& S, \
		const Array<double,1>& T,double xK,const TermStructure& xts,int number_of_observations_in_existing_average,\
		double existing_average)
		: ts(xts),K(xK),geo(xts,T.extent(firstDim),T.extent(firstDim)-1,1),
		AB(xts,T.extent(firstDim),T.extent(firstDim)-1,1),Mgeo(nullptr),MAB(nullptr)
	{
		firstIndex  idx;
		secondIndex jdx;
		int m = T.extent(firstDim)-1;
		int n = m + number_of_observations_in_existing_average;
		geo.underlying.push_back(S);
		AB.underlying.push_back(S);
		geo.timeline  = T;
		AB.timeline   = T;
		geo.index     = idx * (jdx+1);
		AB.index      = idx * (jdx+1);
		geo.alpha     = 1.0/double(n);
		AB.alpha      = 0.0;
		AB.alpha(m-1) = 1.0;
		geo.S         = 1;
		AB.S          = 1;
		geo.A         = -1.0/double(n);
		geo.A(0,m-1) += 1.0;
		AB.A          = geo.A;
		factor        = std::pow(existing_average,number_of_observations_in_existing_average/double(n));
		geo.a         = K * factor;
		AB.a          = K * factor;
		Mgeo = std::unique_ptr<MBinary>(new MBinary(geo));
		MAB = std::unique_ptr<MBinary>(new MBinary(AB));
	}

	DiscreteBarrierOut::DiscreteBarrierOut(const std::shared_ptr<BlackScholesAssetAdapter>& S,  ///< Underlying asset.
		const Array<double,1>& T,    ///< Barrier monitoring time points.
		double xK,                   ///< Strike.
		double barrier,              ///< Barrier.
		const TermStructure& xts,    ///< (Deterministic) term structure of interest rates.
		int xcallput,                ///< Call (1) or put (-1). Call is default. 
		int updown                   ///< Up (1) or down (-1) option. Down is default.
		) : ts(xts),K(xK),AB(xts,T.extent(firstDim),T.extent(firstDim),T.extent(firstDim)),
		BB(xts,T.extent(firstDim),T.extent(firstDim),T.extent(firstDim)),
		MBB(nullptr),MAB(nullptr),callput(xcallput)
	{
		firstIndex  idx;
		secondIndex jdx;
		int n = T.extent(firstDim)-1;
		AB.underlying.push_back(S);
		BB.underlying.push_back(S);
		AB.timeline   = T;
		BB.timeline   = T;
		AB.index      = idx * (jdx+1);
		BB.index      = idx * (jdx+1);
		AB.index(1,AB.index.extent(secondDim)-1) = AB.index(1,AB.index.extent(secondDim)-2);
		BB.index(1,BB.index.extent(secondDim)-1) = BB.index(1,BB.index.extent(secondDim)-2);
		AB.alpha      = 0.0;
		AB.alpha(n-1) = 1.0;
		BB.alpha      = 0.0; 
		AB.S          = (idx==jdx) * -updown;
		AB.S(n,n)     = callput;
		BB.S          = AB.S;
		AB.A          = (idx==jdx);
		BB.A          = AB.A;
		AB.a          = barrier;
		AB.a(n)       = K;
		BB.a        = AB.a;
		MAB = std::unique_ptr<MBinary>(new MBinary(AB));
		MBB = std::unique_ptr<MBinary>(new MBinary(BB));
	}

	DiscreteBarrierOut::~DiscreteBarrierOut()
	{
	}

	std::shared_ptr<MCPayoffList> DiscreteGeometricMeanFixedStrike::get_payoff() const
	{
		std::shared_ptr<MCPayoffList> result(new MCPayoffList());
		std::shared_ptr<MBinaryPayoff> M1payoff = Mgeo->get_payoff();
		std::shared_ptr<MBinaryPayoff> M2payoff = MBB->get_payoff();
		result->push_back(M1payoff,factor);
		result->push_back(M2payoff,-K);
		return result;
	}

	std::shared_ptr<MCPayoffList> DiscreteGeometricMeanFloatingStrike::get_payoff() const
	{
		std::shared_ptr<MCPayoffList> result(new MCPayoffList());
		std::shared_ptr<MBinaryPayoff> M2payoff = Mgeo->get_payoff();
		std::shared_ptr<MBinaryPayoff> M1payoff = MAB->get_payoff();
		result->push_back(M1payoff);
		result->push_back(M2payoff,-K*factor);
		return result;
	}

	std::shared_ptr<MCPayoffList> DiscreteBarrierOut::get_payoff() const
	{
		std::shared_ptr<MCPayoffList> result(new MCPayoffList());
		std::shared_ptr<MBinaryPayoff> M1payoff = MAB->get_payoff();
		std::shared_ptr<MBinaryPayoff> M2payoff = MBB->get_payoff();
		result->push_back(M1payoff);
		result->push_back(M2payoff,-K);
		return result;
	}

	std::shared_ptr<MCPayoffList> Margrabe::get_payoff() const
	{
		std::shared_ptr<MCPayoffList> result(new MCPayoffList());
		std::shared_ptr<MBinaryPayoff> M1payoff = M1->get_payoff();
		std::shared_ptr<MBinaryPayoff> M2payoff = M2->get_payoff();
		result->push_back(M1payoff,K1);
		result->push_back(M2payoff,-K2);
		return result;
	}

	double Margrabe::early_exercise_payoff(const Array<double,1>& T,const Array<double,2>& history) const
	{
		// history is a (time points) x (state variables) Array
		// making assumption that first and second state variables are asset prices determining option payoff
		int n = history.extent(firstDim) - 1;
		return (std::max(0.0,K1*history(n,0)-K2*history(n,1)));
	}

	ProductOption::ProductOption(const std::shared_ptr<BlackScholesAssetAdapter>& S1,const std::shared_ptr<BlackScholesAssetAdapter>& S2, \
		double t,double T,double xK1,const TermStructure& xts,int xsign)
		: K1(xK1),ts(xts),part1(xts,2,2,1),part2(xts,2,2,1),M1(nullptr),M2(nullptr),sign(xsign)
	{
		part1.underlying.push_back(S1);
		part1.underlying.push_back(S2);
		part2.underlying.push_back(S1);
		part2.underlying.push_back(S2);
		part1.timeline = t, T;
		part2.timeline = t, T;
		part1.index = 0, 1,
			1, 1; 
		part2.index = 0, 1,
			1, 1;      
		part1.alpha = 1.0, 1.0;
		part2.alpha = 0.0, 0.0;
		part1.S = sign;
		part2.S = sign;
		part1.A = 1.0, 1.0;
		part2.A = 1.0, 1.0;
		part1.a = K1;
		part2.a = K1;
		M1 = std::unique_ptr<MBinary>(new MBinary(part1));
		M2 = std::unique_ptr<MBinary>(new MBinary(part2));
	}

	ProductOption::~ProductOption()
	{  
	}

	std::shared_ptr<MCPayoffList> ProductOption::get_payoff() const
	{
		std::shared_ptr<MCPayoffList> result(new MCPayoffList());
		std::shared_ptr<MBinaryPayoff> M1payoff = M1->get_payoff();
		std::shared_ptr<MBinaryPayoff> M2payoff = M2->get_payoff();
		result->push_back(M1payoff);
		result->push_back(M2payoff,-K1);
		return result;
	}

	double ProductOption::early_exercise_payoff(const Array<double,1>& T,const Array<double,2>& history) const
	{
		// history is a (time points) x (state variables) Array
		// making assumption that first and second state variables are asset prices determining option payoff
		int n = history.extent(firstDim) - 1;
		return (std::max(0.0,sign*(history(n,0)*history(n,1)-K1)));
	}
}
