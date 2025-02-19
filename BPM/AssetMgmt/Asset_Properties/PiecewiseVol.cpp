/*
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schl�gl
*/

#include "PiecewiseVol.hpp"
#include "QFArrayUtil.hpp"
#include "DeterministicVolMediator.hpp"

namespace derivative
{
	/// Constructor.
	PiecewiseConstVol::PiecewiseConstVol(const Array<double, 1>& xT, ///< Time line defining calendar time segments on which parameters are constant.
		const Array<double, 2>& xv  ///< Matrix of volatility scale parameters. Dimensions: time segment (row) by driving factor (column).
		)
		: timeline(xT), v(xv), vol_sq(xv.extent(firstDim))
	{
		int i, j;
		for (i = 0; i < v.extent(firstDim); i++)
		{
			vol_sq(i) = 0.0;
			for (j = 0; j < v.extent(secondDim); j++) vol_sq(i) += v(i, j)*v(i, j);
		}
	}

	/// clone this object
	std::shared_ptr<DeterministicAssetVol> PiecewiseConstVol::Clone() const
	{
		std::shared_ptr<DeterministicAssetVol> obj = \
			std::make_shared<PiecewiseConstVol>(timeline.copy(), v.copy());
		return obj;
	}

	std::shared_ptr<DeterministicAssetVol> PiecewiseConstVol::component_vol(int i) const
	{
		Array<double, 2> cv(v.extent(firstDim), v.extent(secondDim)); // Matrix of volatility scale parameters. Dimensions: time segment (row) by driving factor (column).
		cv = 0.0;
		cv(blitz::Range::all(), i) = v(blitz::Range::all(), i);
		std::shared_ptr<DeterministicAssetVol> result = std::make_shared<PiecewiseConstVol>(timeline, cv);
		return result;
	}

	bool PiecewiseConstVol::get_volatility_level(double t, double T, Array<double, 1>& vol_lvl) const
	{
		if (vol_lvl.extent(firstDim) != v.extent(secondDim)) throw std::logic_error("Volatility dimension mismatch");
		int i = find_segment(t, timeline);
		if (t == timeline(i + 1)) i++;
		int j = find_segment(T, timeline);
		if ((i != j) && (t != T))
		{
			return false;
		}
		vol_lvl = v(i, blitz::Range::all());
		return true;
	}

	/// The integral over the scalar product between two piecewise constant volatility vectors.
	double PiecewiseConstVol::volproduct(double t, double dt, const DeterministicAssetVol& xv) const
	{
		/// local Array used in this function. To eliminate multiple allocation, deallocation defined
		/// in class scope.
		if (lvl.extent(firstDim) != v.extent(secondDim)) lvl.resize(v.extent(secondDim));

		/// Temp Array used to return result in integrate member function.
		if (temp.extent(firstDim) != v.extent(secondDim)) temp.resize(v.extent(secondDim));

		double result = 0.0;
		int    i = find_segment(t, timeline);
		double covered = 0.0;
		while ((covered < dt - 1e-12) && (i < v.extent(firstDim)))
		{
			double tstep = std::min(timeline(i + 1) - std::max(t, timeline(i)), dt - covered);
			lvl = v(i, blitz::Range::all());
			result += DeterministicVolMediator::volproduct_ConstVol(t + covered, tstep, lvl, xv, temp);
			covered += tstep;
			i++;
		}
		if (covered < dt - 1e-12) throw std::logic_error("Cannot extrapolate");
		return result;
	}

	Array<double, 1> PiecewiseConstVol::integral(double t, double dt) const
	{
		Array<double, 1> result(v.extent(secondDim));
		result = 0.0;
		int    i = find_segment(t, timeline);
		double covered = 0.0;
		while ((covered < dt - 1e-12) && (i < v.extent(firstDim)))
		{
			double tstep = std::min(timeline(i + 1) - std::max(t, timeline(i)), dt - covered);
			result += tstep * v(i, blitz::Range::all());
			covered += tstep;
			i++;
		}
		if (covered < dt - 1e-12) throw std::logic_error("Cannot extrapolate");
		return result;
	}

	void PiecewiseConstVol::integral(double t, double dt, Array<double, 1>& result) const
	{
		result = 0.0;
		int    i = find_segment(t, timeline);
		double covered = 0.0;
		while ((covered < dt - 1e-12) && (i < v.extent(firstDim)))
		{
			double tstep = std::min(timeline(i + 1) - std::max(t, timeline(i)), dt - covered);
			result += tstep * v(i, blitz::Range::all());
			covered += tstep;
			i++;
		}
		if (covered < dt - 1e-12) throw std::logic_error("Cannot extrapolate");
	}

	void PiecewiseConstVol::interpolate(const std::shared_ptr<DeterministicAssetVol>& neibor, double factor)
	{
		/// throw exception if number of factors are different
		if (factors() != neibor->factors())
		{
			throw std::logic_error("mismatch factors");
		}
	
		Array<double, 1> merged_T = unique_merge(timeline, dynamic_pointer_cast<PiecewiseConstVol>(neibor)->timeline);
		Array<double, 2> merged_v(merged_T.extent(firstDim) -1, v.extent(secondDim));
		for (int i = 0; i < merged_T.size() - 1; ++i)
		{
			Array<double, 1> vol_lvl_this(v.extent(secondDim));
			Array<double, 1> vol_lvl_neibor(v.extent(secondDim));
			
			bool status_this = false;
			bool status_neibor = false;
			try
			{
				status_this = get_volatility_level(merged_T(i), merged_T(i + 1), vol_lvl_this);
			}
			catch (std::out_of_range& e)
			{}
			try
			{
				status_neibor = neibor->get_volatility_level(merged_T(i), merged_T(i + 1), vol_lvl_neibor);
			}
			catch (std::out_of_range& e)
			{}
			if (status_this && status_neibor)
			{
				for (int j = 0; j < factors(); ++j)
				{
					merged_v(j, i) = (1-factor)*vol_lvl_this(j) + factor*vol_lvl_neibor(j);
				}
			}
			else if (status_this)
			{
				for (int j = 0; j < factors(); ++j)
				{
					merged_v(j, i) = vol_lvl_this(j);
				}
		    }
			else if (status_neibor)
			{
				for (int j = 0; j < factors(); ++j)
				{
					merged_v(j, i) = vol_lvl_neibor(j);
				}
			}
			else
			{
				throw std::logic_error("Cannot extrapolate");
			}
		}

		timeline.resize(merged_T.extent(firstDim));
		timeline = merged_T.copy();
		v.resize(merged_v.extent(firstDim), merged_v.extent(secondDim));
		v = merged_v.copy();
		vol_sq.resize(merged_v.extent(firstDim));
		int i, j;
		for (i = 0; i < v.extent(firstDim); i++)
		{
			vol_sq(i) = 0.0;
			for (j = 0; j < v.extent(secondDim); j++) vol_sq(i) += v(i, j)*v(i, j);
		}
	}

	int PiecewiseConstVol::factors() const
	{
		return v.extent(secondDim);
	}

	double PiecewiseConstVol::timeframe() const
	{
		return timeline(timeline.extent(firstDim) - 1);
	}

	/// Integral over the square of the forward zero coupon bond volatility.
	double PiecewiseConstVol::FwdBondVol(double t, double T1, double T2) const
	{
		throw std::logic_error("Can't do PiecewiseConstVol::FwdBondVol() yet");
	}

	Array<double, 1> PiecewiseConstVol::segments(double t, double dt) const
	{
		Array<double, 1> tmp(2);
		tmp(0) = t;
		tmp(1) = t + dt;
		int i = find_segment(t, timeline);
		int j = find_segment(t + dt, timeline);
		Array<double, 1> tmp2(timeline(blitz::Range(i + 1, j)));
		return unique_merge(tmp, tmp2);
	}

	/// Integral over the scalar product between the bond volatility given by (*this) and a deterministic asset volatility.
	double PiecewiseConstVol::bondvolproduct(double t, double dt, double bondmat, const DeterministicAssetVol& xv) const
	{
		throw std::logic_error("Can't do PiecewiseConstVol::bondvolproduct() yet");
	}

	/** Integral over the scalar product between the bond volatility given by (*this) and a
	bond volatility given by another DeterministicVol object. */
	double PiecewiseConstVol::bondbondvolproduct(double t, double dt, double T1, double T2, const DeterministicAssetVol& xv) const
	{
		throw std::logic_error("Can't do PiecewiseConstVol::bondbondvolproduct() yet");
	}

	/// Function needed for the exponential-affine representation of zero coupon bond prices.
	double PiecewiseConstVol::A(double t, double T) const
	{
		throw std::logic_error("PiecewiseConstVol::A() not yet implemented");
	}

	/// Function needed for the exponential-affine representation of zero coupon bond prices.
	Array<double, 1> PiecewiseConstVol::B(double t, double T) const
	{
		throw std::logic_error("PiecewiseConstVol::B() not yet implemented");
	}

	/// For Monte Carlo simulation: variance of increment from t to dt of state variable i.
	double PiecewiseConstVol::var(int i, double t, double dt) const
	{
		throw std::logic_error("PiecewiseConstVol::var() not implemented");
		return 0.0;
	}

	/// For Monte Carlo simulation: covariance of increment from t to dt of state variable i with increment of Brownian motion \f$ \Delta W^{(i)} \f$.
	double PiecewiseConstVol::covar_dW(int i, double t, double dt) const
	{
		throw std::logic_error("PiecewiseConstVol::covar_dW() not implemented");
		return 0.0;
	}

	Array<double, 1> PiecewiseConstVol::bondvolintegral(double t, double dt, double T) const
	{
		Array<double, 1> result(1);
		throw std::logic_error("PiecewiseConstVol::bondvolintegral() not implemented");
		return result;
	}

	Array<double, 1> PiecewiseConstVol::StateVariableMean(double t, double dt, double T) const
	{
		Array<double, 1> result(1);
		throw std::logic_error("PiecewiseConstVol::StateVariableMean() not implemented");
		return result;
	}
}


