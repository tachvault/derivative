/*
    Copyright (C) Nathan Muruganantha 2015
*/

#ifndef _DERIVATIVE_COMPARISON_H
#define _DERIVATIVE_COMPARISON_H

/// floating - point comparisons
namespace derivative
{

    /*! Follows somewhat the advice of Knuth on checking for floating-point
        equality. The closeness relationship is:
        \f[
        \mathrm{close}(x,y,n) \equiv |x-y| \leq \varepsilon |x|
                              \wedge |x-y| \leq \varepsilon |y|
        \f]
        where \f$ \varepsilon \f$ is \f$ n \f$ times the machine accuracy;
        \f$ n \f$ equals 42 if not given.
    */
    bool close(double x, double y);
    bool close(double x, double y, std::size_t n);

    /*! Follows somewhat the advice of Knuth on checking for floating-point
        equality. The closeness relationship is:
        \f[
        \mathrm{close}(x,y,n) \equiv |x-y| \leq \varepsilon |x|
                                \vee |x-y| \leq \varepsilon |y|
        \f]
        where \f$ \varepsilon \f$ is \f$ n \f$ times the machine accuracy;
        \f$ n \f$ equals 42 if not given.
    */
    bool close_enough(double x, double y);
    bool close_enough(double x, double y, std::size_t n);


    // inline definitions

    inline bool close(double x, double y)
	{
        return close(x,y,42);
    }

    inline bool close(double x, double y, std::size_t n) 
	{
        // Deals with +infinity and -infinity representations etc.
        if (x == y)
            return true;

		double diff = std::fabs(x - y), tolerance = n * ((std::numeric_limits<double>::epsilon)());

        if (x * y == 0.0) // x or y = 0.0
            return diff < (tolerance * tolerance);

        return diff <= tolerance*std::fabs(x) &&
               diff <= tolerance*std::fabs(y);
    }

    inline bool close_enough(double x, double y) 
	{
        return close_enough(x,y,42);
    }

    inline bool close_enough(double x, double y, std::size_t n) 
	{
        // Deals with +infinity and -infinity representations etc.
        if (x == y)
            return true;

		double diff = std::fabs(x - y), tolerance = n * ((std::numeric_limits<double>::epsilon)());

        if (x * y == 0.0) // x or y = 0.0
            return diff < (tolerance * tolerance);

        return diff <= tolerance*std::fabs(x) ||
               diff <= tolerance*std::fabs(y);
    }



    //! compare two objects by date
    /*! There is no generic implementation of this struct.
        Template specializations will have to be defined for
        each needed type (see CashFlow for an example.)
    */
    template <class T> struct earlier_than;

    /* partial specialization for shared pointers, forwarding to their
       pointees. */
    template <class T>
    struct earlier_than<std::shared_ptr<T> >
        : std::binary_function<std::shared_ptr<T>,
                               std::shared_ptr<T>,
                               bool> 
	{
        bool operator()(const std::shared_ptr<T>& x,
                        const std::shared_ptr<T>& y) 
		{
            return earlier_than<T>()(*x,*y);
        }
    };
}
/* namespace derivative */

#endif /* _DERIVATIVE_COMPARISON_H */

