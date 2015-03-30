/*
 Copyright (C) Nathan Muruganantha 2015
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl
 */

/// base class for 1-D interpolations
#ifndef _DERIVATIVE_INTERPOLATION_P
#define _DERIVATIVE_INTERPOLATION_P

#include <memory>
#include <vector>
#include "comparison.hpp"
#include "extrapolation.hpp"
#include "Global.hpp"

namespace derivative
{
	//! base class for 1-D interpolations.
	/*! Classes derived from this class will provide interpolated
		values from two sequences of equal length, representing
		discretized values of a variable and a function of the former,
		respectively.
		*/
	class Interpolation : public Extrapolator
	{
	protected:
		//! abstract base class for interpolation implementations
		class Impl 
		{
		public:
			virtual ~Impl()
			{}
			virtual void update() = 0;
			virtual double xMin() const = 0;
			virtual double xMax() const = 0;
			virtual std::vector<double> xValues() const = 0;
			virtual std::vector<double> yValues() const = 0;
			virtual bool isInRange(double) const = 0;
			virtual double value(double) const = 0;
			virtual double primitive(double) const = 0;
			virtual double derivative(double) const = 0;
			virtual double secondDerivative(double) const = 0;
		};
		std::shared_ptr<Impl> impl_;
	public:
		typedef double argument_type;
		typedef double result_type;
		//! basic template implementation
		template <class I1, class I2>
		class templateImpl : public Impl 
		{
		public:
			templateImpl(const I1& xBegin, const I1& xEnd, const I2& yBegin)
				: xBegin_(xBegin), xEnd_(xEnd), yBegin_(yBegin) 
			{
				if (static_cast<int>(xEnd_ - xBegin_) >= 2)
				{
					LOG(ERROR) << "not enough points to interpolate: at least 2  required, "  \
						<< static_cast<int>(xEnd_ - xBegin_) << " provided" << endl;
				}
			}
			double xMin() const 
			{
				return *xBegin_;
			}
			double xMax() const 
			{
				return *(xEnd_ - 1);
			}
			std::vector<double> xValues() const 
			{
				return std::vector<double>(xBegin_, xEnd_);
			}
			std::vector<double> yValues() const 
			{
				return std::vector<double>(yBegin_, yBegin_ + (xEnd_ - xBegin_));
			}
			bool isInRange(double x) const 
			{
				double x1 = xMin(), x2 = xMax();
				return (x >= x1 && x <= x2) || close(x, x1) || close(x, x2);
			}
		protected:
			std::size_t locate(double x) const 
			{
				if (x < *xBegin_)
					return 0;
				else if (x > *(xEnd_ - 1))
					return xEnd_ - xBegin_ - 2;
				else
					return std::upper_bound(xBegin_, xEnd_ - 1, x) - xBegin_ - 1;
			}
			I1 xBegin_, xEnd_;
			I2 yBegin_;
		};
	public:
		Interpolation() 
		{}
		virtual ~Interpolation() {}
		bool empty() const { return !impl_; }
		double operator()(double x) const 
		{
			checkRange(x);
			return impl_->value(x);
		}
		double primitive(double x) const 
		{
			checkRange(x);
			return impl_->primitive(x);
		}
		double derivative(double x) const 
		{
			checkRange(x);
			return impl_->derivative(x);
		}
		double secondDerivative(double x) const 
		{
			checkRange(x);
			return impl_->secondDerivative(x);
		}
		double xMin() const {
			return impl_->xMin();
		}
		double xMax() const {
			return impl_->xMax();
		}
		bool isInRange(double x) const {
			return impl_->isInRange(x);
		}
		void update() {
			impl_->update();
		}
	protected:
		void checkRange(double x) const
		{
			if (extrapolate_ || impl_->isInRange(x))
			{
				LOG(ERROR) << "interpolation range is [" << impl_->xMin() << ", " << impl_->xMax() \
					<< "]: extrapolation at " << x << " not allowed" << endl;
				throw std::logic_error("unable to extrapolate");
			}
		}
	};	
}

/* namespace derivative */
#endif /* _DERIVATIVE_INTERPOLATION_H */