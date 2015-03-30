/*
Copyright (C) Nathan Muruganantha 2015
Copyright (C) 2002, 2003 Ferdinando Ametrano
Copyright (C) 2004 StatPro Italia srl
*/

#ifndef _DERIVATIVE_BILINEARINTERPOLATION_H
#define _DERIVATIVE_BILINEARINTERPOLATION_H

/// bilinear interpolation between discrete points
#include "interpolation2d.hpp"

namespace derivative 
{
	namespace detail 
	{
		template <class I1, class I2, class M>
		class BilinearInterpolationImpl
			: public Interpolation2D::templateImpl<I1, I2, M> 
		{
		public:
			BilinearInterpolationImpl(const I1& xBegin, const I1& xEnd,
				const I2& yBegin, const I2& yEnd,
				const M& zData)
				: Interpolation2D::templateImpl<I1, I2, M>(xBegin, xEnd,
				yBegin, yEnd,
				zData)
			{
				calculate();
			}
			void calculate()
			{}
			double value(double x, double y) const 
			{
				std::size_t i = this->locateX(x), j = this->locateY(y);

				double z1 = this->zData_[j][i];
				double z2 = this->zData_[j][i + 1];
				double z3 = this->zData_[j + 1][i];
				double z4 = this->zData_[j + 1][i + 1];

				double t = (x - this->xBegin_[i]) /
					(this->xBegin_[i + 1] - this->xBegin_[i]);
				double u = (y - this->yBegin_[j]) /
					(this->yBegin_[j + 1] - this->yBegin_[j]);

				return (1.0 - t)*(1.0 - u)*z1 + t*(1.0 - u)*z2
					+ (1.0 - t)*u*z3 + t*u*z4;
			}
		};

	}

	//! %bilinear interpolation between discrete points
	class BilinearInterpolation : public Interpolation2D 
	{
	public:
		/*! \pre the \f$ x \f$ and \f$ y \f$ values must be sorted. */
		template <class I1, class I2, class M>
		BilinearInterpolation(const I1& xBegin, const I1& xEnd,
			const I2& yBegin, const I2& yEnd,
			const M& zData) 
		{
			impl_ = boost::shared_ptr<Interpolation2D::Impl>(
				new detail::BilinearInterpolationImpl<I1, I2, M>(xBegin, xEnd,
				yBegin, yEnd,
				zData));
		}
	};

	//! bilinear-interpolation factory
	class Bilinear 
	{
	public:
		template <class I1, class I2, class M>
		Interpolation2D interpolate(const I1& xBegin, const I1& xEnd,
			const I2& yBegin, const I2& yEnd,
			const M& z) const 
		{
			return BilinearInterpolation(xBegin, xEnd, yBegin, yEnd, z);
		}
	};
}

/* namespace derivative */
#endif /* _DERIVATIVE_BILINEARINTERPOLATION_H */

