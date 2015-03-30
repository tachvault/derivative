/* 
Copyright (C) Nathan Muruganantha 2013 - 2014
Initial version: Copyright 2003, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013 by Erik Schlögl
*/

#ifndef _DERIVATIVE_INTERFACECLAPACK_H_
#define _DERIVATIVE_INTERFACECLAPACK_H_

#include "MSWarnings.hpp"
#include <blitz/array.h>

#if defined SERVICEUTIL_EXPORTS
#define SERVICE_UTIL_DLL_API __declspec(dllexport)
#else
#define SERVICE_UTIL_DLL_API __declspec(dllimport)
#endif

namespace derivative
{ 
	namespace interfaceCLAPACK 
	{
		using blitz::Array;

		/// Calculate Cholesky factorization of a real symmetric positive definite matrix.
		void SERVICE_UTIL_DLL_API Cholesky(const Array<double,2>& A,Array<double,2>& triangular,char LorU);

		void SERVICE_UTIL_DLL_API SingularValueDecomposition(const Array<double,2>& A,Array<double,2>& U,Array<double,1>& sigma,Array<double,2>& V);

		/// Solve symmetric eigenvalue problem.
		void SERVICE_UTIL_DLL_API SymmetricEigenvalueProblem(const Array<double,2>& A,Array<double,1>& eigval,Array<double,2>& eigvec,double eps = 1e-12);

		/// Solve system of linear equations A X = B using CLAPACK routines.
		void SERVICE_UTIL_DLL_API SolveLinear(const Array<double,2>& A,Array<double,2>& X,const Array<double,2>& B);

		/// Solve system of linear equations A X = B using CLAPACK routines, where A is a tridiagonal matrix.
		void SERVICE_UTIL_DLL_API SolveTridiagonal(const Array<double,2>& A,Array<double,2>& X,const Array<double,2>& B);
		void SERVICE_UTIL_DLL_API SolveTridiagonalSparse(const Array<double,2>& A,Array<double,2>& X,const Array<double,2>& B);

		/// Determinant of a real symmetric positive definite matrix.
		double SERVICE_UTIL_DLL_API PositiveSymmetricMatrixDeterminant(const Array<double,2>& A);

		/// Inverse of a real symmetric positive definite matrix.
		void SERVICE_UTIL_DLL_API PositiveSymmetricMatrixInverse(const Array<double,2>& A,Array<double,2>& inverseA);

		void SERVICE_UTIL_DLL_API MoorePenroseInverse(const Array<double,2>& A,Array<double,2>& inverseA,double eps = 1e-6);
	}
}

/* namespace derivative */

#endif /* _DERIVATIVE_INTERFACECLAPACK_H_ */
