#include <iostream>
#include <ios>
#include <math.h>
#include <iomanip>

#include "cumnorm1.h"
#include "cumnorm2.h"
#include "cumnorm3.h"

int main()
{
	cout << "Press 1, 2 or 3 for the dim you want to test." << endl;
	int dim;
	cin >> dim;
	if (dim == 1)
		{
			cout << "UNIVARIATE TEST" << endl;
			cout << "Give the value of x." << endl;
			double x;
			cin >> x;
			cout << setprecision(6) <<"The value of N(" << x << ") is " 
				<< setprecision(20) << cumnorm(x) << endl;
		}
	else if (dim == 2)
		{
			cout << "BIVARIATE TEST" << endl;
			cout << "Give the values of x, y and rho." << endl;
			double x, y, rho;
			cin >> x >> y >> rho;
			cout << setprecision(6) 
				<< "The value of N_2(" << x << "," << y << "," << rho << ") is " 
				<< setprecision(20) << bivarcumnorm(x,y,rho) << endl;
		}
	else if (dim == 3)
		{
			cout << "TRIVARIATE TEST" << endl;
			cout << "Give the values of x, y and z" << endl;
			double x, y, z;
			cin >> x >> y >> z;
			cout << "Give the value of rho12, rho13 and rho23" << endl;
			double rho12, rho13, rho23;
			cin >> rho12 >> rho13 >> rho23;
			cout << setprecision(6) 
				<< "The value of N_3(" << x << "," << y << "," << z << "," << rho12 << "," 
				<< rho13 << "," << rho23 << ") is " << setprecision(20) 
				<< trivarcumnorm(x,y,z,rho12,rho13,rho23) << endl;
		}
	
	cout << "Press any key, and then return, to end" << endl;
	char dummy;
	cin >> dummy;
	return 0;
}
