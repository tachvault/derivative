/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#include "MBinary.hpp"
#include "GaussMarkovWorld.hpp"

namespace derivative
{
	MBinary::MBinary(const GaussMarkovWorld& world,MBinaryPayoff& xpayoff)
		: payoff_dimension(xpayoff.index.extent(secondDim)),exercise_dimension(xpayoff.A.extent(firstDim)),number_of_assets(xpayoff.underlying.size()),
		ts(xpayoff.ts),underlying(xpayoff.underlying),timeline(xpayoff.timeline.copy()),index(xpayoff.index.copy()),alpha(xpayoff.alpha.copy()),
		S(xpayoff.S.copy()),A(xpayoff.A.copy()),a(xpayoff.a.copy()),
		mu(xpayoff.index.extent(secondDim)),sgm(xpayoff.index.extent(secondDim),xpayoff.index.extent(secondDim)),Sd(xpayoff.a.extent(firstDim)),
		x(xpayoff.index.extent(secondDim)),SCS(xpayoff.a.extent(firstDim),xpayoff.a.extent(firstDim)),mvrnd(nullptr),worthless(false),notional(xpayoff.notional)
	{
		int i,j,k,l;
		const DeterministicAssetVol* sgm_Xi;
		const DeterministicAssetVol* sgm_Di;
		const DeterministicAssetVol* sgm_istar;
		const DeterministicAssetVol* sgm_izcb;
		const DeterministicAssetVol* sgm_Xl;
		const DeterministicAssetVol* sgm_Dl;
		const DeterministicAssetVol* sgm_lstar;
		const DeterministicAssetVol* sgm_lzcb;
		firstIndex idx;
		secondIndex jdx;
		thirdIndex kdx;
		double now = timeline(0);
		double payoff_time = timeline(timeline.extent(firstDim)-1);
		double time_horizon = world.time_horizon();
		for (i=0;i<payoff_dimension;i++) {
			j = index(0,i);
			if (world.reportable_list[j].asset_index>=0) { // terminal fwd asset
				x(i) = world.get_terminal_forward_asset(world.reportable_list[j].currency_index,world.reportable_list[j].asset_index); }
			else {
				if (world.reportable_list[j].asset_index==-2) { // terminal forward exchange rate
					x(i) = world.get_forward_exchange_rate(world.reportable_list[j].currency_index,time_horizon); }
				else { // fwd zero coupon bond
					x(i)  = (world.get_economies())[world.reportable_list[j].currency_index]->initialTS->operator()(world.reportable_list[j].maturity);
					x(i) /= (world.get_economies())[world.reportable_list[j].currency_index]->initialTS->operator()(payoff_time);	  }}}
		for (j=0;j<payoff_dimension;j++) {  // distribution under the domestic terminal forward measure
			i = index(0,j);  // asset index
			k = index(1,j);  // time index
			// populate the covariance matrix
			sgm_Xi    = NULL;
			sgm_Di    = NULL;
			sgm_istar = NULL;
			sgm_izcb  = NULL;
			if (world.reportable_list[i].asset_index==-1) { // zero coupon bond
				sgm_izcb = &*((world.get_economies())[world.reportable_list[i].currency_index]->v); }
			else {
				if (world.reportable_list[i].asset_index>=0) { // terminal fwd asset
					sgm_Xi = &((world.get_economies())[world.reportable_list[i].currency_index]->underlying[world.reportable_list[i].asset_index]->GetVolatilityFunction()); 
					sgm_istar = &*((world.get_economies())[world.reportable_list[i].currency_index]->v); }
				else {
					if (world.reportable_list[i].asset_index==-2) { // terminal fwd exchange rate
						sgm_istar = &*((world.get_economies())[0]->v);
						sgm_Xi    = &*((world.get_FXvolatilities())[world.reportable_list[i].currency_index-1]);
						sgm_Di    = &*((world.get_economies())[world.reportable_list[i].currency_index]->v); }}}
			for (l=j;l<payoff_dimension;l++) {  
				sgm_Xl    = NULL;
				sgm_Dl    = NULL;
				sgm_lstar = NULL;
				sgm_lzcb  = NULL;
				if (world.reportable_list[index(0,l)].asset_index==-1) { // zero coupon bond
					sgm_lzcb = &*((world.get_economies())[world.reportable_list[index(0,l)].currency_index]->v); }
				else {
					if (world.reportable_list[index(0,l)].asset_index>=0) { // terminal fwd asset
						sgm_lstar = &*((world.get_economies())[world.reportable_list[index(0,l)].currency_index]->v);
						sgm_Xl    = &((world.get_economies())[world.reportable_list[index(0,l)].currency_index]->underlying[world.reportable_list[index(0,l)].asset_index]->GetVolatilityFunction()); }
					else {
						if (world.reportable_list[index(0,l)].asset_index==-2) { // terminal fwd exchange rate
							sgm_lstar = &*((world.get_economies())[0]->v);
							sgm_Xl    = &*((world.get_FXvolatilities())[world.reportable_list[index(0,l)].currency_index-1]);
							sgm_Dl    = &*((world.get_economies())[world.reportable_list[index(0,l)].currency_index]->v); }}}
				double tmp = 0.0;
				double t_upper = std::min(timeline(k),timeline(index(1,l)));
				double dt = t_upper-now;
				if (sgm_Xi) {
					if (sgm_Xl) {
						tmp += sgm_Xi->volproduct(now,dt,*sgm_Xl);
						tmp -= sgm_istar->bondvolproduct(now,dt,time_horizon,*sgm_Xl);
						tmp -= sgm_lstar->bondvolproduct(now,dt,time_horizon,*sgm_Xi);
						tmp += sgm_istar->bondbondvolproduct(now,dt,time_horizon,time_horizon,*sgm_lstar);
						if (sgm_Dl) {
							tmp += sgm_Dl->bondvolproduct(now,dt,time_horizon,*sgm_Xi); 
							tmp -= sgm_istar->bondbondvolproduct(now,dt,time_horizon,time_horizon,*sgm_Dl); }}
					if (sgm_lzcb) {
						tmp += sgm_lzcb->bondvolproduct(now,dt,world.reportable_list[index(0,l)].maturity,*sgm_Xi); 
						tmp -= sgm_lzcb->bondvolproduct(now,dt,payoff_time,*sgm_Xi); 
						tmp -= sgm_istar->bondbondvolproduct(now,dt,time_horizon,world.reportable_list[index(0,l)].maturity,*sgm_lzcb);
						tmp += sgm_istar->bondbondvolproduct(now,dt,time_horizon,payoff_time,*sgm_lzcb); }
					if (sgm_Di) {
						if (sgm_Xl) {
							tmp += sgm_Di->bondvolproduct(now,dt,time_horizon,*sgm_Xl);
							tmp -= sgm_lstar->bondbondvolproduct(now,dt,time_horizon,time_horizon,*sgm_Di);
							if (sgm_Dl) tmp += sgm_Dl->bondbondvolproduct(now,dt,time_horizon,time_horizon,*sgm_Di); }
						if (sgm_lzcb) {
							tmp += sgm_lzcb->bondbondvolproduct(now,dt,world.reportable_list[index(0,l)].maturity,time_horizon,*sgm_Di); 
							tmp -= sgm_lzcb->bondbondvolproduct(now,dt,payoff_time,time_horizon,*sgm_Di); }}}
				if (sgm_izcb) {
					if (sgm_Xl) {
						tmp += sgm_izcb->bondvolproduct(now,dt,world.reportable_list[i].maturity,*sgm_Xl);
						tmp -= sgm_lstar->bondbondvolproduct(now,dt,time_horizon,world.reportable_list[i].maturity,*sgm_izcb);
						if (sgm_Dl) tmp += sgm_Dl->bondbondvolproduct(now,dt,time_horizon,world.reportable_list[i].maturity,*sgm_izcb); 
						tmp -= sgm_izcb->bondvolproduct(now,dt,payoff_time,*sgm_Xl);
						tmp += sgm_lstar->bondbondvolproduct(now,dt,time_horizon,payoff_time,*sgm_izcb);
						if (sgm_Dl) tmp -= sgm_Dl->bondbondvolproduct(now,dt,time_horizon,payoff_time,*sgm_izcb); }
					if (sgm_lzcb) {
						tmp += sgm_lzcb->bondbondvolproduct(now,dt,world.reportable_list[index(0,l)].maturity,world.reportable_list[i].maturity,*sgm_izcb); 
						tmp -= sgm_lzcb->bondbondvolproduct(now,dt,world.reportable_list[index(0,l)].maturity,payoff_time,*sgm_izcb); 
						tmp -= sgm_lzcb->bondbondvolproduct(now,dt,payoff_time,world.reportable_list[i].maturity,*sgm_izcb); 
						tmp += sgm_lzcb->bondbondvolproduct(now,dt,payoff_time,payoff_time,*sgm_izcb); }}
				sgm(j,l) = sgm(l,j) = tmp; }
			mu(j) = - 0.5*sgm(j,j); 
			if ((payoff_time!=time_horizon)&&(world.reportable_list[i].asset_index!=-1)) {
				int fidx = 0;
				mu(j) -= ((world.get_economies())[fidx]->v)->bondvolproduct(now,timeline(k)-now,time_horizon,*sgm_Xi);
				mu(j) += ((world.get_economies())[fidx]->v)->bondvolproduct(now,timeline(k)-now,payoff_time,*sgm_Xi); 
				mu(j) += ((world.get_economies())[fidx]->v)->bondbondvolproduct(now,timeline(k)-now,time_horizon,time_horizon,*sgm_istar);
				mu(j) -= ((world.get_economies())[fidx]->v)->bondbondvolproduct(now,timeline(k)-now,payoff_time,time_horizon,*sgm_istar); 
				if (sgm_Di) {
					mu(j) -= ((world.get_economies())[fidx]->v)->bondbondvolproduct(now,timeline(k)-now,time_horizon,time_horizon,*sgm_Di);
					mu(j) += ((world.get_economies())[fidx]->v)->bondbondvolproduct(now,timeline(k)-now,payoff_time,time_horizon,*sgm_Di); }}
			// foreign terminal fwd asset, change to domestic terminal measure
			if ((world.reportable_list[i].asset_index>=0)&&(world.reportable_list[i].currency_index>0)) { 
				mu(j) -= ((world.get_economies())[world.reportable_list[i].currency_index]->v)->bondvolproduct(now,timeline(k)-now,time_horizon,*sgm_Xi);
				mu(j) -= ((world.get_FXvolatilities())[world.reportable_list[i].currency_index-1])->volproduct(now,timeline(k)-now,*sgm_Xi);
				mu(j) += ((world.get_economies())[0]->v)->bondvolproduct(now,timeline(k)-now,time_horizon,*sgm_Xi);
				if (sgm_Di) {
					mu(j) -= ((world.get_economies())[world.reportable_list[i].currency_index]->v)->bondbondvolproduct(now,timeline(k)-now,time_horizon,time_horizon,*sgm_Di);
					mu(j) -= sgm_Di->bondvolproduct(now,timeline(k)-now,time_horizon,*((world.get_FXvolatilities())[world.reportable_list[i].currency_index-1]));
					mu(j) += ((world.get_economies())[0]->v)->bondbondvolproduct(now,timeline(k)-now,time_horizon,time_horizon,*sgm_Di); }
				mu(j) += ((world.get_economies())[world.reportable_list[i].currency_index]->v)->bondbondvolproduct(now,timeline(k)-now,time_horizon,time_horizon,*sgm_istar);
				mu(j) += sgm_istar->bondvolproduct(now,timeline(k)-now,time_horizon,*((world.get_FXvolatilities())[world.reportable_list[i].currency_index-1]));
				mu(j) -= ((world.get_economies())[0]->v)->bondbondvolproduct(now,timeline(k)-now,time_horizon,time_horizon,*sgm_istar); }
			// foreign zero coupon bond, change to domestic payoff measure
			if ((world.reportable_list[i].asset_index==-1)&&(world.reportable_list[i].currency_index>0)) { 
				mu(j) -= ((world.get_economies())[world.reportable_list[i].currency_index]->v)->bondbondvolproduct(now,timeline(k)-now,payoff_time,world.reportable_list[i].maturity,*sgm_izcb);
				mu(j) -= sgm_izcb->bondvolproduct(now,timeline(k)-now,world.reportable_list[i].maturity,*((world.get_FXvolatilities())[world.reportable_list[i].currency_index-1]));
				mu(j) += ((world.get_economies())[0]->v)->bondbondvolproduct(now,timeline(k)-now,payoff_time,world.reportable_list[i].maturity,*sgm_izcb);
				mu(j) += ((world.get_economies())[world.reportable_list[i].currency_index]->v)->bondbondvolproduct(now,timeline(k)-now,payoff_time,payoff_time,*sgm_izcb);
				mu(j) += sgm_izcb->bondvolproduct(now,timeline(k)-now,payoff_time,*((world.get_FXvolatilities())[world.reportable_list[i].currency_index-1]));
				mu(j) -= ((world.get_economies())[0]->v)->bondbondvolproduct(now,timeline(k)-now,payoff_time,payoff_time,*sgm_izcb); }}
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
}

