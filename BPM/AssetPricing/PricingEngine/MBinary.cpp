/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#include "MBinary.hpp"
#include "IAssetValue.hpp"

namespace derivative
{
	double MBinaryPayoff::operator()(const Array<double,1>& underlying_values,const Array<double,1>& numeraire_values)
	{
		int i,j;
		double     result = 0.0;
		bool   indicator = true;
		for (i=0;i<A.extent(firstDim);i++) 
		{
			double x = 1.0;
			for (j=0;j<A.extent(secondDim);j++) x *= std::pow(underlying_values(j),A(i,j));
			if (S(i,i)*x<=S(i,i)*a(i))
			{
				indicator = false;
				break;
			}
		}
		if (indicator) 
		{
			result = 1.0;
			for (j=0;j<alpha.extent(firstDim);j++) result *= std::pow(underlying_values(j),alpha(j)); 
		}
		return notional * numeraire_values(0)/numeraire_values(numeraire_values.extent(firstDim)-1) * result;
	}

	MBinary::MBinary(std::vector<std::shared_ptr<BlackScholesAssetAdapter> >& xunderlying,  ///< Vector of pointers to underlying assets.
		const TermStructure&                           xts,  ///< (Deterministic) term structure of interest rates.
		const Array<double,1>&                   xtimeline,  ///< Time line collecting all event dates.
		const Array<int,2>&                         xindex,  ///< A 2 x N matrix of indices, where each column represents the 
		///< indices of an (asset,time) combination affecting the payoff.
		const Array<double,1>&                      xalpha,  ///< Payoff powers.
		const Array<int,2>&                             xS,  ///< Exercise indicators.
		const Array<double,2>&                          xA,  ///< Exercise condition matrix.  
		const Array<double,1>&                          xa,  ///< Strike vector.
		double xnotional
		)
		: payoff_dimension(xindex.extent(secondDim)),exercise_dimension(xA.extent(firstDim)),number_of_assets(xunderlying.size()),
		ts(xts),underlying(xunderlying),timeline(xtimeline.copy()),index(xindex.copy()),alpha(xalpha.copy()),S(xS.copy()),A(xA.copy()),a(xa.copy()),
		mu(xindex.extent(secondDim)),sgm(xindex.extent(secondDim),xindex.extent(secondDim)),Sd(xa.extent(firstDim)),
		x(xindex.extent(secondDim)),SCS(xa.extent(firstDim),xa.extent(firstDim)),mvrnd(nullptr),worthless(false),notional(xnotional)
	{
		initialise();   
	}

	MBinary::MBinary(MBinaryPayoff& xpayoff)
		: payoff_dimension(xpayoff.index.extent(secondDim)),exercise_dimension(xpayoff.A.extent(firstDim)),number_of_assets(xpayoff.underlying.size()),
		ts(xpayoff.ts),underlying(xpayoff.underlying),timeline(xpayoff.timeline.copy()),index(xpayoff.index.copy()),alpha(xpayoff.alpha.copy()),
		S(xpayoff.S.copy()),A(xpayoff.A.copy()),a(xpayoff.a.copy()),
		mu(xpayoff.index.extent(secondDim)),sgm(xpayoff.index.extent(secondDim),xpayoff.index.extent(secondDim)),Sd(xpayoff.a.extent(firstDim)),
		x(xpayoff.index.extent(secondDim)),SCS(xpayoff.a.extent(firstDim),xpayoff.a.extent(firstDim)),mvrnd(nullptr),worthless(false),notional(xpayoff.notional)
	{
		initialise();   
	}

	MBinary::MBinary(const MBinary& original,const Array<double,1>& T,const Array<double,2>& history)
		: payoff_dimension(original.payoff_dimension),exercise_dimension(original.exercise_dimension),number_of_assets(original.number_of_assets),
		ts(original.ts),underlying(original.underlying),timeline(original.timeline.copy()),index(original.index.copy()),alpha(original.alpha.copy()),
		S(original.S.copy()),A(original.A.copy()),a(original.a.copy()),mu(original.mu.copy()),sgm(original.sgm.copy()),Sd(original.Sd.copy()),
		x(original.x.copy()),SCS(original.SCS.copy()),mvrnd(nullptr),worthless(false),notional(original.notional)
	{
		initialise(T,history);   
	}

	void MBinary::initialise()
	{
		int i,j,k,l;
		firstIndex idx;
		secondIndex jdx;
		thirdIndex kdx;
		for (i=0;i<payoff_dimension;i++) x(i) = underlying[index(0,i)]->GetAssetValue()->GetTradePrice();
		double now = timeline(0);
		Array<double,1> r_tau_k(timeline.extent(firstDim));
		for (k=0;k<timeline.extent(firstDim);k++) r_tau_k(k) = -std::log(ts(timeline(k))/ts(timeline(0)));
		for (j=0;j<payoff_dimension;j++) 
		{
			i = index(0,j);  // asset index
			k = index(1,j);  // time index
			for (l=j;l<payoff_dimension;l++) 
			{  // populate the covariance matrix
				sgm(j,l) = sgm(l,j) = underlying[i]->GetCovariance(now,std::min(timeline(k),timeline(index(1,l)))-now, \
					underlying[index(0,l)]); 
			}
			mu(j)  = r_tau_k(k) - underlying[i]->GetDivYield(now,timeline(k))*(timeline(k)-now) - 0.5*sgm(j,j); 
		}
		// The last date in the timeline is assumed to be the option expiry.
		beta  = ts(timeline(timeline.extent(firstDim)-1))/ts(timeline(0));
		beta *= std::exp(blitz::sum(alpha*mu) + 0.5*blitz::sum(alpha*blitz::sum(sgm(idx,jdx)*alpha(jdx),jdx)));
		Array<double,2> aga(exercise_dimension,exercise_dimension);
		Array<double,2> ag(exercise_dimension,payoff_dimension);
		ag  = blitz::sum(A(idx,kdx)*sgm(kdx,jdx),kdx);
		aga = blitz::sum(ag(idx,kdx)*A(jdx,kdx),kdx);
		Array<double,1> A_log_x(exercise_dimension);
		Array<double,1> tmp(exercise_dimension);
		Array<double,2> tmp2(exercise_dimension,exercise_dimension);
		A_log_x = blitz::sum(A(idx,jdx)*log(x(jdx)),jdx);
		tmp     = blitz::sum(A(idx,jdx)*mu(jdx),jdx) + blitz::sum(ag(idx,jdx)*alpha(jdx),jdx);
		Sd      = blitz::sum(S(idx,jdx) * (A_log_x(jdx) - log(a(jdx)) + tmp(jdx)),jdx);
		tmp2    = blitz::sum(S(idx,kdx)*aga(kdx,jdx),kdx);
		SCS     = blitz::sum(tmp2(idx,kdx)*S(kdx,jdx),kdx);
		mvrnd.reset(new MultivariateNormal(SCS));
		P_t = I_t = -1.0;
	}

	void MBinary::initialise(const Array<double,1>& T,const Array<double,2>& history)
	{
		int i,j,k,l,count;
		firstIndex idx;
		secondIndex jdx;
		thirdIndex kdx;
		P_t = I_t = -1.0;
		// check input dimension - history is a state variables x timepoints Array 
		if ((T.extent(firstDim)!=history.extent(secondDim))||(x.extent(firstDim)!=history.extent(firstDim))) 
			throw std::runtime_error("Dimension mismatch in MBinary::initialise");
		// check history timeline against original timeline
		int remaining_T;
		for (remaining_T=timeline.extent(firstDim);(remaining_T>0)&&(timeline(remaining_T-1)>T(T.extent(firstDim)-1));remaining_T--);
		remaining_T = timeline.extent(firstDim) - remaining_T;
		if (remaining_T<1) throw std::runtime_error("MBinary has already matured in MBinary::initialise");
		// map timeline to T
		Array<int,1> tmap(timeline.extent(firstDim)-remaining_T);
		j = 0;
		for (i=0;i<tmap.extent(firstDim);i++)
		{
			while ((j<T.extent(firstDim))&&(timeline(i)!=T(j))) j++;
			if (j==T.extent(firstDim)) throw std::runtime_error("Timeline mismatch in MBinary::initialise");
			tmap(i) = j; 
		}
		double now = T(T.extent(firstDim)-1);
		Array<double,1> r_tau_k(remaining_T+1);
		// interest rate yield times time to maturity
		r_tau_k(0) = 0.0;
		for (k=1;k<=remaining_T;k++) r_tau_k(k) = -std::log(ts(timeline(timeline.extent(firstDim)-1-remaining_T+k))/ts(now));
		// adjust timeline and index
		Array<double,1> oldtimeline(timeline.copy());
		timeline.resize(remaining_T+1);
		timeline(0) = now;
		for (k=1;k<=remaining_T;k++) timeline(k) = oldtimeline(oldtimeline.extent(firstDim)-remaining_T-1+k);
		Array<int,2>    oldindex(index.copy());
		Array<double,1> oldalpha(alpha.copy());
		int oldpayoff_dimension = payoff_dimension;
		payoff_dimension = 0;
		int oldexercise_dimension = exercise_dimension;
		exercise_dimension = 0;
		for (j=0;j<oldpayoff_dimension;j++) 
		{
			k = oldindex(1,j);  // time index
			if (oldtimeline(k)>now) 
			{
				index(0,payoff_dimension) = oldindex(0,j);
				index(1,payoff_dimension) = k - (timeline.extent(firstDim) - remaining_T) + 1;
				alpha(payoff_dimension)   = oldalpha(j);
				payoff_dimension++; 
			}
			else 
			{ // known part of payoff is applied to notional
				notional *= std::pow(history(oldindex(0,j),tmap(k)),oldalpha(j)); 
			}
		}
		// preserve row in A only if there is a non-zero exponent for an asset value still in the future, otherwise reduce exercise dimension
		Array<double,2> oldA(A.copy());
		Array<int,2>    oldS(S.copy());
		Array<double,1> olda(a.copy());
		for (l=0;l<oldexercise_dimension;l++) 
		{
			bool keep = false;
			a(exercise_dimension) = olda(l);
			S(exercise_dimension,exercise_dimension) = oldS(l,l);
			count = 0;
			for (j=0;j<oldpayoff_dimension;j++) 
			{
				i = oldindex(0,j);  // asset index
				k = oldindex(1,j);  // time index
				if (oldtimeline(k)>now) 
				{
					A(exercise_dimension,count) = oldA(l,j);
					count++;
					if (oldA(l,j)!=0.0) keep = true;  
				}
				else a(exercise_dimension) /= std::pow(history(i,tmap(k)),oldA(l,j)); 
			}
			if (keep)
			{ // keep elements of A and adjust a
				exercise_dimension++; 
			}
			else 
			{ // indicator function can be evaluated based on history - if indicator evaluates to zero, value of MBinary is zero
				if (oldS(l,l)<=oldS(l,l)*a(exercise_dimension)) 
				{
					worthless = true;
					return; 
				}
			}
		}
		if (payoff_dimension<1) throw std::runtime_error("MBinary has already matured in MBinary::initialise");
		if (payoff_dimension<oldpayoff_dimension) 
		{ // resize arrays
			index.resizeAndPreserve(2,payoff_dimension);
			alpha.resizeAndPreserve(payoff_dimension);
			mu.resize(payoff_dimension);
			sgm.resize(payoff_dimension,payoff_dimension);
			x.resize(payoff_dimension); 
		}
		if (exercise_dimension<1) 
		{ // no check of exercise condition needed, but keep one exercise dimension
			I_t = 1.0;
			exercise_dimension++;   
		}
		if (exercise_dimension<oldexercise_dimension) 
		{ // resize arrays
			a.resizeAndPreserve(exercise_dimension);
			S.resizeAndPreserve(exercise_dimension,exercise_dimension);
			Sd.resize(exercise_dimension);
			SCS.resize(exercise_dimension,exercise_dimension); 
		}
		if ((exercise_dimension<oldexercise_dimension)||(payoff_dimension<oldpayoff_dimension))
		{
			A.resizeAndPreserve(exercise_dimension,payoff_dimension);
		}
		// set current asset values
		for (i=0;i<payoff_dimension;i++) 
		{
			x(i) = history(index(0,i),history.extent(secondDim)-1);
		}
		for (j=0;j<payoff_dimension;j++) 
		{
			i = index(0,j);  // asset index
			k = index(1,j);  // time index
			for (l=j;l<payoff_dimension;l++) 
			{  // populate the covariance matrix
				sgm(j,l) = sgm(l,j) = underlying[i]->GetCovariance(now,std::min(timeline(k),timeline(index(1,l)))-now,underlying[index(0,l)]);
			}
			mu(j)  = r_tau_k(k) - underlying[i]->GetDivYield(now,timeline(k))*(timeline(k)-now) - 0.5*sgm(j,j); 
		}
		// The last date in the timeline is assumed to be the option expiry.
		beta  = ts(timeline(timeline.extent(firstDim)-1))/ts(timeline(0));
		beta *= std::exp(blitz::sum(alpha*mu) + 0.5*blitz::sum(alpha*blitz::sum(sgm(idx,jdx)*alpha(jdx),jdx)));
		Array<double,2> aga(exercise_dimension,exercise_dimension);
		Array<double,2> ag(exercise_dimension,payoff_dimension);
		ag  = blitz::sum(A(idx,kdx)*sgm(kdx,jdx),kdx);
		aga = blitz::sum(ag(idx,kdx)*A(jdx,kdx),kdx);
		Array<double,1> A_log_x(exercise_dimension);
		Array<double,1> tmp(exercise_dimension);
		Array<double,2> tmp2(exercise_dimension,exercise_dimension);
		A_log_x = blitz::sum(A(idx,jdx)*log(x(jdx)),jdx);
		tmp     = blitz::sum(A(idx,jdx)*mu(jdx),jdx) + blitz::sum(ag(idx,jdx)*alpha(jdx),jdx);
		Sd      = blitz::sum(S(idx,jdx) * (A_log_x(jdx) - log(a(jdx)) + tmp(jdx)),jdx);
		tmp2    = blitz::sum(S(idx,kdx)*aga(kdx,jdx),kdx);
		SCS     = blitz::sum(tmp2(idx,kdx)*S(kdx,jdx),kdx);
		mvrnd.reset(new MultivariateNormal(SCS));
	}

	MBinary::~MBinary()
	{
	}

	double MBinary::price(unsigned long n)
	{
		if (worthless) return 0.0;
		if (P_t<0.0) P_t = beta*std::exp(blitz::sum(log(x)*alpha));
		if ((I_t<0.0)||(n!=current_n)) {
			I_t = mvrnd->CDF(Sd,n,true);
			current_n = n; }
		return notional*P_t*I_t;
	}

	double MBinary::price(double t,double single_underlying,unsigned long n)
	{
		if (worthless) return 0.0;
		if ((timeline.extent(firstDim)!=2)||(underlying.size()>1)) throw std::runtime_error("MBinary depends on more than one asset observation");
		if (t>timeline(1)) throw std::runtime_error("Attempt to evaluate MBinary after expiry");
		if (I_t==1.0) return (ts(timeline(1))/ts(t) * std::pow(single_underlying,alpha(0)));
		std::shared_ptr<MBinaryPayoff> po = get_payoff();
		if (t==timeline(1)) 
		{
			Array<double,1> u(1),num(2);
			u = single_underlying;
			num = 1.0; 
			return (*po)(u,num); 
		}
		BlackScholesAsset& tmpasset = const_cast<BlackScholesAsset&>(underlying[0]->GetOrigin());
		tmpasset.initial_value(single_underlying);
		po->underlying[0] = underlying[0];
		po->timeline(0) = t;
		MBinary tmpMB(*po);
		return tmpMB.price(n);
	}

	double MBinary::price(const Array<double,1>& T,const Array<double,2>& history,unsigned long n)
	{
		if (worthless) return 0.0;
		MBinary tmpMBinary(*this,T,history);
		return tmpMBinary.price(n);
	}

	double MBinary::delta(int i,unsigned long n)
	{
		if (worthless) return 0.0;
		if (P_t<0.0) P_t = beta*std::exp(blitz::sum(log(x)*alpha));
		if (I_t==1.0) return notional*dPdX(i)*I_t;
		if (I_t<0.0) I_t = mvrnd->CDF(Sd,n);
		return notional*(dPdX(i)*I_t + P_t*dIdX(i));
	}

	double MBinary::dPdX(int i)
	{
		int j;
		if (P_t<0.0) P_t = beta*std::exp(blitz::sum(log(x)*alpha));
		double alpha_i = 0.0;
		for (j=0;j<payoff_dimension;j++)
		{
			int ij = index(0,j);
			if (ij==i) alpha_i += alpha(j);
		}
		return alpha_i*P_t/(underlying[i]->GetOrigin().initial_value());
	}

	double MBinary::dIdX(int i)
	{
		int j;
		double result = 0.0;
		for (j=0;j<exercise_dimension;j++) 
		{ }
		return result;
	}

	std::shared_ptr<MBinaryPayoff> MBinary::get_payoff() const
	{
		std::shared_ptr<MBinaryPayoff> result(new MBinaryPayoff(ts,timeline.extent(firstDim),payoff_dimension,exercise_dimension,notional));
		std::vector<std::shared_ptr<BlackScholesAssetAdapter> >::const_iterator iter;
		for (iter=underlying.begin();iter!=underlying.end();++iter) result->underlying.push_back(*iter);
		result->timeline = timeline; 
		result->index    = index; 
		result->alpha    = alpha;  
		result->S        = S;      
		result->A        = A;
		result->a        = a; 
		return result;
	}
}
