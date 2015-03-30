/*
Copyright (C) Nathan Muruganantha 2015
Initial versions:  Copyright (C) 2002, 2003, 2006 Ferdinando Ametrano
Copyright (C) 2004, 2005, 2006, 2007 StatPro Italia srl
*/

/// abstract base classes for 2-D interpolations
#ifndef _DERIVATIVE_INTERPOLATION2D_H
#define _DERIVATIVE_INTERPOLATION2D_H

#include <memory>
#include <vector>
#include "extrapolation.hpp"
#include "comparison.hpp"
#include "Global.hpp"

namespace derivative
{	
	//! base class for 2-D interpolations.
	/*! Classes derived from this class will provide interpolated
		values from two sequences of length \f$ N \f$ and \f$ M \f$,
		representing the discretized values of the \f$ x \f$ and \f$ y
		\f$ variables, and a \f$ N \times M \f$ matrix representing
		the tabulated function values.
		*/

	class Interpolation2D : public Extrapolator
	{
	protected:
		//! abstract base class for 2-D interpolation implementations
		class Impl
		{
		public:
			virtual ~Impl() 
			{}
			virtual void calculate() = 0;
			virtual double xMin() const = 0;
			virtual double xMax() const = 0;
			virtual std::vector<double> xValues() const = 0;
			virtual std::size_t locateX(double x) const = 0;
			virtual double yMin() const = 0;
			virtual double yMax() const = 0;
			virtual std::vector<double> yValues() const = 0;
			virtual std::size_t locateY(double y) const = 0;
			//virtual const Matrix& zData() const = 0;
			virtual bool isInRange(double x, double y) const = 0;
			virtual double value(double x, double y) const = 0;
		};
		std::shared_ptr<Impl> impl_;
	public:
		typedef double first_argument_type;
		typedef double second_argument_type;
		typedef double result_type;
		//! basic template implementation
		template <class I1, class I2, class M>
		class templateImpl : public Impl 
		{
		public:
			templateImpl(const I1& xBegin, const I1& xEnd,
				const I2& yBegin, const I2& yEnd,
				const M& zData)
				: xBegin_(xBegin), xEnd_(xEnd), yBegin_(yBegin), yEnd_(yEnd),
				zData_(zData) 
			{
				if (xEnd_ - xBegin_ >= 2)
				{
					LOG(ERROR) << "not enough x points to interpolate: at least 2 " \
						<< "required, " << xEnd_ - xBegin_ << " provided" << endl;

					throw std::logic_error("not enough points to interpolate");
				}

				if (yEnd_ - yBegin_ >= 2)
				{
					LOG(ERROR) << "not enough y points to interpolate: at least 2 required, "  \
						<< yEnd_ - yBegin_ << " provided" << endl;

					throw std::logic_error("not enough points to interpolate");
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
			double yMin() const 
			{
				return *yBegin_;
			}
			double yMax() const 
			{
				return *(yEnd_ - 1);
			}
			std::vector<double> yValues() const 
			{
				return std::vector<double>(yBegin_, yEnd_);
			}
			const Matrix& zData() const 
			{
				return zData_;
			}
			bool isInRange(double x, double y) const 
			{
				double x1 = xMin(), x2 = xMax();
				bool xIsInrange = (x >= x1 && x <= x2) ||
					close(x, x1) ||
					close(x, x2);
				if (!xIsInrange) return false;
				double y1 = yMin(), y2 = yMax();
				return (y >= y1 && y <= y2) || close(y, y1) || close(y, y2);
			}
		protected:
			std::size_t locateX(double x) const 
			{
				if (x < *xBegin_)
					return 0;
				else if (x > *(xEnd_ - 1))
					return xEnd_ - xBegin_ - 2;
				else
					return std::upper_bound(xBegin_, xEnd_ - 1, x) - xBegin_ - 1;
			}
			std::size_t locateY(double y) const 
			{
				if (y < *yBegin_)
					return 0;
				else if (y > *(yEnd_ - 1))
					return yEnd_ - yBegin_ - 2;
				else
					return std::upper_bound(yBegin_, yEnd_ - 1, y) - yBegin_ - 1;
			}
			I1 xBegin_, xEnd_;
			I2 yBegin_, yEnd_;
			const M& zData_;
		};
	public:
		Interpolation2D() 
		{}
		double operator()(double x, double y) const 
		{
			checkRange(x, y);
			return impl_->value(x, y);
		}
		double xMin() const 
		{
			return impl_->xMin();
		}
		double xMax() const 
		{
			return impl_->xMax();
		}
		std::vector<double> xValues() const 
		{
			return impl_->xValues();
		}
		std::size_t locateX(double x) const 
		{
			return impl_->locateX(x);
		}
		double yMin() const 
		{
			return impl_->yMin();
		}
		double yMax() const 
		{
			return impl_->yMax();
		}
		std::vector<double> yValues() const 
		{
			return impl_->yValues();
		}
		std::size_t locateY(double y) const 
		{
			return impl_->locateY(y);
		}
		/*const Matrix& zData() const 
		{
			return impl_->zData();
		}*/
		bool isInRange(double x, double y) const 
		{
			return impl_->isInRange(x, y);
		}
		void update() {
			impl_->calculate();
		}
	protected:
		void checkRange(double x, double y) const
		{
			if (extrapolate_ || impl_->isInRange(x, y))
			{
				LOG(ERROR) << "interpolation range is [" << impl_->xMin() << ", " << impl_->xMax() \
					<< "] x [" << impl_->yMin() << ", " << impl_->yMax() << "]: extrapolation at (" \
					<< x << ", " << y << ") not allowed" << endl;

				throw std::logic_error("expolation is allowed");
			}
		}
	};	
}
/* namespace derivative */

#endif /* _DERIVATIVE_INTERPOLATION2D_H */