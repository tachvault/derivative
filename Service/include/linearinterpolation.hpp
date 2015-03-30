/*
Copyright (C) Nathan Muruganantha 2015
Initial versions:  Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2008 StatPro Italia srl
*/

#ifndef _DERIVATIVE_LINEARINTERPOLATION_P
#define _DERIVATIVE_LINEARINTERPOLATION_P

#include <vector>
#include "interpolation.hpp"

/// linear interpolation between discrete points
namespace derivative
{
    namespace detail 
	{
		template <class I1, class I2>
        class LinearInterpolationImpl
            : public Interpolation::templateImpl<I1,I2> 
		{
          public:
            LinearInterpolationImpl(const I1& xBegin, const I1& xEnd,const I2& yBegin)
            : Interpolation::templateImpl<I1,I2>(xBegin, xEnd, yBegin),
              primitiveConst_(xEnd-xBegin), s_(xEnd-xBegin)
			{}
            void update() 
			{
                primitiveConst_[0] = 0.0;
                for (std::size_t i=1; i<std::size_t(this->xEnd_-this->xBegin_); ++i) 
				{
                    double dx = this->xBegin_[i]-this->xBegin_[i-1];
                    s_[i-1] = (this->yBegin_[i]-this->yBegin_[i-1])/dx;
                    primitiveConst_[i] = primitiveConst_[i-1]
                        + dx*(this->yBegin_[i-1] +0.5*dx*s_[i-1]);
                }
            }
            double value(double x) const 
			{
                std::size_t i = this->locate(x);
                return this->yBegin_[i] + (x-this->xBegin_[i])*s_[i];
            }
            double primitive(double x) const 
			{
                std::size_t i = this->locate(x);
                double dx = x-this->xBegin_[i];
                return primitiveConst_[i] +
                    dx*(this->yBegin_[i] + 0.5*dx*s_[i]);
            }
            double derivative(double x) const 
			{
                std::size_t i = this->locate(x);
                return s_[i];
            }
            double secondDerivative(double) const 
			{
                return 0.0;
            }
          private:
            std::vector<double> primitiveConst_, s_;
        };

    }

    //! %Linear interpolation between discrete points
    class LinearInterpolation : public Interpolation 
	{
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        LinearInterpolation(const I1& xBegin, const I1& xEnd,
                            const I2& yBegin) {
            impl_ = std::shared_ptr<Interpolation::Impl>(new
                detail::LinearInterpolationImpl<I1,I2>(xBegin, xEnd, yBegin));
            impl_->update();
        }
    };

    //! %Linear-interpolation factory and traits
    class Linear 
	{
      public:
        template <class I1, class I2>
        Interpolation interpolate(const I1& xBegin, const I1& xEnd,
                                  const I2& yBegin) const 
		{
            return LinearInterpolation(xBegin, xEnd, yBegin);
        }
        static const bool global = false;
        static const std::size_t requiredPoints = 2;
    };
}

/* namespace derivative */
#endif /* _DERIVATIVE_LINEARINTERPOLATION_H */

