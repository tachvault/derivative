/** \file  rootfinder.cpp
	\brief C++ source file for C++ port of FORTRAN 90 code implementing the General Complex Polynomial Root Solver of Skowron & Gould 2012.

	Paper:  Skowron & Gould 2012
	"General Complex Polynomial Root Solver and Its Further Optimization for Binary Microlenses"

	for a full text see:
	http://www.astrouw.edu.pl/~jskowron/cmplx_roots_sg/
	or http://arxiv.org/find/astro-ph
	or http://www.adsabs.harvard.edu/abstract_service.html.

	C++ port copyright 2015 by Erik Schl�gl
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

#include <complex>
#include <blitz/array.h>

namespace derivative
{
	using blitz::Array;
	using blitz::Range;
	using std::complex;

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

	/*
	!-------------------------------------------------------------------!
	! _1_                  CMPLX_ROOTS_GEN                              !
	!-------------------------------------------------------------------!
	*/
	// subroutine cmplx_roots_gen(roots, poly, degree, polish_roots_after, use_roots_as_starting_points)
	void cmplx_roots_gen(Array<complex<double>, 1>& roots, const Array<complex<double>, 1>& poly, const int degree, bool& polish_roots_after, bool& use_roots_as_starting_points) {
		/*
		! This subroutine finds roots of a complex polynomial.
		! It is general, however less fast or robust than cmplx_roots_5
		! which contains failsafe checks in the polishing stage, but is
		! designed only for 5th order polynomials.
		! It uses a new dynamic root finding algorithm (see the Paper).
		!
		! It can use Laguerre's method (subroutine cmplx_laguerre)
		! or Laguerre->SG->Newton method (subroutine
		! cmplx_laguerre2newton - this is default choice) to find
		! roots. It divides polynomial one by one by found roots. At the
		! end it finds last root from Viete's formula for quadratic
		! equation. Finally, it polishes all found roots using a full
		! polynomial and Newton's or Laguerre's method (default is
		! Laguerre's - subroutine cmplx_laguerre).
		! You can change default choices by commenting out and uncommenting
		! certain lines in the code below.
		!
		! Note:
		! - we solve for the last root with Viete's formula rather
		!   than doing full Laguerre step (which is time consuming
		!   and unnecessary)
		! - we do not introduce any preference to real roots
		! - in Laguerre implementation we omit unneccesarry calculation of
		!   absolute values of denominator
		! - we do not sort roots. If you need to sort
		!   roots - we have provided sorting subroutine called:
		!   sort_5_points_by_separation, which sorts points from most
		!   isolated to most close. Algorithm in this routine can be
		!   easily used for number of points different than 5.
		!
		implicit none
		! roots  - array which will hold all roots that had been found.
		!          If the flag 'use_roots_as_starting_points' is set to
		!          .true., then instead of point (0,0) we use value from
		!          this array as starting point for cmplx_laguerre
		! poly -   is an array of polynomial cooefs, length = degree+1,
		!          poly(1) is a constant term:
		!               1              2             3
		!          poly(1) x^0 + poly(2) x^1 + poly(3) x^2 + ...
		! degree - degree of the polynomial and size of 'roots' array
		! polish_roots_after - after all roots have been found by dividing
		!          original polynomial by each root found,
		!          you can opt in to polish all roots using full
		!          polynomial
		! use_roots_as_starting_points - usually we start Laguerre's
		!          method from point (0,0), but you can decide to use the
		!          values of 'roots' array as starting point for each new
		!          root that is searched for. This is useful if you have
		!          very rough idea where some of the roots can be.
		!
		integer, parameter :: RK = 8
		integer, intent(in) :: degree
		complex(kind=RK), dimension(degree+1), intent(in) :: poly ! coeffs of the polynomial
		complex(kind=RK), dimension(degree), intent(inout) :: roots
		logical, intent(in) :: polish_roots_after, use_roots_as_starting_points
		*/

		// complex(kind=RK), dimension(degree+1) :: poly2
		Array<complex<double>, 1> poly2(degree + 1);
		// complex(kind=RK), parameter :: zero = cmplx(0d0,0d0,RK)
		complex<double> zero(0.0);
		// integer :: i, n, iter
		int i, n, iter;
		// logical :: success
		bool success;
		// complex(kind=RK) :: coef, prev
		complex<double> coef, prev;

		poly2 = poly;

		// initialize starting points
		// if(.not.use_roots_as_starting_points) roots=zero
		if (!use_roots_as_starting_points) roots = zero;

		// skip small degree polynomials from doing Laguerre's method
		/*
		if(degree<=1)then
		if(degree==1) roots(1)=-poly(2)/poly(1)
		return
		endif */
		if (degree <= 1) {
			if (degree == 1) roots(0) = -poly(1) / poly(0);
			return;
		}

		//do n=degree, 3, -1
		for (n = degree; n >= 3; n--) {
			/*
			! find root with Laguerre's method
			!call cmplx_laguerre(poly2, n, roots(n), iter, success)
			! or
			! find root with (Laguerre's method -> SG method -> Newton's method) */
			// call cmplx_laguerre2newton(poly2, n, roots(n), iter, success, 2)
			cmplx_laguerre2newton(poly2, n, roots(n - 1), iter, success, 2);
			/*
			if(.not.success) then
			roots(n)=zero
			call cmplx_laguerre(poly2, n, roots(n), iter, success)
			endif */
			if (!success) {
				roots(n - 1) = zero;
				cmplx_laguerre(poly2, n, roots(n - 1), iter, success);
			}

			// divide the polynomial by this root
			/*
			coef = poly2(n+1)
			do i=n,1,-1
			prev=poly2(i)
			poly2(i)=coef
			coef=prev+roots(n)*coef
			enddo */
			coef = poly2(n);
			for (i = n; i >= 1; i--) {
				prev = poly2(i - 1);
				poly2(i - 1) = coef;
				coef = prev + roots(n - 1)*coef;
			}
			// variable coef now holds a remainder - should be close to 0
			// enddo
		}

		// find all but last root with Laguerre's method
		/*
		!call cmplx_laguerre(poly2, 2, roots(2), iter, success)
		! or
		call cmplx_laguerre2newton(poly2, 2, roots(2), iter, success, 2) */
		cmplx_laguerre2newton(poly2, 2, roots(1), iter, success, 2);
		/*
		if(.not.success) then
		call solve_quadratic_eq(roots(2),roots(1),poly2)
		else
		! calculate last root from Viete's formula
		roots(1)=-(roots(2)+poly2(2)/poly2(3))
		endif */
		if (!success) solve_quadratic_eq(roots(1), roots(0), poly2);
		else {
			// calculate last root from Viete's formula
			roots(0) = -(roots(1) + poly2(1) / poly2(2));
		}

		/*
		if(polish_roots_after)then
		do n=1, degree ! polish roots one-by-one with a full polynomial
		call cmplx_laguerre(poly, degree, roots(n), iter, success)
		!call cmplx_newton_spec(poly, degree, roots(n), iter, success)
		enddo
		endif */
		if (polish_roots_after) {
			for (n = 1; n <= degree; n++) { // polish roots one-by-one with a full polynomial
				cmplx_laguerre(poly, degree, roots(n - 1), iter, success);
			}
		}

		return;
		// end
	}

	/*
	!-------------------------------------------------------------------!
	! _2_                     CMPLX_ROOTS_5                             !
	!-------------------------------------------------------------------!
	*/
	// subroutine cmplx_roots_5(roots, first_3_roots_order_changed, poly, polish_only)
	void cmplx_roots_5(Array<complex<double>, 1>& roots, bool& first_3_roots_order_changed, Array<complex<double>, 1>& poly, bool& polish_only) {
		/*
		implicit none
		! Subroutine finds or polishes roots of a complex polynomial
		! (degree=5)
		! This routine is especially tailored for solving binary lens
		! equation in form of 5th order polynomial.
		!
		! Use of this routine, in comparission to 'cmplx_roots_gen' can yield
		! consideribly faster code, because it makes polishing of the roots
		! (that come in as a guess from previous solutions) secure by
		! implementing additional checks on the result of polishing.
		! If those checks are not satisfied then routine reverts to the
		! robust algorithm. These checks are designed to work for 5th order
		! polynomial originated from binary lens equation.
		!
		! Usage:
		!
		! polish_only == false - I do not know the roots, routine should
		!                find them from scratch. At the end it
		!                sorts roots from the most distant to closest.
		!                Two last roots are the closest (in no particular
		!                order).
		! polish_only = true - I do know the roots pretty well, for example
		!                I have changed the coefficiens of the polynomial
		!                only a bit, so the two closest roots are
		!                most likely still the closest ones.
		!                If the output flag 'first_3_roots_order_changed'
		!                is returned as 'false', then first 3 returned roots
		!                are in the same order as initialy given to the
		!                routine. The last two roots are the closest ones,
		!                but in no specific order (!).
		!                If 'first_3_roots_order_changed' is 'true' then
		!                it means that all roots had been resorted.
		!                Two last roots are the closest ones. First is most
		!                isolated one.
		!
		!
		! If you do not know the position of the roots just use flag
		! polish_only=.false. In this case routine will find the roots by
		! itself.

		! Returns all five roots in the 'roots' array.
		!
		! poly  - is an array of polynomial cooefs, length = degree+1
		!       poly(1) x^0 + poly(2) x^1 + poly(3) x^2 + poly(4) x^3 + ...
		! roots - roots of the polynomial ('out' and optionally 'in')
		!
		!
		! Jan Skowron 2011
		integer, parameter :: degree=5
		integer, parameter :: RK=8  ! kind for real and complex variables
		complex(kind=RK), dimension(degree), intent(inout) :: roots
		logical, intent(out) :: first_3_roots_order_changed
		complex(kind=RK), dimension(degree+1), intent(in)  :: poly
		logical, intent(in) :: polish_only
		!------------------------------------------------------------------
		!
		*/
		const int degree = 5;
		/*
	  complex(kind=RK) :: remainder, roots_robust(degree)
	  real(kind=RK) :: d2min
	  integer :: iter, loops, go_to_robust, m, root4, root5, i, i2
	  complex(kind=RK), dimension(degree+1) :: poly2
	  !integer, dimension(degree) :: sorted_ind
	  complex(kind=RK), parameter :: zero=cmplx(0d0,0d0,RK)
	  logical :: succ
	  */
		complex<double> remainder;
		Array<complex<double>, 1> roots_robust(5);
		double d2min;
		int iter, loops, go_to_robust, m, root4, root5, i, i2;
		Array<complex<double>, 1> poly2(degree + 1);
		complex<double> zero(0.0);
		bool succ;

		/*
		go_to_robust=0
		if(.not.polish_only) then
		! initialize roots
		roots=zero
		go_to_robust=1
		endif
		first_3_roots_order_changed=.false. */
		go_to_robust = 0;
		if (!polish_only) {
			// initialize roots
			roots = zero;
			go_to_robust = 1;
		}
		first_3_roots_order_changed = false;

		// do loops=1,3
		for (loops = 1; loops >= 3; loops++) {

			// ROBUST
			// (we do not know the roots)
			// if(go_to_robust>0) then
			if (go_to_robust > 0) {

				/*
				if(go_to_robust>2)then  ! something is wrong
				roots=roots_robust    ! return not-polished roots, because polishing creates errors
				return
				endif */
				if (go_to_robust > 2) {  // something is wrong
					roots = roots_robust;    // return not-polished roots, because polishing creates errors
					return;
				}

				/*
				poly2=poly ! copy coeffs
				do m=degree,4,-1 ! find the roots one-by-one (until 3 are left to be found)
				call cmplx_laguerre2newton(poly2, m, roots(m), iter, succ, 2)
				if(.not.succ)then
				roots(m)=zero
				call cmplx_laguerre(poly2, m, roots(m), iter, succ)
				endif
				! divide polynomial by this root
				call divide_poly_1(poly2, remainder, roots(m), poly2, m)
				enddo */
				poly2 = poly; // copy coeffs
				for (m = degree; m >= 4; m--) { // find the roots one-by-one (until 3 are left to be found)
					cmplx_laguerre2newton(poly2, m, roots(m - 1), iter, succ, 2);
					if (!succ) {
						roots(m - 1) = zero;
						cmplx_laguerre(poly2, m, roots(m - 1), iter, succ);
					}
					// divide polynomial by this root
					divide_poly_1(poly2, remainder, roots(m - 1), poly2, m);
				}
				// find last 3 roots with cubic euqation solver (Lagrange's method)
				// call solve_cubic_eq(roots(1),roots(2),roots(3),poly2)
				solve_cubic_eq(roots(0), roots(1), roots(2), poly2);
				// all roots found

				// sort roots - first will be most isolated, last two will be the closest
				// call sort_5_points_by_separation(roots)
				sort_5_points_by_separation(roots);
				// copy roots in case something will go wrong during polishing
				// roots_robust=roots
				roots_robust = roots;

				// set flag, that roots have been resorted
				// first_3_roots_order_changed=.true.
				first_3_roots_order_changed = true;
			} // endif  ! go_to_robust>0

			/*
			! POLISH
			! (we know the roots approximately, and we guess that last two are closest)
			!--------------------- */
			// poly2=poly ! copy coeffs
			poly2 = poly; // copy coeffs

			/*
			do m=1,degree-2
			!do m=1,degree                      ! POWN - polish only with Newton (option)
			*/
			for (m = 1; m <= degree - 2; m++) {

				// polish roots with full polynomial
				// call cmplx_newton_spec(poly2, degree, roots(m), iter, succ)
				cmplx_newton_spec(poly2, degree, roots(m - 1), iter, succ);

				/*
				if(.not.succ)then
				! go back to robust
				go_to_robust=go_to_robust+1
				roots=zero
				exit
				endif */
				if (!succ) {
					// go back to robust
					go_to_robust = go_to_robust + 1;
					roots = zero;
					break;
				}
			} // enddo ! m=1,degree-2

			// if(succ) then
			if (succ) {

				// comment out division and quadratic if you (POWN) polish with Newton only
				/*
				do m=1,degree-2
				call divide_poly_1(poly2, remainder, roots(m), poly2, degree-m+1)
				enddo */
				for (m = 1; m <= degree - 2; m++) divide_poly_1(poly2, remainder, roots(m - 1), poly2, degree - m + 1);
				// last two roots are found with quadratic equation solver
				// (this is faster and more robust, although little less accurate)
				/*
				call solve_quadratic_eq(roots(degree-1), &
				roots(degree  ),poly2) */
				solve_quadratic_eq(roots(degree - 2), roots(degree - 1), poly2);
				// all roots found and polished

				// TEST ORDER
				// test closest roots if they are the same pair as given to polish
				// call find_2_closest_from_5(root4,root5, d2min, roots)
				find_2_closest_from_5(root4, root5, d2min, roots);

				/*
				! check if the closest roots are not too close, this could happen
				! when using polishing with Newton only, when two roots erroneously
				! colapsed to the same root. This check is not needed for polishing
				! 3 roots by Newton and using quadratic for the remaining two.
				! If the real roots are so close indeed (very low probability), this will just
				! take more time and the unpolished result be returned at the end
				! but algorithm will work, and will return accurate enough result
				!if(d2min<1d-18) then             ! POWN - polish only with Newton
				!  go_to_robust=go_to_robust+1    ! POWN - polish only with Newton
				!else                             ! POWN - polish only with Newton
				*/

				// if((root4<degree-1).or.(root5<degree-1)) then
				if ((root4 < degree - 1) || (root5 < degree - 1)) {
					// after polishing some of the 3 far roots become one of the 2 closest ones
					// go back to robust
					if (go_to_robust > 0) {
						// if came from robust
						// copy two most isolated roots as starting points for new robust
						for (i = 1; i <= degree - 3; i++) roots(degree - i) = roots_robust(i - 1);
					}
					else {
						// came from users initial guess
						// copy some 2 roots (except the closest ones)
						i2 = degree;
						/*
						do i=1,degree
						if((i/=root4).and.(i/=root5))then
						roots(i2)=roots(i)
						i2=i2-1
						endif
						if(i2<=3) exit ! do not copy those that will be done by cubic in robust
						enddo */
						for (i = 1; i <= degree; i++) {
							if ((i != root4) && (i != root5)) {
								roots(i2 - 1) = roots(i - 1);
								i2 = i2 - 1;
							}
							if (i2 <= 3) break;
						}
					} // do not copy those that will be done by cubic in robust
					go_to_robust = go_to_robust + 1;
				}
				else {
					// root4 and root5 comes from the initial closest pair
					// most common case
					return;
				}

				// !endif                            ! POWN - polish only with Newton
			}
			// !---------------------
		} // enddo ! loops

		return;
	} // end


	/*
	!-------------------------------------------------------------------!
	! _3_                SORT_5_POINTS_BY_SEPARATION                    !
	!-------------------------------------------------------------------! */
	// subroutine sort_5_points_by_separation(points)
	void sort_5_points_by_separation(Array<complex<double>, 1>& points) {
		/*
	  ! Sort array of five points
	  ! Most isolated point will become the first point in the array
	  ! The closest points will be the last two points in the array
	  !
	  ! Algorithm works well for all dimensions. We put n=5 as
	  ! a hardcoded value just for optimization purposes.
	  implicit none
	  integer, parameter :: RK=8
	  integer, parameter :: n=5 !  works for different n as well, but is faster for n as constant (optimization)
	  complex(kind=RK), dimension(n), intent(inout) :: points
	  */

		/*
	  integer, dimension(n) :: sorted_points
	  complex(kind=RK), dimension(n) :: savepoints
	  integer :: i */
		int n = 5;
		Array<int, 1> sorted_points(n);
		Array<complex<double>, 1> savepoints(n);
		int i;

		/*
		call sort_5_points_by_separation_i(sorted_points, points)
		savepoints=points
		do i=1,n
		points(i)=savepoints(sorted_points(i))
		enddo
		return */
		sort_5_points_by_separation_i(sorted_points, points);
		savepoints = points;
		for (i = 1; i <= n; i++) points(i - 1) = savepoints(sorted_points(i - 1) - 1);
		return;
	} // end

	/*
	!-------------------------------------------------------------------!
	! _4_              SORT_5_POINTS_BY_SEPARATION_I                    !
	!-------------------------------------------------------------------! */
	// subroutine sort_5_points_by_separation_i(sorted_points, points)
	void sort_5_points_by_separation_i(Array<int, 1>& sorted_points, Array<complex<double>, 1>& points) {
		/*
	  ! Return index array that sorts array of five points
	  ! Index of the most isolated point will appear on the firts place
	  ! of the output array.
	  ! The indices of the closest 2 points will be at the last two
	  ! places in the 'sorted_points' array
	  !
	  ! Algorithm works well for all dimensions. We put n=5 as
	  ! a hardcoded value just for optimization purposes. */
		/*
	  implicit none
	  integer, parameter :: RK=8
	  integer, parameter :: n=5 !  works for different n as well, but is faster for n as constant (optimization)
	  integer, dimension(n), intent(out) :: sorted_points
	  complex(kind=RK), dimension(n), intent(in) :: points */
		int n = 5; // !  works for different n as well, but is faster for n as constant (optimization)

		/*
		real(kind=RK) :: dmin, d1, d2, d
		real(kind=RK), dimension(n,n) :: distances2
		integer :: ki, kj, ind2, put
		real(kind=RK), dimension(n) :: neigh1st, neigh2nd
		complex(kind=RK) :: p
		*/
		double dmin, d1, d2, d;
		Array<double, 2> distances2(n, n);
		int ki, kj, ind2, put;
		Array<double, 1> neigh1st(n), neigh2nd(n);
		complex<double> p;

		/*
		distances2=1d100
		dmin=1d100 */
		distances2 = 1.0e100;
		dmin = 1.0e100;

		/*
		do kj=1, n
		do ki=1, kj-1
		p=points(ki)-points(kj)
		d=conjg(p)*p
		distances2(ki,kj)=d
		distances2(kj,ki)=d
		enddo
		enddo */
		for (kj = 1; kj <= n; kj++) {
			for (ki = 1; ki <= kj - 1; ki++) {
				p = points(ki - 1) - points(kj - 1);
				d = (std::conj(p)*p).real();
				distances2(ki - 1, kj - 1) = d;
				distances2(kj - 1, ki - 1) = d;
			}
		}

		// find neighbours
		/*
		neigh1st=1d100
		neigh2nd=1d100
		do kj=1, n
		do ki=1, n
		d=distances2(kj,ki)
		if(d<neigh2nd(kj))then
		if(d<neigh1st(kj))then
		neigh2nd(kj)=neigh1st(kj)
		neigh1st(kj)=d
		else
		neigh2nd(kj)=d
		endif
		endif
		enddo
		enddo */
		neigh1st = 1.0e100;
		neigh2nd = 1.0e100;
		for (kj = 1; kj <= n; kj++) {
			for (ki = 1; ki <= n; ki++) {
				d = distances2(kj - 1, ki - 1);
				if (d < neigh2nd(kj - 1)) {
					if (d < neigh1st(kj - 1)) {
						neigh2nd(kj - 1) = neigh1st(kj - 1);
						neigh1st(kj - 1) = d;
					}
					else neigh2nd(kj - 1) = d;
				}
			}
		}

		// initialize sorted_points
		/*
		do ki=1,n
		sorted_points(ki)=ki
		enddo */
		for (ki = 1; ki <= n; ki++) sorted_points(ki - 1) = ki;

		// sort the rest 1..n-2
		/*
		do kj=2,n
		d1=neigh1st(kj)
		d2=neigh2nd(kj)
		put=1
		do ki=kj-1,1,-1
		ind2=sorted_points(ki)
		d=neigh1st(ind2)
		if(d>=d1) then
		if(d==d1)then
		if(neigh2nd(ind2)>d2)then
		put=ki+1
		exit
		endif
		else
		put=ki+1
		exit
		endif
		endif
		sorted_points(ki+1)=sorted_points(ki)
		enddo
		sorted_points(put)=kj
		enddo */
		for (kj = 2; kj <= n; kj++) {
			d1 = neigh1st(kj - 1);
			d2 = neigh2nd(kj - 1);
			put = 1;
			for (ki = kj - 1; ki >= 1; ki--) {
				ind2 = sorted_points(ki - 1);
				d = neigh1st(ind2 - 1);
				if (d >= d1) {
					if (d == d1) {
						if (neigh2nd(ind2 - 1) > d2) {
							put = ki + 1;
							break;
						}
					}
					else {
						put = ki + 1;
						break;
					}
				}
				sorted_points(ki) = sorted_points(ki - 1);
			}
			sorted_points(put - 1) = kj;
		}

		return;
	} // end


	/*
	!-------------------------------------------------------------------!
	! _5_                     FIND_2_CLOSEST_FROM_5                     !
	!-------------------------------------------------------------------! */
	// subroutine find_2_closest_from_5(i1,i2, d2min, points)
	void find_2_closest_from_5(int& i1, int& i2, double& d2min, Array<complex<double>, 1>& points) {
		// Returns indices of the two closest points out of array of 5
		/*
	  implicit none
	  integer, parameter :: RK=8
	  integer, parameter :: n=5 ! will work for other n too, but it is faster with n as constant
	  integer, intent(out) :: i1, i2
	  !real(kind=RK), dimension(n,n) :: distances2
	  complex(kind=RK), dimension(n), intent(in) :: points
	  real(kind=RK), intent(out) :: d2min  ! square of minimal distance */
		int n = 5; // will work for other n too, but it is faster with n as constant

		/*
		real(kind=RK) :: d2min1, d2
		integer :: i,j
		complex(kind=RK) :: p */
		double d2min1, d2;
		int i, j;
		complex<double> p;

		/*
		d2min1=1d100
		do j=1,n
		!distances2(j,j)=0d0
		do i=1,j-1
		p=points(i)-points(j)
		d2=conjg(p)*p
		!distances2(i,j)=d2
		!distances2(j,i)=d2
		if(d2<=d2min1)then
		i1=i
		i2=j
		d2min1=d2
		endif
		enddo
		enddo */
		d2min1 = 1.0e100;
		for (j = 1; j <= n; j++) {
			for (i = 1; i <= j - 1; i++) {
				p = points(i - 1) - points(j - 1);
				d2 = (std::conj(p)*p).real();
				if (d2 <= d2min1) {
					i1 = i;
					i2 = j;
					d2min1 = d2;
				}
			}
		}

		//d2min=d2min1
		d2min = d2min1;

	} // end

	/*
	!-------------------------------------------------------------------!
	! _6_                     CMPLX_LAGUERRE                            !
	!-------------------------------------------------------------------! */
	// recursive subroutine cmplx_laguerre(poly, degree, root, iter, success)
	void cmplx_laguerre(const Array<complex<double>, 1>& poly, const int degree, complex<double>& root, int& iter, bool& success) {
		/*
	  implicit none
	  ! Subroutine finds one root of a complex polynomial using
	  ! Laguerre's method. In every loop it calculates simplified
	  ! Adams' stopping criterion for the value of the polynomial.
	  !
	  ! Uses 'root' value as a starting point (!!!!!)
	  ! Remember to initialize 'root' to some initial guess or to
	  ! point (0,0) if you have no prior knowledge.
	  !
	  ! poly - is an array of polynomial cooefs
	  !        length = degree+1, poly(1) is constant
	  !               1              2             3
	  !          poly(1) x^0 + poly(2) x^1 + poly(3) x^2 + ...
	  ! degree - a degree of the polynomial
	  ! root - input: guess for the value of a root
	  !        output: a root of the polynomial
	  ! iter - number of iterations performed (the number of polynomial
	  !        evaluations and stopping criterion evaluation)
	  ! success - is false if routine reaches maximum number of iterations
	  !
	  ! For a summary of the method go to:
	  ! http://en.wikipedia.org/wiki/Laguerre's_method
	  */

		/*
	  integer, parameter :: RK=8  ! kind for real and complex variables
	  integer, parameter :: MAX_ITERS=200   ! Laguerre is used as a failsafe
	  ! constants needed to break cycles in the scheme
	  integer, parameter :: FRAC_JUMP_EVERY=10
	  integer, parameter :: FRAC_JUMP_LEN=10
	  real(kind=RK), dimension(FRAC_JUMP_LEN), parameter :: FRAC_JUMPS=(/0.64109297d0, &
	  0.91577881d0, 0.25921289d0,  0.50487203d0, &
	  0.08177045d0, 0.13653241d0,  0.306162d0  , &
	  0.37794326d0, 0.04618805d0,  0.75132137d0/) ! some random numbers
	  real(kind=RK), parameter :: pi = 3.141592653589793d0
	  real(kind=RK) :: faq ! jump length
	  real(kind=RK), parameter :: FRAC_ERR = 2.0d-15  ! fractional error for kind=8 (see. Adams 1967 Eqs 9 and 10)

	  integer, intent(in) :: degree
	  complex(kind=RK), dimension(degree+1), intent(in)  :: poly
	  integer, intent(out) :: iter
	  complex(kind=RK), intent(inout) :: root
	  logical, intent(out) :: success */
		const int MAX_ITERS = 200;   // Laguerre is used as a failsafe
		// constants needed to break cycles in the scheme
		const int FRAC_JUMP_EVERY = 10;
		const int FRAC_JUMP_LEN = 10;
		double FRAC_JUMPS[FRAC_JUMP_LEN] = { 0.64109297, 0.91577881, 0.25921289, 0.50487203, 0.08177045, 0.13653241, 0.306162, 0.37794326, 0.04618805, 0.75132137 }; // some random numbers
		const double pi = 3.141592653589793;
		double faq; // jump length
		const double FRAC_ERR = 2.0e-15;  // fractional error for kind=8 (see. Adams 1967 Eqs 9 and 10)

		/*
		complex(kind=RK) :: p         ! value of polynomial
		complex(kind=RK) :: dp        ! value of 1st derivative
		complex(kind=RK) :: d2p_half  ! value of 2nd derivative
		integer :: i, k
		logical :: good_to_go
		!complex(kind=RK) :: G, H, G2
		complex(kind=RK) :: denom, denom_sqrt, dx, newroot
		real(kind=RK) :: ek, absroot, abs2p
		complex(kind=RK) :: fac_netwon, fac_extra, F_half, c_one_nth
		real(kind=RK) :: one_nth, n_1_nth, two_n_div_n_1
		complex(kind=RK), parameter :: c_one=cmplx(1d0,0d0,RK)
		complex(kind=RK), parameter :: zero=cmplx(0d0,0d0,RK)
		real(kind=RK) :: stopping_crit2 */
		complex<double> p;         // value of polynomial
		complex<double> dp;        // value of 1st derivative
		complex<double> d2p_half;  // value of 2nd derivative
		int i, k;
		bool good_to_go;
		complex<double> denom, denom_sqrt, dx, newroot;
		double ek, absroot, abs2p;
		complex<double> fac_netwon, fac_extra, F_half, c_one_nth;
		double one_nth, n_1_nth, two_n_div_n_1;
		complex<double> c_one(1.0, 0.0);
		complex<double> zero(0.0, 0.0);
		double stopping_crit2;

		//---------------------------------------

		iter = 0;
		success = true;

		/*
		! next if-endif block is an EXTREME failsafe, not usually needed, and thus turned off in this version.
		if(.false.)then ! change false-->true if you would like to use caution about having first coefficient == 0
		if(degree<0) then
		write(*,*) 'Error: cmplx_laguerre: degree<0'
		return
		endif
		if(poly(degree+1)==zero) then
		if(degree==0) return
		call cmplx_laguerre(poly, degree-1, root, iter, success)
		return
		endif
		if(degree<=1)then
		if(degree==0) then  ! we know from previous check than poly(1) not equal zero
		success=.false.
		write(*,*) 'Warning: cmplx_laguerre: degree=0 and poly(1)/=0, no roots'
		return
		else
		root=-poly(1)/poly(2)
		return
		endif
		endif
		endif
		!  end EXTREME failsafe */


		/*good_to_go=.false.
		one_nth=1d0/degree
		n_1_nth=(degree-1d0)*one_nth
		two_n_div_n_1=2d0/n_1_nth
		c_one_nth=cmplx(one_nth,0d0,RK)
		*/
		good_to_go = false;
		one_nth = 1.0 / degree;
		n_1_nth = (degree - 1.0)*one_nth;
		two_n_div_n_1 = 2.0 / n_1_nth;
		c_one_nth = one_nth;

		// do i=1,MAX_ITERS
		for (i = 1; i <= MAX_ITERS; i++) {
			// prepare stopping criterion
			/*
		  ek=abs(poly(degree+1))
		  absroot=abs(root)
		  ! calculate value of polynomial and its first two derivatives
		  p  =poly(degree+1)
		  dp =zero
		  d2p_half=zero
		  do k=degree,1,-1 ! Horner Scheme, see for eg.  Numerical Recipes Sec. 5.3 how to evaluate polynomials and derivatives
		  d2p_half=dp + d2p_half*root
		  dp =p + dp*root
		  p  =poly(k)+p*root    ! b_k
		  ! Adams, Duane A., 1967, "A stopping criterion for polynomial root finding",
		  ! Communications of the ACM, Volume 10 Issue 10, Oct. 1967, p. 655
		  ! ftp://reports.stanford.edu/pub/cstr/reports/cs/tr/67/55/CS-TR-67-55.pdf
		  ! Eq 8.
		  ek=absroot*ek+abs(p)
		  enddo
		  iter=iter+1 */
			ek = std::abs(poly(degree));
			absroot = std::abs(root);
			// calculate value of polynomial and its first two derivatives
			p = poly(degree);
			dp = zero;
			d2p_half = zero;
			for (k = degree; k >= 1; k--) { // Horner Scheme, see for eg.  Numerical Recipes Sec. 5.3 how to evaluate polynomials and derivatives
				d2p_half = dp + d2p_half*root;
				dp = p + dp*root;
				p = poly(k - 1) + p*root;    // b_k
				/*
				! Adams, Duane A., 1967, "A stopping criterion for polynomial root finding",
				! Communications of the ACM, Volume 10 Issue 10, Oct. 1967, p. 655
				! ftp://reports.stanford.edu/pub/cstr/reports/cs/tr/67/55/CS-TR-67-55.pdf
				! Eq 8. */
				ek = absroot*ek + std::abs(p);
			}
			iter = iter + 1;

			/*
			abs2p=conjg(p)*p
			if(abs2p==0d0) return
			stopping_crit2=(FRAC_ERR*ek)**2
			if(abs2p<stopping_crit2) then ! (simplified a little Eq. 10 of Adams 1967)
			! do additional iteration if we are less than 10x from stopping criterion
			if(abs2p<0.01d0*stopping_crit2) then
			return ! return immediately, because we are at very good place
			else
			good_to_go=.true. ! do one iteration more
			endif
			else
			good_to_go=.false.  ! reset if we are outside the zone of the root
			endif */
			abs2p = (std::conj(p)*p).real();
			if (abs2p == 0.0) return;
			stopping_crit2 = (FRAC_ERR*ek)*(FRAC_ERR*ek);
			if (abs2p < stopping_crit2) { // (simplified a little Eq. 10 of Adams 1967)
				// do additional iteration if we are less than 10x from stopping criterion
				if (abs2p < 0.01*stopping_crit2) return; // return immediately, because we are at very good place
				else good_to_go = true;
			} // do one iteration more
			else good_to_go = false;  // reset if we are outside the zone of the root

			faq = 1.0;

			/*
			fac_netwon=p/dp
			fac_extra=d2p_half/dp
			F_half=fac_netwon*fac_extra */
			fac_netwon = p / dp;
			fac_extra = d2p_half / dp;
			F_half = fac_netwon*fac_extra;

			// denom_sqrt=sqrt(c_one-two_n_div_n_1*F_half)
			denom_sqrt = std::sqrt(c_one - two_n_div_n_1*F_half);

			/*
			!G=dp/p  ! gradient of ln(p)
			!G2=G*G
			!H=G2-2d0*d2p_half/p  ! second derivative of ln(p)
			!denom_sqrt=sqrt( (degree-1)*(degree*H-G2) ) */

			// NEXT LINE PROBABLY CAN BE COMMENTED OUT
			/*
			if(real(denom_sqrt)>=0d0)then
			! real part of a square root is positive for probably all compilers. You can
			! test this on your compiler and if so, you can omit this check
			denom=c_one_nth+n_1_nth*denom_sqrt
			else
			denom=c_one_nth-n_1_nth*denom_sqrt
			endif
			if(denom==zero)then !test if demoninators are > 0.0 not to divide by zero
			dx=(absroot+1d0)*exp(cmplx(0d0,FRAC_JUMPS(mod(i,FRAC_JUMP_LEN)+1)*2*pi,RK)) ! make some random jump
			else
			dx=fac_netwon/denom
			!dx=degree/denom
			endif */
			if (denom_sqrt.real() >= 0.0) {
				// real part of a square root is positive for probably all compilers. You can
				// test this on your compiler and if so, you can omit this check
				denom = c_one_nth + n_1_nth*denom_sqrt;
			}
			else denom = c_one_nth - n_1_nth*denom_sqrt;
			if (denom == zero) { // test if demoninators are > 0.0 not to divide by zero
				complex<double> tmp(0.0, FRAC_JUMPS[i%FRAC_JUMP_LEN] * 2.0*pi);
				dx = (absroot + 1.0)*std::exp(tmp);
			} // make some random jump
			else dx = fac_netwon / denom;

			/*
			newroot=root-dx
			if(newroot==root) return ! nothing changes -> return
			if(good_to_go)then       ! this was jump already after stopping criterion was met
			root=newroot
			return
			endif */
			newroot = root - dx;
			if (newroot == root) return; // nothing changes -> return
			if (good_to_go) {          // this was jump already after stopping criterion was met
				root = newroot;
				return;
			}

			/*
			if(mod(i,FRAC_JUMP_EVERY)==0) then ! decide whether to do a jump of modified length (to break cycles)
			faq=FRAC_JUMPS(mod(i/FRAC_JUMP_EVERY-1,FRAC_JUMP_LEN)+1)
			newroot=root-faq*dx ! do jump of some semi-random length (0<faq<1)
			endif
			root=newroot
			enddo
			success=.false.
			! too many iterations here */
			if (i%FRAC_JUMP_EVERY == 0) { // decide whether to do a jump of modified length (to break cycles)
				faq = FRAC_JUMPS[(i / FRAC_JUMP_EVERY - 1) % FRAC_JUMP_LEN];
				newroot = root - faq*dx;
			} // do jump of some semi-random length (0<faq<1)
			root = newroot;
		}
		success = false;
		// too many iterations here
	} // end

	/*
	!-------------------------------------------------------------------!
	! _7_                     CMPLX_NEWTON_SPEC                         !
	!-------------------------------------------------------------------! */
	// recursive subroutine cmplx_newton_spec(poly, degree, root, iter, success)
	void cmplx_newton_spec(const Array<complex<double>, 1>& poly, const int degree, complex<double>& root, int& iter, bool& success) {
		/*
	  implicit none
	  ! Subroutine finds one root of a complex polynomial using
	  ! Newton's method. It calculates simplified Adams' stopping
	  ! criterion for the value of the polynomial once per 10 iterations (!),
	  ! after initial iteration. This is done to speed up calculations
	  ! when polishing roots that are known preety well, and stopping
	  ! criterion does significantly change in their neighborhood.
	  !
	  ! Uses 'root' value as a starting point (!!!!!)
	  ! Remember to initialize 'root' to some initial guess.
	  ! Do not initilize 'root' to point (0,0) if the polynomial
	  ! coefficients are strictly real, because it will make going
	  ! to imaginary roots impossible.
	  !
	  ! poly - is an array of polynomial cooefs
	  !        length = degree+1, poly(1) is constant
	  !               1              2             3
	  !          poly(1) x^0 + poly(2) x^1 + poly(3) x^2 + ...
	  ! degree - a degree of the polynomial
	  ! root - input: guess for the value of a root
	  !        output: a root of the polynomial
	  ! iter - number of iterations performed (the number of polynomial
	  !        evaluations)
	  ! success - is false if routine reaches maximum number of iterations
	  !
	  ! For a summary of the method go to:
	  ! http://en.wikipedia.org/wiki/Newton's_method
	  */

		/*
	  integer, parameter :: RK=8  ! kind for real and complex variables
	  integer, parameter :: MAX_ITERS=50
	  ! constants needed to break cycles in the scheme
	  integer, parameter :: FRAC_JUMP_EVERY=10
	  integer, parameter :: FRAC_JUMP_LEN=10
	  real(kind=RK), dimension(FRAC_JUMP_LEN), parameter :: FRAC_JUMPS=(/0.64109297d0, &
	  0.91577881d0, 0.25921289d0,  0.50487203d0, &
	  0.08177045d0, 0.13653241d0,  0.306162d0  , &
	  0.37794326d0, 0.04618805d0,  0.75132137d0/) ! some random numbers
	  real(kind=8), parameter :: pi = 3.141592653589793d0
	  real(kind=RK) :: faq ! jump length
	  real(kind=RK), parameter :: FRAC_ERR = 2.0d-15  ! fractional error for kind=8 (see. Adams 1967 Eqs 9 and 10) */
		const int MAX_ITERS = 50;
		// constants needed to break cycles in the scheme
		const int FRAC_JUMP_EVERY = 10;
		const int FRAC_JUMP_LEN = 10;
		double FRAC_JUMPS[FRAC_JUMP_LEN] = { 0.64109297, 0.91577881, 0.25921289, 0.50487203, 0.08177045, 0.13653241, 0.306162, 0.37794326, 0.04618805, 0.75132137 }; // some random numbers
		const double pi = 3.141592653589793;
		double faq; // jump length
		const double FRAC_ERR = 2.0 - 15;  // fractional error for kind=8 (see. Adams 1967 Eqs 9 and 10)

		/*
		integer, intent(in) :: degree
		complex(kind=RK), dimension(degree+1), intent(in)  :: poly
		integer, intent(out) :: iter
		complex(kind=RK), intent(inout) :: root
		logical, intent(out) :: success */

		/*
		complex(kind=RK) :: p    ! value of polynomial
		complex(kind=RK) :: dp   ! value of 1st derivative
		integer :: i, k
		logical :: good_to_go
		complex(kind=RK) :: dx, newroot
		real(kind=RK) :: ek, absroot, abs2p
		complex(kind=RK), parameter :: zero=cmplx(0d0,0d0,RK)
		real(kind=RK) :: stopping_crit2 */
		complex<double> p;    // value of polynomial
		complex<double> dp;    // value of 1st derivative
		int i, k;
		bool good_to_go;
		complex<double> dx, newroot;
		double ek, absroot, abs2p;
		complex<double> zero(0.0, 0.0);
		double stopping_crit2;

		//---------------------------------------

		/*
		iter=0
		success=.true. */
		iter = 0;
		success = true;

		/*
		! next if-endif block is an EXTREME failsafe, not usually needed, and thus turned off in this version.
		if(.false.)then ! change false-->true if you would like to use caution about having first coefficient == 0
		if(degree<0) then
		write(*,*) 'Error: cmplx_newton_spec: degree<0'
		return
		endif
		if(poly(degree+1)==zero) then
		if(degree==0) return
		call cmplx_newton_spec(poly, degree-1, root, iter, success)
		return
		endif
		if(degree<=1)then
		if(degree==0) then  ! we know from previous check than poly(1) not equal zero
		success=.false.
		write(*,*) 'Warning: cmplx_newton_spec: degree=0 and poly(1)/=0, no roots'
		return
		else
		root=-poly(1)/poly(2)
		return
		endif
		endif
		endif
		!  end EXTREME failsafe */

		//good_to_go=.false.
		good_to_go = false;

		/*
		do i=1,MAX_ITERS
		faq=1d0 */
		for (i = 1; i <= MAX_ITERS; i++) {
			faq = 1.0;

			// prepare stopping criterion
			// calculate value of polynomial and its first two derivatives
			/*
			p  =poly(degree+1)
			dp =zero */
			p = poly(degree);
			dp = zero;

			/*
			if(mod(i,10)==1) then ! calculate stopping criterion every tenth iteration
			ek=abs(poly(degree+1))
			absroot=abs(root)
			do k=degree,1,-1 ! Horner Scheme, see for eg.  Numerical Recipes Sec. 5.3 how to evaluate polynomials and derivatives
			dp =p + dp*root
			p  =poly(k)+p*root    ! b_k
			! Adams, Duane A., 1967, "A stopping criterion for polynomial root finding",
			! Communications of the ACM, Volume 10 Issue 10, Oct. 1967, p. 655
			! ftp://reports.stanford.edu/pub/cstr/reports/cs/tr/67/55/CS-TR-67-55.pdf
			! Eq 8.
			ek=absroot*ek+abs(p)
			enddo
			stopping_crit2=(FRAC_ERR*ek)**2
			else               ! calculate just the value and derivative
			do k=degree,1,-1 ! Horner Scheme, see for eg.  Numerical Recipes Sec. 5.3 how to evaluate polynomials and derivatives
			dp =p + dp*root
			p  =poly(k)+p*root    ! b_k
			enddo
			endif
			iter=iter+1 */
			if (i % 10 == 1) { // calculate stopping criterion every tenth iteration
				ek = std::abs(poly(degree));
				absroot = abs(root);
				for (k = degree; k >= 1; k--) { // Horner Scheme, see for eg.  Numerical Recipes Sec. 5.3 how to evaluate polynomials and derivatives
					dp = p + dp*root;
					p = poly(k - 1) + p*root;    // b_k
					/*
					! Adams, Duane A., 1967, "A stopping criterion for polynomial root finding",
					! Communications of the ACM, Volume 10 Issue 10, Oct. 1967, p. 655
					! ftp://reports.stanford.edu/pub/cstr/reports/cs/tr/67/55/CS-TR-67-55.pdf
					! Eq 8. */
					ek = absroot*ek + std::abs(p);
				}
				stopping_crit2 = (FRAC_ERR*ek)*(FRAC_ERR*ek);
			}
			else  {             // calculate just the value and derivative
				for (k = degree; k >= 1; k--) { // Horner Scheme, see for eg.  Numerical Recipes Sec. 5.3 how to evaluate polynomials and derivatives
					dp = p + dp*root;
					p = poly(k - 1) + p*root;
				}
			}   // b_k
			iter = iter + 1;

			/*
			abs2p=conjg(p)*p
			if(abs2p==0d0) return */
			abs2p = (std::conj(p)*p).real();
			if (abs2p == 0.0) return;

			/*
			if(abs2p<stopping_crit2) then ! (simplified a little Eq. 10 of Adams 1967)
			if(dp==zero) return ! if we have problem with zero, but we are close to the root, just accept
			! do additional iteration if we are less than 10x from stopping criterion
			if(abs2p<0.01d0*stopping_crit2) then
			return ! return immediately, because we are at very good place
			else
			good_to_go=.true. ! do one iteration more
			endif
			else
			good_to_go=.false. ! reset if we are outside the zone of the root
			endif */
			if (abs2p < stopping_crit2) { // (simplified a little Eq. 10 of Adams 1967)
				if (dp == zero) return; // if we have problem with zero, but we are close to the root, just accept
				// do additional iteration if we are less than 10x from stopping criterion
				if (abs2p < 0.01*stopping_crit2) return; // return immediately, because we are at very good place
				else good_to_go = true;
			} // do one iteration more
			else good_to_go = false; // reset if we are outside the zone of the root

			/*
			if(dp==zero)then
			! problem with zero
			dx=(abs(root)+1d0)*exp(cmplx(0d0,FRAC_JUMPS(mod(i,FRAC_JUMP_LEN)+1)*2*pi,RK)) ! make some random jump
			else
			dx=p/dp  ! Newton method, see http://en.wikipedia.org/wiki/Newton's_method
			endif */
			if (dp == zero) {
				// problem with zero
				complex<double> tmp(0.0, FRAC_JUMPS[i%FRAC_JUMP_LEN] * 2.0*pi);
				dx = (std::abs(root) + 1.0)*exp(tmp);
			} // make some random jump
			else dx = p / dp;  // Newton method, see http://en.wikipedia.org/wiki/Newton's_method

			/*
			newroot=root-dx
			if(newroot==root) return ! nothing changes -> return
			if(good_to_go)then       ! this was jump already after stopping criterion was met
			root=newroot
			return
			endif */
			newroot = root - dx;
			if (newroot == root) return; // nothing changes -> return
			if (good_to_go) {          // this was jump already after stopping criterion was met
				root = newroot;
				return;
			}

			/*
			if(mod(i,FRAC_JUMP_EVERY)==0) then ! decide whether to do a jump of modified length (to break cycles)
			faq=FRAC_JUMPS(mod(i/FRAC_JUMP_EVERY-1,FRAC_JUMP_LEN)+1)
			newroot=root-faq*dx ! do jump of some semi-random length (0<faq<1)
			endif
			root=newroot
			enddo
			success=.false.
			return
			! too many iterations here */
			if ((i%FRAC_JUMP_EVERY) == 0) { // decide whether to do a jump of modified length (to break cycles)
				faq = FRAC_JUMPS[(i / FRAC_JUMP_EVERY - 1) % FRAC_JUMP_LEN];
				newroot = root - faq*dx;
			} // do jump of some semi-random length (0<faq<1)
			root = newroot;
		}
		success = false;
		return;
		// too many iterations here
	} // end

	/*
	!-------------------------------------------------------------------!
	! _8_                     CMPLX_LAGUERRE2NEWTON                     !
	!-------------------------------------------------------------------! */
	// recursive subroutine cmplx_laguerre2newton(poly, degree, root, iter, success, starting_mode)
	void cmplx_laguerre2newton(const Array<complex<double>, 1>& poly, const int degree, complex<double>& root, int& iter, bool& success, int starting_mode) {
		/*
	  implicit none
	  ! Subroutine finds one root of a complex polynomial using
	  ! Laguerre's method, Second-order General method and Newton's
	  ! method - depending on the value of function F, which is a
	  ! combination of second derivative, first derivative and
	  ! value of polynomial [F=-(p"*p)/(p'p')].
	  !
	  ! Subroutine has 3 modes of operation. It starts with mode=2
	  ! which is the Laguerre's method, and continues until F
	  ! becames F<0.50, at which point, it switches to mode=1,
	  ! i.e., SG method (see paper). While in the first two
	  ! modes, routine calculates stopping criterion once per every
	  ! iteration. Switch to the last mode, Newton's method, (mode=0)
	  ! happens when becomes F<0.05. In this mode, routine calculates
	  ! stopping criterion only once, at the beginning, under an
	  ! assumption that we are already very close to the root.
	  ! If there are more than 10 iterations in Newton's mode,
	  ! it means that in fact we were far from the root, and
	  ! routine goes back to Laguerre's method (mode=2).
	  !
	  ! Uses 'root' value as a starting point (!!!!!)
	  ! Remember to initialize 'root' to some initial guess or to
	  ! point (0,0) if you have no prior knowledge.
	  !
	  ! poly - is an array of polynomial cooefs
	  !        length = degree+1, poly(1) is constant
	  !               1              2             3
	  !          poly(1) x^0 + poly(2) x^1 + poly(3) x^2 + ...
	  ! degree - a degree of the polynomial
	  ! root - input: guess for the value of a root
	  !        output: a root of the polynomial
	  ! iter - number of iterations performed (the number of polynomial
	  !        evaluations and stopping criterion evaluation)
	  ! success - is false if routine reaches maximum number of iterations
	  ! starting_mode - this should be by default = 2. However if you
	  !                 choose to start with SG method put 1 instead.
	  !                 Zero will cause the routine to
	  !                 start with Newton for first 10 iterations, and
	  !                 then go back to mode 2.
	  !
	  !
	  ! For a summary of the method see the paper: Skowron & Gould (2012)
	  */

		/*
	  integer, parameter :: RK=8  ! kind for real and complex variables
	  integer, parameter :: MAX_ITERS=50
	  ! constants needed to break cycles in the scheme
	  integer, parameter :: FRAC_JUMP_EVERY=10
	  integer, parameter :: FRAC_JUMP_LEN=10
	  real(kind=RK), dimension(FRAC_JUMP_LEN), parameter :: FRAC_JUMPS=(/0.64109297d0, &
	  0.91577881d0, 0.25921289d0,  0.50487203d0, &
	  0.08177045d0, 0.13653241d0,  0.306162d0  , &
	  0.37794326d0, 0.04618805d0,  0.75132137d0/) ! some random numbers
	  real(kind=RK), parameter :: pi = 3.141592653589793d0
	  real(kind=RK) :: faq ! jump length
	  real(kind=RK), parameter :: FRAC_ERR = 2.0d-15  ! fractional error for kind=8 (see. Adams 1967 Eqs 9 and 10) */
		const int MAX_ITERS = 50;
		// constants needed to break cycles in the scheme
		const int FRAC_JUMP_EVERY = 10;
		const int FRAC_JUMP_LEN = 10;
		double FRAC_JUMPS[FRAC_JUMP_LEN] = { 0.64109297, 0.91577881, 0.25921289, 0.50487203, 0.08177045, 0.13653241, 0.306162, 0.37794326, 0.04618805, 0.75132137 }; // some random numbers
		const double pi = 3.141592653589793;
		double faq;
		const double FRAC_ERR = 2.0e-15; // fractional error for kind=8 (see. Adams 1967 Eqs 9 and 10)

		/*
		integer, intent(in) :: degree
		complex(kind=RK), dimension(degree+1), intent(in)  :: poly
		integer, intent(out) :: iter
		complex(kind=RK), intent(inout) :: root
		integer, intent(in) :: starting_mode
		logical, intent(out) :: success */

		/*
		complex(kind=RK) :: p         ! value of polynomial
		complex(kind=RK) :: dp        ! value of 1st derivative
		complex(kind=RK) :: d2p_half  ! value of 2nd derivative
		integer :: i, j, k
		logical :: good_to_go
		!complex(kind=RK) :: G, H, G2
		complex(kind=RK) :: denom, denom_sqrt, dx, newroot
		real(kind=RK) :: ek, absroot, abs2p, abs2_F_half
		complex(kind=RK) :: fac_netwon, fac_extra, F_half, c_one_nth
		real(kind=RK) :: one_nth, n_1_nth, two_n_div_n_1
		integer :: mode
		complex(kind=RK), parameter :: c_one=cmplx(1d0,0d0,RK)
		complex(kind=RK), parameter :: zero=cmplx(0d0,0d0,RK)
		real(kind=RK) :: stopping_crit2 */
		complex<double> p;         // value of polynomial
		complex<double> dp;        // value of 1st derivative
		complex<double> d2p_half;  // value of 2nd derivative
		int i, j, k;
		bool good_to_go;
		complex<double> denom, denom_sqrt, dx, newroot;
		double ek, absroot, abs2p, abs2_F_half;
		complex<double> fac_netwon, fac_extra, F_half, c_one_nth;
		double one_nth, n_1_nth, two_n_div_n_1;
		int mode;
		complex<double> c_one(1.0, 0.0);
		complex<double> zero(0.0, 0.0);
		double stopping_crit2;

		/*
		iter=0
		success=.true. */
		iter = 0;
		success = true;

		/*
		! next if-endif block is an EXTREME failsafe, not usually needed, and thus turned off in this version.
		if(.false.)then ! change false-->true if you would like to use caution about having first coefficient == 0
		if(degree<0) then
		write(*,*) 'Error: cmplx_laguerre2newton: degree<0'
		return
		endif
		if(poly(degree+1)==zero) then
		if(degree==0) return
		call cmplx_laguerre2newton(poly, degree-1, root, iter, success, starting_mode)
		return
		endif
		if(degree<=1)then
		if(degree==0) then  ! we know from previous check than poly(1) not equal zero
		success=.false.
		write(*,*) 'Warning: cmplx_laguerre2newton: degree=0 and poly(1)/=0, no roots'
		return
		else
		root=-poly(1)/poly(2)
		return
		endif
		endif
		endif
		!  end EXTREME failsafe */

		/*
		j=1
		good_to_go=.false. */
		j = 1;
		good_to_go = false;

		// mode=starting_mode  ! mode=2 full laguerre, mode=1 SG, mode=0 newton
		mode = starting_mode;  // mode=2 full laguerre, mode=1 SG, mode=0 newton

		for (;;) { // infinite loop, just to be able to come back from newton, if more than 10 iteration there

			//------------------------------------------------------------- mode 2
			if (mode >= 2) {  // LAGUERRE'S METHOD
				/*
			  one_nth=1d0/degree
			  n_1_nth=(degree-1d0)*one_nth
			  two_n_div_n_1=2d0/n_1_nth
			  c_one_nth=cmplx(one_nth,0d0,RK)

			  do i=1,MAX_ITERS  !
			  faq=1d0

			  ! prepare stoping criterion
			  ek=abs(poly(degree+1))
			  absroot=abs(root)
			  ! calculate value of polynomial and its first two derivatives
			  p  =poly(degree+1)
			  dp =zero
			  d2p_half=zero
			  do k=degree,1,-1 ! Horner Scheme, see for eg.  Numerical Recipes Sec. 5.3 how to evaluate polynomials and derivatives
			  d2p_half=dp + d2p_half*root
			  dp =p + dp*root
			  p  =poly(k)+p*root    ! b_k
			  ! Adams, Duane A., 1967, "A stopping criterion for polynomial root finding",
			  ! Communications of the ACM, Volume 10 Issue 10, Oct. 1967, p. 655
			  ! ftp://reports.stanford.edu/pub/cstr/reports/cs/tr/67/55/CS-TR-67-55.pdf
			  ! Eq 8.
			  ek=absroot*ek+abs(p)
			  enddo
			  abs2p=conjg(p)*p !abs(p)
			  iter=iter+1
			  if(abs2p==0d0) return */
				one_nth = 1.0 / degree;
				n_1_nth = (degree - 1.0)*one_nth;
				two_n_div_n_1 = 2.0 / n_1_nth;
				c_one_nth = one_nth;

				for (i = 1; i <= MAX_ITERS; i++) {
					faq = 1.0;

					// prepare stopping criterion
					ek = std::abs(poly(degree));
					absroot = std::abs(root);
					// calculate value of polynomial and its first two derivatives
					p = poly(degree);
					dp = zero;
					d2p_half = zero;
					for (k = degree; k >= 1; k--) { // Horner Scheme, see for eg.  Numerical Recipes Sec. 5.3 how to evaluate polynomials and derivatives
						d2p_half = dp + d2p_half*root;
						dp = p + dp*root;
						p = poly(k - 1) + p*root;   // b_k
						/*
						! Adams, Duane A., 1967, "A stopping criterion for polynomial root finding",
						! Communications of the ACM, Volume 10 Issue 10, Oct. 1967, p. 655
						! ftp://reports.stanford.edu/pub/cstr/reports/cs/tr/67/55/CS-TR-67-55.pdf
						! Eq 8. */
						ek = absroot*ek + std::abs(p);
					}
					abs2p = (std::conj(p)*p).real();
					iter = iter + 1;
					if (abs2p == 0.0) return;

					/*
					stopping_crit2=(FRAC_ERR*ek)**2
					if(abs2p<stopping_crit2) then ! (simplified a little Eq. 10 of Adams 1967)
					! do additional iteration if we are less than 10x from stopping criterion
					if(abs2p<0.01d0*stopping_crit2) then ! ten times better than stopping criterion
					return ! return immediately, because we are at very good place
					else
					good_to_go=.true. ! do one iteration more
					endif
					else
					good_to_go=.false. ! reset if we are outside the zone of the root
					endif */
					stopping_crit2 = (FRAC_ERR*ek)*(FRAC_ERR*ek);
					if (abs2p < stopping_crit2) { // (simplified a little Eq. 10 of Adams 1967)
						// do additional iteration if we are less than 10x from stopping criterion
						if (abs2p < 0.01*stopping_crit2) { // ten times better than stopping criterion
							return;
						} // return immediately, because we are at very good place
						else good_to_go = true;
					} // do one iteration more
					else good_to_go = false; // reset if we are outside the zone of the root

					/*
					fac_netwon=p/dp
					fac_extra=d2p_half/dp
					F_half=fac_netwon*fac_extra */
					fac_netwon = p / dp;
					fac_extra = d2p_half / dp;
					F_half = fac_netwon*fac_extra;

					/*
					abs2_F_half=conjg(F_half)*F_half
					if(abs2_F_half<=0.0625d0)then     ! F<0.50, F/2<0.25
					! go to SG method
					if(abs2_F_half<=0.000625d0)then ! F<0.05, F/2<0.025
					mode=0 ! go to Newton's
					else
					mode=1 ! go to SG
					endif
					endif */
					abs2_F_half = (std::conj(F_half)*F_half).real();
					if (abs2_F_half <= 0.0625) {     // F<0.50, F/2<0.25
						// go to SG method
						if (abs2_F_half <= 0.000625) { // F<0.05, F/2<0.025
							mode = 0;
						} // go to Newton's
						else mode = 1;
					} // go to SG   

					// denom_sqrt=sqrt(c_one-two_n_div_n_1*F_half)
					denom_sqrt = std::sqrt(c_one - two_n_div_n_1*F_half);

					// NEXT LINE PROBABLY CAN BE COMMENTED OUT
					/*
					if(real(denom_sqrt)>=0d0)then
					! real part of a square root is positive for probably all compilers. You can
					! test this on your compiler and if so, you can omit this check
					denom=c_one_nth+n_1_nth*denom_sqrt
					else
					denom=c_one_nth-n_1_nth*denom_sqrt
					endif
					if(denom==zero)then !test if demoninators are > 0.0 not to divide by zero
					dx=(abs(root)+1d0)*exp(cmplx(0d0,FRAC_JUMPS(mod(i,FRAC_JUMP_LEN)+1)*2*pi,RK)) ! make some random jump
					else
					dx=fac_netwon/denom
					endif */
					if (denom_sqrt.real() >= 0.0) {
						// real part of a square root is positive for probably all compilers. You can
						// test this on your compiler and if so, you can omit this check
						denom = c_one_nth + n_1_nth*denom_sqrt;
					}
					else denom = c_one_nth - n_1_nth*denom_sqrt;
					if (denom == zero) { // test if demoninators are > 0.0 not to divide by zero
						complex<double> tmp(0.0, FRAC_JUMPS[i%FRAC_JUMP_LEN] * 2.0*pi);
						dx = (std::abs(root) + 1.0)*exp(tmp);
					} // make some random jump
					else dx = fac_netwon / denom;

					/*
					newroot=root-dx
					if(newroot==root) return ! nothing changes -> return
					if(good_to_go)then       ! this was jump already after stopping criterion was met
					root=newroot
					return
					endif */
					newroot = root - dx;
					if (newroot == root) return; // nothing changes -> return
					if (good_to_go) {          // this was jump already after stopping criterion was met
						root = newroot;
						return;
					}

					/*
					if(mode/=2) then
					root=newroot
					j=i+1    ! remember iteration index
					exit     ! go to Newton's or SG
					endif */
					if (mode != 2) {
						root = newroot;
						j = i + 1;    // remember iteration index
						break;
					}    // go to Newton's or SG

					/*
					if(mod(i,FRAC_JUMP_EVERY)==0) then ! decide whether to do a jump of modified length (to break cycles)
					faq=FRAC_JUMPS(mod(i/FRAC_JUMP_EVERY-1,FRAC_JUMP_LEN)+1)
					newroot=root-faq*dx ! do jump of some semi-random length (0<faq<1)
					endif
					root=newroot
					enddo ! do mode 2 */
					if ((i%FRAC_JUMP_EVERY) == 0) { // decide whether to do a jump of modified length (to break cycles)
						faq = FRAC_JUMPS[(i / FRAC_JUMP_EVERY - 1) % FRAC_JUMP_LEN];
						newroot = root - faq*dx;
					} // do jump of some semi-random length (0<faq<1)
					root = newroot;
				}
				// enddo ! do mode 2

				/*
			  if(i>=MAX_ITERS) then
			  success=.false.
			  return
			  endif */
				if (i >= MAX_ITERS) {
					success = false;
					return;
				}

				// endif ! if mode 2
			} // endif ! if mode 2


			// !------------------------------------------------------------- mode 1
			// if(mode==1) then  ! SECOND-ORDER GENERAL METHOD (SG)
			if (mode == 1) { // SECOND-ORDER GENERAL METHOD (SG)

				/*
			  do i=j,MAX_ITERS  !
			  faq=1d0 */
				for (i = j; i <= MAX_ITERS; i++) {
					faq = 1.0;

					// calculate value of polynomial and its first two derivatives
					/*
					p  =poly(degree+1)
					dp =zero
					d2p_half=zero
					if(mod(i-j,10)==0)then
					! prepare stoping criterion
					ek=abs(poly(degree+1))
					absroot=abs(root)
					do k=degree,1,-1 ! Horner Scheme, see for eg.  Numerical Recipes Sec. 5.3 how to evaluate polynomials and derivatives
					d2p_half=dp + d2p_half*root
					dp =p + dp*root
					p  =poly(k)+p*root    ! b_k
					! Adams, Duane A., 1967, "A stopping criterion for polynomial root finding",
					! Communications of the ACM, Volume 10 Issue 10, Oct. 1967, p. 655
					! ftp://reports.stanford.edu/pub/cstr/reports/cs/tr/67/55/CS-TR-67-55.pdf
					! Eq 8.
					ek=absroot*ek+abs(p)
					enddo
					stopping_crit2=(FRAC_ERR*ek)**2
					else
					do k=degree,1,-1 ! Horner Scheme, see for eg.  Numerical Recipes Sec. 5.3 how to evaluate polynomials and derivatives
					d2p_half=dp + d2p_half*root
					dp =p + dp*root
					p  =poly(k)+p*root    ! b_k
					enddo
					endif */
					p = poly(degree);
					dp = zero;
					d2p_half = zero;
					if (((i - j) % 10) == 0) {
						// prepare stopping criterion
						ek = std::abs(poly(degree));
						absroot = std::abs(root);
						for (k = degree; k >= 1; k--) { // Horner Scheme, see for eg.  Numerical Recipes Sec. 5.3 how to evaluate polynomials and derivatives
							d2p_half = dp + d2p_half*root;
							dp = p + dp*root;
							p = poly(k - 1) + p*root;    // b_k
							/*
							! Adams, Duane A., 1967, "A stopping criterion for polynomial root finding",
							! Communications of the ACM, Volume 10 Issue 10, Oct. 1967, p. 655
							! ftp://reports.stanford.edu/pub/cstr/reports/cs/tr/67/55/CS-TR-67-55.pdf
							! Eq 8. */
							ek = absroot*ek + std::abs(p);
						}
						stopping_crit2 = (FRAC_ERR*ek)*(FRAC_ERR*ek);
					}
					else {
						for (k = degree; k >= 1; k--) { // Horner Scheme, see for eg.  Numerical Recipes Sec. 5.3 how to evaluate polynomials and derivatives
							d2p_half = dp + d2p_half*root;
							dp = p + dp*root;
							p = poly(k - 1) + p*root;
						}
					} // b_k

					/*
					abs2p=conjg(p)*p !abs(p)**2
					iter=iter+1
					if(abs2p==0d0) return */
					abs2p = (std::conj(p)*p).real();
					iter = iter + 1;
					if (abs2p == 0.0) return;

					/*
					if(abs2p<stopping_crit2) then ! (simplified a little Eq. 10 of Adams 1967)
					if(dp==zero) return
					! do additional iteration if we are less than 10x from stopping criterion
					if(abs2p<0.01d0*stopping_crit2) then ! ten times better than stopping criterion
					return ! return immediately, because we are at very good place
					else
					good_to_go=.true. ! do one iteration more
					endif
					else
					good_to_go=.false. ! reset if we are outside the zone of the root
					endif */
					if (abs2p < stopping_crit2) { // (simplified a little Eq. 10 of Adams 1967)
						if (dp == zero) return;
						// do additional iteration if we are less than 10x from stopping criterion
						if (abs2p < 0.01*stopping_crit2) { // ten times better than stopping criterion
							return;
						} // return immediately, because we are at very good place
						else good_to_go = true;
					} // do one iteration more
					else good_to_go = false; // reset if we are outside the zone of the root

					/*
					if(dp==zero)then !test if demoninators are > 0.0 not to divide by zero
					dx=(abs(root)+1d0)*exp(cmplx(0d0,FRAC_JUMPS(mod(i,FRAC_JUMP_LEN)+1)*2*pi,RK)) ! make some random jump
					else
					fac_netwon=p/dp
					fac_extra=d2p_half/dp
					F_half=fac_netwon*fac_extra

					abs2_F_half=conjg(F_half)*F_half
					if(abs2_F_half<=0.000625d0)then ! F<0.05, F/2<0.025
					mode=0 ! set Newton's, go there after jump
					endif

					dx=fac_netwon*(c_one+F_half)  ! SG
					endif */
					if (dp == zero) { // test if demoninators are > 0.0 not to divide by zero
						complex<double> tmp(0.0, FRAC_JUMPS[i%FRAC_JUMP_LEN] * 2.0*pi);
						dx = (std::abs(root) + 1.0)*exp(tmp);
					} // make some random jump
					else {
						fac_netwon = p / dp;
						fac_extra = d2p_half / dp;
						F_half = fac_netwon*fac_extra;

						abs2_F_half = (std::conj(F_half)*F_half).real();
						if (abs2_F_half <= 0.000625) { // F<0.05, F/2<0.025
							mode = 0;
						} // set Newton's, go there after jump

						dx = fac_netwon*(c_one + F_half);
					} // SG

					/*
					newroot=root-dx
					if(newroot==root) return ! nothing changes -> return
					if(good_to_go)then       ! this was jump already after stopping criterion was met
					root=newroot
					return
					endif */
					newroot = root - dx;
					if (newroot == root) return; // nothing changes -> return
					if (good_to_go) {          // this was jump already after stopping criterion was met
						root = newroot;
						return;
					}

					/*
					if(mode/=1) then
					root=newroot
					j=i+1    ! remember iteration number
					exit     ! go to Newton's
					endif */
					if (mode != 1) {
						root = newroot;
						j = i + 1;    // remember iteration number
						break;
					}  // go to Newton's

					/*
					if(mod(i,FRAC_JUMP_EVERY)==0) then ! decide whether to do a jump of modified length (to break cycles)
					faq=FRAC_JUMPS(mod(i/FRAC_JUMP_EVERY-1,FRAC_JUMP_LEN)+1)
					newroot=root-faq*dx ! do jump of some semi-random length (0<faq<1)
					endif
					root=newroot */
					if ((i%FRAC_JUMP_EVERY) == 0) { // decide whether to do a jump of modified length (to break cycles)
						faq = FRAC_JUMPS[(i / FRAC_JUMP_EVERY - 1) % FRAC_JUMP_LEN];
						newroot = root - faq*dx;
					} // do jump of some semi-random length (0<faq<1)
					root = newroot;

				} // enddo ! do mode 1

				/*
				if(i>=MAX_ITERS) then
				success=.false.
				return
				endif */
				if (i >= MAX_ITERS) {
					success = false;
					return;
				}

			} // endif ! if mode 1

			//!------------------------------------------------------------- mode 0
			// if(mode==0) then  ! NEWTON'S METHOD
			if (mode == 0) { // NEWTON'S METHOD

				/*
			  do i=j,j+10  ! do only 10 iterations the most, then go back to full Laguerre's
			  faq=1d0 */
				for (i = j; i <= j + 10; i++) {  // do only 10 iterations the most, then go back to full Laguerre's
					faq = 1.0;

					// calculate value of polynomial and its first two derivatives
					/*
					p  =poly(degree+1)
					dp =zero
					if(i==j)then ! calculate stopping crit only once at the begining
					! prepare stoping criterion
					ek=abs(poly(degree+1))
					absroot=abs(root)
					do k=degree,1,-1 ! Horner Scheme, see for eg.  Numerical Recipes Sec. 5.3 how to evaluate polynomials and derivatives
					dp =p + dp*root
					p  =poly(k)+p*root    ! b_k
					! Adams, Duane A., 1967, "A stopping criterion for polynomial root finding",
					! Communications of the ACM, Volume 10 Issue 10, Oct. 1967, p. 655
					! ftp://reports.stanford.edu/pub/cstr/reports/cs/tr/67/55/CS-TR-67-55.pdf
					! Eq 8.
					ek=absroot*ek+abs(p)
					enddo
					stopping_crit2=(FRAC_ERR*ek)**2
					else        !
					do k=degree,1,-1 ! Horner Scheme, see for eg.  Numerical Recipes Sec. 5.3 how to evaluate polynomials and derivatives
					dp =p + dp*root
					p  =poly(k)+p*root    ! b_k
					enddo
					endif
					abs2p=conjg(p)*p !abs(p)**2
					iter=iter+1
					if(abs2p==0d0) return */
					p = poly(degree);
					dp = zero;
					if (i == j) { // calculate stopping crit only once at the begining
						// prepare stopping criterion
						ek = std::abs(poly(degree));
						absroot = std::abs(root);
						for (k = degree; k >= 1; k--) { // Horner Scheme, see for eg.  Numerical Recipes Sec. 5.3 how to evaluate polynomials and derivatives
							dp = p + dp*root;
							p = poly(k - 1) + p*root;   // b_k
							/*
							! Adams, Duane A., 1967, "A stopping criterion for polynomial root finding",
							! Communications of the ACM, Volume 10 Issue 10, Oct. 1967, p. 655
							! ftp://reports.stanford.edu/pub/cstr/reports/cs/tr/67/55/CS-TR-67-55.pdf
							! Eq 8. */
							ek = absroot*ek + std::abs(p);
						}
						stopping_crit2 = (FRAC_ERR*ek)*(FRAC_ERR*ek);
					}
					else {
						for (k = degree; k >= 1; k--) { // Horner Scheme, see for eg.  Numerical Recipes Sec. 5.3 how to evaluate polynomials and derivatives
							dp = p + dp*root;
							p = poly(k - 1) + p*root;
						}
					}   // b_k
					abs2p = (std::conj(p)*p).real();
					iter = iter + 1;
					if (abs2p == 0.0) return;

					/*
					if(abs2p<stopping_crit2) then ! (simplified a little Eq. 10 of Adams 1967)
					if(dp==zero) return
					! do additional iteration if we are less than 10x from stopping criterion
					if(abs2p<0.01d0*stopping_crit2) then ! ten times better than stopping criterion
					return ! return immediately, because we are at very good place
					else
					good_to_go=.true. ! do one iteration more
					endif
					else
					good_to_go=.false. ! reset if we are outside the zone of the root
					endif */
					if (abs2p < stopping_crit2) { // (simplified a little Eq. 10 of Adams 1967)
						if (dp == zero) return;
						// do additional iteration if we are less than 10x from stopping criterion
						if (abs2p < 0.01*stopping_crit2) {  // ten times better than stopping criterion
							return;
						} // return immediately, because we are at very good place
						else good_to_go = true;
					} // do one iteration more
					else good_to_go = false; // reset if we are outside the zone of the root

					/*
					if(dp==zero)then ! test if demoninators are > 0.0 not to divide by zero
					dx=(abs(root)+1d0)*exp(cmplx(0d0,FRAC_JUMPS(mod(i,FRAC_JUMP_LEN)+1)*2*pi,RK)) ! make some random jump
					else
					dx=p/dp
					endif */
					if (dp == zero) { // test if demoninators are > 0.0 not to divide by zero
						complex<double> tmp(0.0, FRAC_JUMPS[i%FRAC_JUMP_LEN] * 2.0*pi);
						dx = (abs(root) + 1.0)*exp(tmp);
					} // make some random jump
					else dx = p / dp;

					/*
				  newroot=root-dx
				  if(newroot==root) return ! nothing changes -> return
				  if(good_to_go)then
				  root=newroot
				  return
				  endif */
					newroot = root - dx;
					if (newroot == root) return; // nothing changes -> return
					if (good_to_go) {
						root = newroot;
						return;
					}

					/*
					! this loop is done only 10 times. So skip this check
					!if(mod(i,FRAC_JUMP_EVERY)==0) then ! decide whether to do a jump of modified length (to break cycles)
					!  faq=FRAC_JUMPS(mod(i/FRAC_JUMP_EVERY-1,FRAC_JUMP_LEN)+1)
					!  newroot=root-faq*dx ! do jump of some semi-random length (0<faq<1)
					!endif */
					// root=newroot
					root = newroot;

				} // enddo ! do mode 0 10 times

				/*
				if (iter>=MAX_ITERS) then
				! too many iterations here
				success=.false.
				return
				endif
				mode=2 ! go back to Laguerre's. This happens when we were unable to converge in 10 iterations with Newton's
				*/
				if (iter >= MAX_ITERS) {
					// too many iterations here
					success = false;
					return;
				}
				mode = 2; // go back to Laguerre's. This happens when we were unable to converge in 10 iterations with Newton's

			} // endif ! if mode 0

		} // enddo ! end of infinite loop

		//-------------------------------------------------------------
		success = false;
	} // end

	/*
	!-------------------------------------------------------------------!
	! _9_                     SOLVE_QUADRATIC_EQ                        !
	!-------------------------------------------------------------------! */
	// subroutine solve_quadratic_eq(x0,x1,poly)
	void solve_quadratic_eq(complex<double>& x0, complex<double>& x1, const Array<complex<double>, 1>& poly) {
		// Quadratic equation solver for complex polynomial (degree=2)
		/*
		implicit none
		complex(kind=8), intent(out) :: x0, x1
		complex(kind=8), dimension(*), intent(in) :: poly ! coeffs of the polynomial
		! poly - is an array of polynomial cooefs, length = degree+1, poly(1) is constant
		!             1              2             3
		!        poly(1) x^0 + poly(2) x^1 + poly(3) x^2 */

		/*
		complex(kind=8) :: a, b, c, b2, delta

		complex(kind=8) :: val, x
		integer :: i */
		complex<double> a, b, c, b2, delta;

		complex<double> val, x, tmp;
		complex<double> zero(0.0, 0.0);
		int i;

		/*
		a=poly(3)
		b=poly(2)
		c=poly(1)
		! quadratic equation: a z^2 + b z + c = 0 */
		a = poly(2);
		b = poly(1);
		c = poly(0);
		// quadratic equation: a z^2 + b z + c = 0

		/*
		b2=b*b
		delta=sqrt(b2-4d0*(a*c))
		if( real(conjg(b)*delta)>=0d0 )then  ! scallar product to decide the sign yielding bigger magnitude
		x0=-0.5d0*(b+delta)
		else
		x0=-0.5d0*(b-delta)
		endif
		if(x0==cmplx(0d0,0d0,8))then
		x1=cmplx(0d0,0d0,8)
		else ! Viete's formula
		x1=c/x0
		x0=x0/a
		endif */
		b2 = b*b;
		delta = std::sqrt(b2 - 4.0*(a*c));
		tmp = std::conj(b)*delta;
		if (tmp.real() >= 0.0) { // scalar product to decide the sign yielding bigger magnitude
			x0 = -0.5*(b + delta);
		}
		else x0 = -0.5*(b - delta);
		if (x0 == zero) x1 = zero;
		else { // Viete's formula
			x1 = c / x0;
			x0 = x0 / a;
		}

		/*
		if(.false.)then  ! print the results

		x=x0
		val=poly(3)
		do i=2,1,-1
		val=val*x+poly(i)
		enddo
		write(*,'(2f19.15,a3,2f19.15)') x,' ->',val

		x=x1
		val=poly(3)
		do i=2,1,-1
		val=val*x+poly(i)
		enddo
		write(*,'(2f19.15,a3,2f19.15)') x,' ->',val

		endif */
	} // end

	/*
	!-------------------------------------------------------------------!
	! _10_                    SOLVE_CUBIC_EQ                            !
	!-------------------------------------------------------------------! */
	// subroutine solve_cubic_eq(x0,x1,x2,poly)
	void solve_cubic_eq(complex<double>& x0, complex<double>& x1, complex<double>& x2, const Array<complex<double>, 1>& poly) {
		// Cubic equation solver for complex polynomial (degree=3)
		// http://en.wikipedia.org/wiki/Cubic_function   Lagrange's method
		/*
	  implicit none
	  complex(kind=8), intent(out) :: x0, x1, x2
	  complex(kind=8), dimension(*), intent(in) :: poly ! coeffs of the polynomial
	  ! poly - is an array of polynomial cooefs, length = degree+1, poly(1) is constant
	  !             1              2             3             4
	  !        poly(1) x^0 + poly(2) x^1 + poly(3) x^2 + poly(4) x^3 */
		/*
	  complex(kind=8), parameter :: zeta =cmplx(-0.5d0, 0.8660254037844386d0, 8)  ! sqrt3(1)
	  complex(kind=8), parameter :: zeta2=cmplx(-0.5d0,-0.8660254037844386d0, 8)  ! sqrt3(1)**2
	  real(kind=8), parameter    :: third=0.3333333333333333d0                    ! 1/3
	  complex(kind=8) :: s0, s1, s2
	  complex(kind=8) :: E1 ! x0+x1+x2
	  complex(kind=8) :: E2 ! x0x1+x1x2+x2x0
	  complex(kind=8) :: E3 ! x0x1x2
	  complex(kind=8) :: A, B, a_1, E12
	  complex(kind=8) :: delta, A2 */
		complex<double> zeta(-0.5, 0.8660254037844386);    // sqrt3(1)
		complex<double> zeta2(-0.50, -0.8660254037844386); // sqrt3(1)**2
		double third = 0.3333333333333333;                  // 1/3
		complex<double> s0, s1, s2;
		complex<double> E1; // x0+x1+x2
		complex<double> E2; // x0x1+x1x2+x2x0
		complex<double> E3; // x0x1x2
		complex<double> A, B, a_1, E12, delta, A2;

		/*
		complex(kind=8) :: val, x
		integer :: i */
		complex<double> val, x, tmp;
		complex<double> zero(0.0, 0.0);
		int i;

		/*
		a_1=poly(4)**(-1)
		E1=-poly(3)*a_1
		E2=poly(2)*a_1
		E3=-poly(1)*a_1 */
		a_1 = 1.0 / poly(3);
		E1 = -poly(2)*a_1;
		E2 = poly(1)*a_1;
		E3 = -poly(0)*a_1;

		/*
		s0=E1
		E12=E1*E1
		A=2d0*E1*E12-9d0*E1*E2+27d0*E3  ! =  s1^3 + s2^3
		B=E12-3d0*E2                    !  = s1 s2
		! quadratic equation: z^2-Az+B^3=0  where roots are equal to s1^3 and s2^3
		A2=A*A
		delta=sqrt(A2-4d0*(B*B*B))
		if( real(conjg(A)*delta)>=0d0 )then ! scallar product to decide the sign yielding bigger magnitude
		s1=(0.5d0*(A+delta))**third
		else
		s1=(0.5d0*(A-delta))**third
		endif
		if(s1==cmplx(0d0,0d0,8))then
		s2=cmplx(0d0,0d0,8)
		else
		s2=B/s1
		endif */
		s0 = E1;
		E12 = E1*E1;
		A = 2.0*E1*E12 - 9.0*E1*E2 + 27.0*E3; // =  s1^3 + s2^3
		B = E12 - 3.0*E2;                   // = s1 s2
		// quadratic equation: z^2-Az+B^3=0  where roots are equal to s1^3 and s2^3
		A2 = A*A;
		delta = std::sqrt(A2 - 4.0*(B*B*B));
		tmp = std::conj(A)*delta;
		if (tmp.real() >= 0.0) { // scalar product to decide the sign yielding bigger magnitude
			s1 = std::pow(0.5*(A + delta), third);
		}
		else s1 = std::pow(0.5*(A - delta), third);
		if (s1 == zero) s2 = zero;
		else s2 = B / s1;

		/*
		x0=third*(s0+s1+s2)
		x1=third*(s0+s1*zeta2+s2*zeta )
		x2=third*(s0+s1*zeta +s2*zeta2) */
		x0 = third*(s0 + s1 + s2);
		x1 = third*(s0 + s1*zeta2 + s2*zeta);
		x2 = third*(s0 + s1*zeta + s2*zeta2);

		/*
		if(.false.)then  ! print the results

		x=x0
		val=poly(4)
		do i=3,1,-1
		val=val*x+poly(i)
		enddo
		write(*,'(2f19.15,a3,2f19.15)') x,' ->',val

		x=x1
		val=poly(4)
		do i=3,1,-1
		val=val*x+poly(i)
		enddo
		write(*,'(2f19.15,a3,2f19.15)') x,' ->',val

		x=x2
		val=poly(4)
		do i=3,1,-1
		val=val*x+poly(i)
		enddo
		write(*,'(2f19.15,a3,2f19.15)') x,' ->',val

		endif */
	} // end

	/*
	!-------------------------------------------------------------------!
	! _11_                    DIVIDE_POLY_1                             !
	!-------------------------------------------------------------------! */
	// subroutine divide_poly_1(polyout, remainder, p, polyin, degree)
	void divide_poly_1(Array<complex<double>, 1>& polyout, complex<double>& remainder, complex<double>& p, Array<complex<double>, 1>& polyin, const int degree) {
		/*
	  ! Subroutine will divide polynomial 'polyin' by (x-p)
	  ! results will be returned in polynomial 'polyout' of degree-1
	  ! The remainder of the division will be returned in 'remainder'
	  !
	  ! You can provide same array as 'polyin' and 'polyout' - this
	  ! routine will work fine, though it will not set to zero the
	  ! unused, highest coefficient in the output array. You just have
	  ! remember the proper degree of a polynomial.
	  implicit none
	  integer, intent(in) :: degree
	  complex(kind=8), dimension(degree), intent(out) :: polyout
	  complex(kind=8), intent(out) :: remainder
	  complex(kind=8), intent(in) :: p
	  complex(kind=8), dimension(degree+1), intent(in) :: polyin ! coeffs of the polynomial
	  ! poly - is an array of polynomial cooefs, length = degree+1, poly(1) is constant
	  !             1              2             3
	  !        poly(1) x^0 + poly(2) x^1 + poly(3) x^2 + ... */
		/*
	  integer :: i
	  complex(kind=8) :: coef, prev */
		int i;
		complex<double> coef, prev;

		/*
		coef=polyin(degree+1)
		polyout=polyin(1:degree)
		do i=degree,1,-1
		prev=polyout(i)
		polyout(i)=coef
		coef=prev+p*coef
		enddo
		remainder=coef
		return */
		coef = polyin(degree);
		polyout(Range(0, degree - 1)) = polyin(Range(0, degree - 1));
		for (i = degree; i >= 1; i++) {
			prev = polyout(i - 1);
			polyout(i - 1) = coef;
			coef = prev + p*coef;
		}
		remainder = coef;
		return;
	} // end

	/*
	!-------------------------------------------------------------------!
	! _12_                    EVAL_POLY                                 !
	!-------------------------------------------------------------------! */
	// complex(kind=8) function eval_poly(x, poly, degree, errk)
	complex<double> eval_poly(complex<double>& x, Array<complex<double>, 1>& poly, const int& degree, double& errk) {
		/*
	  ! Evaluation of the complex polynomial 'poly' of a given degree
	  ! at the point 'x'. This routine calculates also the simplified
	  ! Adams' (1967) stopping criterion. ('errk' should be multiplied
	  ! by 2d-15 for double precisioni,real*8, arithmetic)
	  implicit none
	  complex(kind=8), intent(in) :: x
	  integer, intent(in) :: degree
	  real(kind=8), intent(out) :: errk !
	  complex(kind=8), dimension(degree+1), intent(in) :: poly ! coeffs of the polynomial
	  ! poly - is an array of polynomial cooefs, length = degree+1, poly(1) is constant
	  !             1              2             3
	  !        poly(1) x^0 + poly(2) x^1 + poly(3) x^2 + ... */
		/*
	  integer :: i
	  complex(kind=8) :: val
	  real(kind=8) :: absx */
		int i;
		complex<double> val, result;
		double absx;

		// prepare stopping criterion
		/*
		errk=abs(poly(degree+1))
		val=poly(degree+1)
		absx=abs(x)
		do i=degree,1,-1  ! Horner Scheme, see for eg.  Numerical Recipes Sec. 5.3 how to evaluate polynomials and derivatives
		val=val*x+poly(i)
		! Adams, Duane A., 1967, "A stopping criterion for polynomial root finding",
		! Communications of the ACM, Volume 10 Issue 10, Oct. 1967, p. 655
		! ftp://reports.stanford.edu/pub/cstr/reports/cs/tr/67/55/CS-TR-67-55.pdf
		! Eq 8.
		errk=errk*absx+abs(val)
		enddo
		eval_poly=val */
		errk = std::abs(poly(degree));
		val = poly(degree);
		absx = std::abs(x);
		for (i = degree; i >= 1; i--) { // Horner Scheme, see for eg.  Numerical Recipes Sec. 5.3 how to evaluate polynomials and derivatives
			val = val*x + poly(i - 1);
			/*
			! Adams, Duane A., 1967, "A stopping criterion for polynomial root finding",
			! Communications of the ACM, Volume 10 Issue 10, Oct. 1967, p. 655
			! ftp://reports.stanford.edu/pub/cstr/reports/cs/tr/67/55/CS-TR-67-55.pdf
			! Eq 8. */
			errk = errk*absx + abs(val);
		}
		result = val;

		// ! if(abs(val)<2d-15*errk) return  ! (simplified a little Eq. 10 of Adams 1967)

		return result;
	} // end

	/*
	!-------------------------------------------------------------------!
	! _13_                    MULTIPLY_POLY_1                           !
	!-------------------------------------------------------------------! */
	// subroutine multiply_poly_1(polyout, p, polyin, degree)
	void multiply_poly_1(Array<complex<double>, 1>& polyout, complex<double>& p, Array<complex<double>, 1>& polyin, const int& degree) {
		/*
	  ! Subroutine will multiply polynomial 'polyin' by (x-p)
	  ! results will be returned in polynomial 'polyout' of degree+1
	  !
	  ! You can provide same array as 'polyin' and 'polyout' - this
	  ! routine will work fine.
	  implicit none
	  integer, intent(in) :: degree  ! OLD degree, new will be +1
	  complex(kind=8), dimension(degree+2), intent(out) :: polyout
	  complex(kind=8), intent(in) :: p
	  complex(kind=8), dimension(degree+1), intent(in) :: polyin ! coeffs of the polynomial
	  ! poly - is an array of polynomial cooefs, length = degree+1, poly(1) is constant
	  !             1              2             3
	  !        poly(1) x^0 + poly(2) x^1 + poly(3) x^2 + ... */
		int i;

		// polyout(1:degree+1)=polyin(1:degree+1) ! copy
		polyout(Range(0, degree)) = polyin(Range(0, degree)); // copy

		/*
		polyout(degree+2)=polyout(degree+1)
		do i=degree+1,2,-1
		polyout(i)=polyout(i-1)-polyout(i)*p
		enddo
		polyout(1)=-polyout(1)*p
		return */
		polyout(degree + 1) = polyout(degree);
		for (i = degree + 1; i >= 2; i--) polyout(i - 1) = polyout(i - 2) - polyout(i - 1)*p;
		polyout(0) = -polyout(0)*p;
		return;
	} // end


	/*
	!-------------------------------------------------------------------!
	! _14_                    CREATE_POLY_FROM_ROOTS                    !
	!-------------------------------------------------------------------! */
	// subroutine create_poly_from_roots(poly, degree, a, roots)
	void create_poly_from_roots(Array<complex<double>, 1>& poly, const int& degree, complex<double>& a, Array<complex<double>, 1>& roots) {
		/*
	  ! Routine will build polynomial from a set of points given in
	  ! the array 'roots'. These points will be zeros of the resulting
	  ! polynomial.
	  !
	  ! poly - is an array of polynomial cooefs, length = degree+1, poly(1) is constant
	  !             1              2             3
	  !        poly(1) x^0 + poly(2) x^1 + poly(3) x^2 + ...
	  ! degree - is and integer denoting size of the 'roots' array
	  ! a - gives the leading coefficient of the resutling polynomial
	  ! roots - input array of points, size=degree
	  !
	  ! This subroutine works, but it is not optimal - it will work
	  ! up to a polynomial of degree~50, if you would like to have
	  ! more robust routine, up to a degree ~ 2000, you should
	  ! split your factors into a binary tree, and then multiply
	  ! leafs level by level from down up with subroutine like:
	  ! multiply_poly for arbitraty polynomial multiplications
	  ! not multiply_poly_1.
	  !
	  implicit none
	  integer, intent(in) :: degree
	  complex(kind=8), dimension(degree+1), intent(out) :: poly
	  complex(kind=8), intent(in) :: a
	  complex(kind=8), dimension(degree), intent(in) :: roots
	  */
		// integer :: i
		int i;
		complex<double> zero(0.0, 0.0);

		poly = zero;
		poly(0) = a;  // leading coeff of the polynomial

		/*
		do i=1,degree
		call multiply_poly_1(poly, roots(i), poly, i-1)
		enddo */
		for (i = 1; i <= degree; i++) multiply_poly_1(poly, roots(i - 1), poly, i - 1);
	} // end
}
// !-------------------------------------------------------------------!
