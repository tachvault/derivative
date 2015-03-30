/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#include <stdexcept>
#include <boost/bind.hpp>
#include "LongstaffSchwartz.hpp"

namespace derivative
{
	LongstaffSchwartzExerciseBoundary1D::LongstaffSchwartzExerciseBoundary1D(const Array<double,1>& timeline,
		const TermStructure& xts,
		const Array<double,2>& state_variable_paths,
		std::function<double (double)> payoff,
		int degree,
		bool positive_payoff_only)
		: T(timeline),ts(xts),f(payoff),positive_only(positive_payoff_only)
	{
		int i,j;
		int N = state_variable_paths.extent(secondDim);
		if (N!=T.extent(firstDim)) throw std::logic_error("Dimension mismatch in LongstaffSchwartzExerciseBoundary1D");
		int npaths = state_variable_paths.extent(firstDim);
		int nval = npaths;
		Array<double,1> continuation_value(npaths),early_exercise(npaths);
		double discount = ts(T(N-1))/ts(T(N-2));
		for (i=0;i<npaths;i++) continuation_value(i) = discount * payoff(state_variable_paths(i,N-1));
		for (j=N-2;j>0;j--) 
		{
			for (i=0;i<npaths;i++) early_exercise(i) = payoff(state_variable_paths(i,j));
			// eliminate non-positive early exercise values if necessary
			if (positive_payoff_only) 
			{
				nval = 0;
				for (i=0;i<npaths;i++) 
				{
					if (early_exercise(i)>0.0) 
					{
						nval++; 
					}
				}
			}
			Array<double,1> x(nval),y(nval);
			if (positive_payoff_only) 
			{
				int c = 0;
				for (i=0;i<npaths;i++) 
				{
					if (early_exercise(i)>0.0) 
					{
						x(c) = state_variable_paths(i,j);
						y(c) = continuation_value(i);
						c++; 
					}
				}
			}
			else
			{
				x = state_variable_paths(Range::all(),j);
				y = continuation_value; 
			}
			// do regression           
			std::shared_ptr<PolynomialLeastSquaresFit> newfit = std::make_shared<PolynomialLeastSquaresFit>(y,x,degree);
			if (!newfit) throw std::logic_error("Unable to do regression in LongstaffSchwartzExerciseBoundary1D");                       
			fit.insert(fit.begin(),newfit);
			// update continuation values
			discount = ts(T(j))/ts(T(j-1));
			for (i=0;i<npaths;i++) continuation_value(i) = discount * std::max(early_exercise(i),newfit->predict(state_variable_paths(i,j))); }
	}

	LongstaffSchwartzExerciseBoundary1D::~LongstaffSchwartzExerciseBoundary1D()
	{
		int i;
		for (i=0;i<(int)fit.size();i++)
		{
			if (fit[i])
			{
				fit.erase(fit.begin() + i);
			}
		}
	}

	double LongstaffSchwartzExerciseBoundary1D::apply(const Array<double,1>& path) const
	{
		int i;
		double result = 0.0;
		int N = path.extent(firstDim);
		if (N!=T.extent(firstDim)) throw std::logic_error("Dimension mismatch in LongstaffSchwartzExerciseBoundary1D");
		for (i=1;i<N-1;i++) 
		{
			double payoff = f(path(i));
			if ((payoff>0.0)||(!positive_only)) 
			{
				if (payoff>fit[i-1]->predict(path(i))) 
				{
					result = payoff * ts(T(i));
					return result; 
				}
			}
		}
		result = f(path(N-1)) * ts(T(N-1));
		return result;               
	}

	LongstaffSchwartzExerciseBoundary::LongstaffSchwartzExerciseBoundary(const Array<double,1>& timeline,
		const Array<double,3>& state_variable_paths,
		const Array<double,2>& numeraire_values,
		std::function<double (double,const Array<double,1>&)> payoff,
		const std::vector<std::function<double (double,const Array<double,1>&)> >& xbasis_functions,
		bool positive_payoff_only)
		: T(timeline),f(payoff),positive_only(positive_payoff_only),basis_functions(xbasis_functions),
		number_of_variables(state_variable_paths.extent(thirdDim))
	{
		int i,j;
		int N = state_variable_paths.extent(secondDim);
		if (N!=T.extent(firstDim)) throw std::logic_error("Dimension mismatch in LongstaffSchwartzExerciseBoundary");
		int npaths = state_variable_paths.extent(firstDim);
		int nval = npaths;
		Array<double,1> continuation_value(npaths),early_exercise(npaths);
		for (i=0;i<npaths;i++) 
		{
			Array<double,1> state_variables = state_variable_paths(i,N-1,Range::all());
			continuation_value(i) = numeraire_values(i,N-2)/numeraire_values(i,N-1) * payoff(T(N-1),state_variables);
		}
		for (j=N-2;j>0;j--) 
		{
			for (i=0;i<npaths;i++) 
			{
				Array<double,1> state_variables = state_variable_paths(i,j,Range::all());
				early_exercise(i) = payoff(T(j),state_variables); 
			}
			// eliminate non-positive early exercise values if necessary
			if (positive_payoff_only) 
			{
				nval = 0;
				for (i=0;i<npaths;i++) if (early_exercise(i)>0.0) nval++;
			}
			Array<double,2> x(nval,state_variable_paths.extent(blitz::thirdDim));
			Array<double,1> y(nval);
			if (positive_payoff_only) 
			{
				int c = 0;
				for (i=0;i<npaths;i++) 
				{
					if (early_exercise(i)>0.0) 
					{
						x(c,Range::all()) = state_variable_paths(i,j,Range::all());
						y(c) = continuation_value(i);
						c++; 
					}
				}
			}
			else
			{
				x = state_variable_paths(Range::all(),j,Range::all());
				y = continuation_value; 
			}
			// prepare basis functions for time T(j)
			std::vector<std::function<double (const Array<double,1>&)> > basisfunctions;
			for (i=0;i<basis_functions.size();i++) 
			{
				std::function<double (const Array<double,1>&)> g = std::bind(basis_functions[i],T(j),std::placeholders::_1);
				basisfunctions.push_back(g); 
			}
			// do regression   
			std::shared_ptr<LeastSquaresFit> newfit = std::make_shared<LeastSquaresFit>(y,x,basisfunctions);
			if (!newfit) throw std::logic_error("Unable to do regression in LongstaffSchwartzExerciseBoundary");                       
			fit.insert(fit.begin(),newfit);
			// update continuation values
			for (i=0;i<npaths;i++) 
			{
				continuation_value(i) = numeraire_values(i,j-1)/numeraire_values(i,j) * std::max(early_exercise(i),newfit->predict(state_variable_paths(i,j,Range::all()))); 
			}
		}
	}

	LongstaffSchwartzExerciseBoundary::~LongstaffSchwartzExerciseBoundary()
	{
		int i;
		for (i=0;i<(int)fit.size();i++) if (fit[i]) fit.erase(fit.begin() + i);                                                                           
	}

	double LongstaffSchwartzExerciseBoundary::apply(const Array<double,2>& path,const Array<double,1>& numeraire_values) const
	{
		int i;
		double result = 0.0;
		int N = path.extent(firstDim);
		if (N!=T.extent(firstDim)) throw std::logic_error("Dimension mismatch in LongstaffSchwartzExerciseBoundary");
		for (i=1;i<N-1;i++) 
		{
			double payoff = f(T(i),path(i,Range::all()));
			if ((payoff>0.0)||(!positive_only)) {
				if (payoff>fit[i-1]->predict(path(i,Range::all()))) 
				{
					result = payoff / numeraire_values(i);
					return result * numeraire_values(0); 
				}
			}
		}
		result = f(T(N-1),path(N-1,Range::all())) / numeraire_values(N-1);
		return result * numeraire_values(0);               
	}

	double polynomial(double t,const Array<double,1>& x,const Array<int,1>& p)
	{
		int i,j;
		double result = 1.0;
		for (i=0;i<x.extent(firstDim);i++)
		{
			for (j=0;j<p(i);j++) result *= x(i); 
		}
		return result;
	}

	void add_polynomial_basis_function(std::vector<std::function<double (double,const Array<double,1>&)> >& basisfunctions,
		const Array<int,1>& p)
	{
		std::function<double (double,const Array<double,1>&)> f = std::bind(polynomial,std::placeholders::_1,std::placeholders::_2,p.copy());
		basisfunctions.push_back(f); 
	}

	RegressionExerciseBoundary::RegressionExerciseBoundary(const Array<double,1>& timeline,
		const Array<double,3>& state_variable_paths,
		const Array<double,2>& numeraire_values,
		std::function<double (const Array<double,1>&,const Array<double,2>&)> payoff,
		const std::vector<std::function<double (const Array<double,1>&,const Array<double,2>&)> >& xbasis_functions,
		bool positive_payoff_only,
		bool ini)
		: T(timeline),f(payoff),positive_only(positive_payoff_only),basis_functions(xbasis_functions),
		number_of_variables(state_variable_paths.extent(thirdDim))
	{
		if (ini) initialise(state_variable_paths,numeraire_values,payoff,positive_payoff_only);
	}

	void RegressionExerciseBoundary::initialise(const Array<double,3>& state_variable_paths,const Array<double,2>& numeraire_values,std::function<double (const Array<double,1>&,const Array<double,2>&)> payoff,bool positive_payoff_only)
	{
		int i,j,k,ii;
		int N = state_variable_paths.extent(secondDim);
		if (N!=T.extent(firstDim)) throw std::logic_error("Dimension mismatch in RegressionExerciseBoundary");
		int npaths = state_variable_paths.extent(firstDim);
		int nval = npaths;
		Array<double,1> continuation_value(npaths),early_exercise(npaths);
		for (i=0;i<npaths;i++) 
		{
			// state_variables is a (time points) x (state variables) Array
			Array<double,2> state_variables = state_variable_paths(i,Range::all(),Range::all());
			continuation_value(i) = numeraire_values(i,N-2)/numeraire_values(i,N-1) * payoff(T,state_variables); 
		}
		for (j=N-2;j>0;j--)
		{
			for (i=0;i<npaths;i++) 
			{
				Array<double,2> state_variables = state_variable_paths(i,Range(fromStart,j),Range::all());
				early_exercise(i) = payoff(T(Range(fromStart,j)),state_variables); 
			}
			// eliminate non-positive early exercise values if necessary
			if (positive_payoff_only) 
			{
				nval = 0;
				for (i=0;i<npaths;i++) if (early_exercise(i)>0.0) nval++; 
			}
			/* must modify x to include history - middle index of state_variable_paths is Range(fromStart,j), but this
			three-dimensional Array then needs to be flattened to a two-dimensional Array, with the second dimension 
			now containing the history of state variable 1, followed by the history of state variable 2, etc. */
			Array<double,3> x(nval,j+1,state_variable_paths.extent(blitz::thirdDim));
			Array<double,1> y(nval);
			if (positive_payoff_only) 
			{
				int c = 0;
				for (i=0;i<npaths;i++) 
				{
					if (early_exercise(i)>0.0) 
					{
						x(c,Range::all(),Range::all()) = state_variable_paths(i,Range(fromStart,j),Range::all());
						y(c) = continuation_value(i);
						c++; 
					}
				}
			}
			else
			{
				x = state_variable_paths(Range::all(),Range(fromStart,j),Range::all());
				y = continuation_value; 
			}
			/* flatten x to a two-dimensional Array, with the second dimension 
			now containing the history of state variable 1, followed by the history of state variable 2, etc. */
			Array<double,2> flattened_x(nval,x.extent(secondDim)*x.extent(thirdDim));
			for (i=0;i<x.extent(secondDim);i++) 
			{
				for (k=0;k<x.extent(thirdDim);k++) 
				{
					flattened_x(Range::all(),k*x.extent(secondDim)+i) = x(Range::all(),i,k); 
				}
			}
			/* Prepare basis functions for time T(j). Note that basis_functions is a vector of
			std::function<double (const Array<double,1>&,const Array<double,2>&)>
			functors, the first argument of which must be bound to T(Range(fromStart,j)) and the second argument
			(un-)flattened as above, to a vector of 
			std::function<double (const Array<double,1>&)>
			functors to the constructor of LeastSquaresFit. */
			std::vector<std::function<double (const Array<double,1>&)> > basisfunctions;
			for (i=0;i<basis_functions.size();i++) 
			{
				BasisFunction* bf = new BasisFunction(basis_functions[i],T(Range(fromStart,j)));
				basis.push_back(bf);
				std::function<double (const Array<double,1>&)> g = std::bind(&BasisFunction::operator(),bf,std::placeholders::_1);
				basisfunctions.push_back(g); 
			}
			// do regression   
			std::shared_ptr<LeastSquaresFit> newfit = std::make_shared<LeastSquaresFit>(y,flattened_x,basisfunctions);
			if (!newfit) throw std::logic_error("Unable to do regression in RegressionExerciseBoundary");                       
			fit.insert(fit.begin(),newfit);
			// update continuation values - can use flattened_x if nval == npaths
			if (nval==npaths) 
			{
				for (i=0;i<npaths;i++) 
				{
					continuation_value(i) = numeraire_values(i,j-1)/numeraire_values(i,j) * std::max(early_exercise(i),newfit->predict(flattened_x(i,Range::all()))); 
				}
			}
			else 
			{ // otherwise need to create flat_x for each path
				Array<double,1> flat_x((j+1)*state_variable_paths.extent(blitz::thirdDim));
				for (i=0;i<npaths;i++) 
				{
					for (ii=0;ii<=j;ii++) 
					{
						for (k=0;k<state_variable_paths.extent(blitz::thirdDim);k++) 
						{
							flat_x(k*(j+1)+ii) = state_variable_paths(i,ii,k);
						}
					}
					continuation_value(i) = numeraire_values(i,j-1)/numeraire_values(i,j) * std::max(early_exercise(i),newfit->predict(flat_x)); 
				}
			}
		}
	}

	RegressionExerciseBoundary::~RegressionExerciseBoundary()
	{
		int i;
		for (i=0;i<(int)fit.size();i++) if (fit[i]) fit.erase(fit.begin() + i);                                                                           
		for (i=0;i<(int)basis.size();i++) if (basis[i]) basis.erase(basis.begin() + i);                                                                           
	}

	double RegressionExerciseBoundary::apply(const Array<double,2>& path,const Array<double,1>& numeraire_values) 
	{
		int i,j;
		double result = 0.0;
		int N = path.extent(firstDim);
		int n = path.extent(secondDim); // number of state variables
		if (N!=T.extent(firstDim)) throw std::logic_error("Dimension mismatch in RegressionExerciseBoundary");
		for (i=1;i<N-1;i++) 
		{
			double payoff = f(T(Range(fromStart,i)),path(Range(fromStart,i),Range::all()));
			if ((payoff>0.0)||(!positive_only)) 
			{
				// create flattened path to T(i)
				Array<double,1> ipath((i+1)*n);
				for (j=0;j<n;j++) ipath(Range(j*(i+1),(j+1)*(i+1)-1)) = path(Range(fromStart,i),j);
				if (payoff>fit[i-1]->predict(ipath)) 
				{
					result = payoff / numeraire_values(i);
					return result * numeraire_values(0); 
				}
			}
		}
		result = f(T,path) / numeraire_values(N-1);
		return result * numeraire_values(0);               
	}

	double RegressionExerciseBoundary::BasisFunction::operator()(const Array<double,1>& x) const
	{
		int i;
		// create a (state variables) X (observation time points) Array
		int n = T.extent(firstDim);
		int m = x.extent(firstDim)/n;
		Array<double,2> unflattened_x(m,n);
		for (i=0;i<m;i++) unflattened_x(i,Range::all()) = x(Range(i*n,(i+1)*n-1));
		// call function
		return f(T,unflattened_x);
	}

	double REBpolynomial(const Array<double,1>& T,const Array<double,2>& x,const Array<int,1>& p)
	{
		int i,j;
		int n = x.extent(secondDim)-1;
		double result = 1.0;
		for (i=0;i<x.extent(firstDim);i++) 
		{
			for (j=0;j<p(i);j++) result *= x(i,n); 
		}
		return result;
	}

	void add_polynomial_basis_function(std::vector<std::function<double (const Array<double,1>&,const Array<double,2>&)> >& basisfunctions,
		const Array<int,1>& p)
	{
		std::function<double (const Array<double,1>&,const Array<double,2>&)> f = std::bind(REBpolynomial,std::placeholders::_1,std::placeholders::_2,p.copy());
		basisfunctions.push_back(f); 
	}

	double LSArrayAdapter(double t,const Array<double,1>& x,std::function<double (double)> f,int idx)
	{
		return f(x(idx));
	}

	double LSArrayAdapterT(double t,const Array<double,1>& x,std::function<double (double,double)> f,int idx)
	{
		return f(t,x(idx));
	}

	double REBAdapter(const Array<double,1>& T,const Array<double,2>& x,std::function<double (double)> f,int idx) 
	{
		return f(x(x.extent(firstDim)-1,idx));
	}

	double REBAdapterT(const Array<double,1>& T,const Array<double,2>& x,std::function<double (double,double)> f,int idx) 
	{
		return f(T(T.extent(firstDim)-1),x(idx,x.extent(secondDim)-1));
	}
}
