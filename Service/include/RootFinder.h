/** \file  rootfinder.h
    \brief Header file for C++ port of FORTRAN 90 code implementing the General Complex Polynomial Root Solver of Skowron & Gould 2012.

     Paper:  Skowron & Gould 2012
          "General Complex Polynomial Root Solver and Its Further Optimization for Binary Microlenses"

     for a full text see:
     http://www.astrouw.edu.pl/~jskowron/cmplx_roots_sg/
     or http://arxiv.org/find/astro-ph
     or http://www.adsabs.harvard.edu/abstract_service.html.  

           C++ port copyright 2015 by Erik Schlögl		  
           */

/*
!  FORTRAN 90 code copyright 2012 Jan Skowron & Andrew Gould
!
!   Licensed under the Apache License, Version 2.0 (the "License");
!   you may not use this file except in compliance with the License.
!   You may obtain a copy of the License at
!
!       http://www.apache.org/licenses/LICENSE-2.0
!
!   Unless required by applicable law or agreed to in writing, software
!   distributed under the License is distributed on an "AS IS" BASIS,
!   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
!   See the License for the specific language governing permissions and
!   limitations under the License.
!
!-------------------------------------------------------------------!
!
! The authors also release this file under the GNU Lesser General
! Public License version 2 or any later version, as well as under
! a "customary scientific license", which implies
! that if this code was important in the scientific process or
! for the results of your scientific work, we ask for the
! appropriate citation of the Paper (Skowron & Gould 2012).
!
!-------------------------------------------------------------------!
!
!    No    Subroutine
!
!     1   cmplx_roots_gen               - general polynomial solver, works for random degree, not as fast or robust as cmplx_roots_5
!     2   cmplx_roots_5                 - complex roots finding algorithm taylored for 5th order polynomial (with failsafes for polishing)
!     3   sort_5_points_by_separation   - sorting of an array of 5 points, 1st most isolated, 4th and 5th - closest
!     4   sort_5_points_by_separation_i - sorting same as above, returns array of indicies rather than sorted array
!     5   find_2_closest_from_5         - finds closest pair of 5 points
!     6   cmplx_laguerre                - Laguerre's method with simplified Adams' stopping criterion
!     7   cmplx_newton_spec             - Newton's method with stopping criterion calculated every 10 steps
!     8   cmplx_laguerre2newton         - three regime method: Laguerre's, Second-order General method and Newton's
!     9   solve_quadratic_eq            - quadratic equation solver
!    10   solve_cubic_eq                - cubic equation solver based on Lagrange's method
!    11   divide_poly_1                 - division of the polynomial by (x-p)
!
! fortran 90 code
!
! Paper:  Skowron & Gould 2012
!         "General Complex Polynomial Root Solver and Its Further Optimization for Binary Microlenses"
!
! for a full text see:
!     http://www.astrouw.edu.pl/~jskowron/cmplx_roots_sg/
!     or http://arxiv.org/find/astro-ph
!     or http://www.adsabs.harvard.edu/abstract_service.html
*/

#ifndef _DERIVATIVE_ROOTFINDER_H
#define _DERIVATIVE_ROOTFINDER_H

#include <complex>
#include <blitz/array.h>

using blitz::Array;
using blitz::Range;
using std::complex;

namespace derivative
{
	void cmplx_roots_gen(Array<complex<double>, 1>& roots, const Array<complex<double>, 1>& poly, const int degree, bool& polish_roots_after, bool& use_roots_as_starting_points);
	void cmplx_roots_5(Array<complex<double>, 1>& roots, bool& first_3_roots_order_changed, Array<complex<double>, 1>& poly, bool& polish_only);
	void sort_5_points_by_separation(Array<complex<double>, 1>& points);
	void sort_5_points_by_separation_i(Array<int, 1>& sorted_points, Array<complex<double>, 1>& points);
	void find_2_closest_from_5(int& i1, int& i2, double& d2min, Array<complex<double>, 1>& points);
	void cmplx_laguerre(const Array<complex<double>, 1>& poly, const int degree, complex<double>& root, int& iter, bool& success);
	void cmplx_newton_spec(const Array<complex<double>, 1>& poly, const int degree, complex<double>& root, int& iter, bool& success);
	void cmplx_laguerre2newton(const Array<complex<double>, 1>& poly, const int degree, complex<double>& root, int& iter, bool& success, int starting_mode);
	void solve_quadratic_eq(complex<double>& x0, complex<double>& x1, const Array<complex<double>, 1>& poly);
	void solve_cubic_eq(complex<double>& x0, complex<double>& x1, complex<double>& x2, const Array<complex<double>, 1>& poly);
	void divide_poly_1(Array<complex<double>, 1>& polyout, complex<double>& remainder, complex<double>& p, Array<complex<double>, 1>& polyin, const int degree);
}
/* namespace derivative */

#endif /* _DERIVATIVE_ROOTFINDER_H_ */
