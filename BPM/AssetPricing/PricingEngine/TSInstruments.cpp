/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#include "TSInstruments.hpp"
#include "QFArrayUtil.hpp"

namespace derivative
{
	double Caplet::payoff(const TermStructure& ts)
	{
		double bond = ts(maturity() + delta);
		return notional * std::max(0.0, 1.0 - bond*(1.0 + delta*lvl));
	}

	double Floorlet::payoff(const TermStructure& ts)
	{
		double bond = ts(maturity() + delta);
		return notional * std::max(0.0, -(1.0 - bond*(1.0 + delta*lvl)));
	}

	Swaption::Swaption(double xp, double xt, double xT, double xlvl, double xdelta, int nperiods, int xsign, double xnotional)
		: TSEuropeanInstrument(xp, xt, xT), lvl(xlvl), notional(xnotional), tenor(nperiods + 1), sign(xsign)
	{
		firstIndex idx;
		tenor = idx*xdelta + xT;
	}

	double Swaption::payoff(const TermStructure& ts)
	{
		double result = std::max(0.0, sign*(lvl - ts.swap(tenor)));
		if (result > 0.0) result *= ts.pvbp(tenor);
		return result;
	}

	int TSBermudanInstrument::exercise_date(double now) const
	{
		return find_first(now, T);
	}

	BermudanSwaption::BermudanSwaption(double xp, double xt, double xT, double xlvl, double xdelta, int nperiods, int xsign, double xnotional)
		: TSBermudanInstrument(xp, xt, nperiods), lvl(xlvl), notional(xnotional), tenor(nperiods + 1), sign(xsign)
	{
		firstIndex idx;
		tenor = idx*xdelta + xT;
		T = tenor(Range(0, nperiods - 1));
	}

	double BermudanSwaption::payoff(double continuation_value, const TermStructure& ts)
	{
		double now = ts.timeline()(0);
		double result = continuation_value;
		int exercise_idx = exercise_date(now);
		if (exercise_idx >= 0) {
			Array<double, 1> tmp_tenor(tenor(Range(exercise_idx, toEnd)));
			double val = sign*(lvl - ts.swap(tmp_tenor)) * ts.pvbp(tmp_tenor);
			result = std::max(result, val);
		}
		return result;
	}

	BarrierInstrument::BarrierInstrument(double xp, double xt, TSEuropeanInstrument& xinstrument, const Array<double, 1>& barrier_monitoring_dates, double xbarrier, double xbarrier_ttm, int xdirection)
		: TSBermudanInstrument(xp, xt, barrier_monitoring_dates.extent(firstDim)), instrument(xinstrument), barrier(xbarrier), direction(xdirection), barrier_ttm(xbarrier_ttm)
	{
		T = barrier_monitoring_dates;
		if (instrument.maturity() != T(T.extent(firstDim) - 1)) throw std::logic_error("BarrierInstrument: last barrier monitoring date must coincide with the maturity of the payoff");
	}

	double BarrierInstrument::payoff(double continuation_value, const TermStructure& ts)
	{
		double now = ts.timeline()(0);
		double result = continuation_value;
		if (now == instrument.maturity()) result = instrument.payoff(ts);
		int exercise_idx = exercise_date(now);
		if (exercise_idx >= 0) {  // check for knock-out
			if (0.0 >= direction*(barrier - ts.simple_rate(now, barrier_ttm))) result = 0.0;
		}
		return result;
	}
}

