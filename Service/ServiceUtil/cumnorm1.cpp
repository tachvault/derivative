
#include "cumnorm1.h"
#include <math.h>
#include "utilities.h"

const double M_SQRT1_2PI = 0.3989422804014327;

double cumnorm(double x)
{
	double cn = 0;
	
	double xabs = fabs(x);
	if (xabs > 37) 
	{
		cn = 0;
	}
	else
	{	double exponential = exp(-pow(xabs,2)/2);
		//if (xabs < 10 * M_SQRT1_2) 
		if (xabs < 10 * M_SQRT1_2PI) 
		{
			double build = 3.52624965998911E-02 * xabs + 0.700383064443688;
			build = build * xabs + 6.37396220353165;
			build = build * xabs + 33.912866078383;
			build = build * xabs + 112.079291497871;
			build = build * xabs + 221.213596169931;
			build = build * xabs + 220.206867912376;
			cn = exponential * build;
			build = 8.83883476483184E-02 * xabs + 1.75566716318264;
			build = build * xabs + 16.064177579207;
			build = build * xabs + 86.7807322029461;
			build = build * xabs + 296.564248779674;
			build = build * xabs + 637.333633378831;
			build = build * xabs + 793.826512519948;
			build = build * xabs + 440.413735824752;
			cn = cn / build;
		}
		else
		{
			double build = xabs + 0.65;
			build = xabs + 4.0 / build;
			build = xabs + 3.0 / build;
			build = xabs + 2.0 / build;
			build = xabs + 1.0 / build;
			cn = exponential / build / 2.506628274631;
		}
	}

	if (x > 0) 
	{
		cn = 1 - cn;
	}
	return cn;
}



double cumnorm6(double x )
{
	//Abramowitz and Stegun - 6dp accuracy.
	double CNS = 0;
	if (0==x)
	{
		CNS = 0.5;
	}
	else
	{
		double T = 1 / (1 + 0.2316419 * fabs(x));
		double Sum = 0.31938153 * T - 0.356563782 * pow(T,2) + 1.781477937 * pow(T,3) - 1.821255978 * pow(T,4) + 1.330274429 * pow(T,5);
		if (x < 0)
		{
			CNS = 0.39894228 * exp(-pow(x,2)/ 2) * Sum;
		}
		else
		{
			CNS = 1 - 0.39894228 * exp(-pow(x,2) / 2) * Sum;
		}
	}
	return CNS;
}

double NPrime(double x )
{
	//derivative of cumulative normal distribution
	return exp(-0.5 * pow(x,2)) * M_SQRT1_2PI;
}

double NPrimePrime(double x)
{
	//second derivative of cumulative normal distribution
	return -x * NPrime(x);
}

double CumNormInverse(double y)
{
	//Moro method
	double zz = 0;
    double z = y - 0.5;
    if (fabs(z) < 0.42)
	{
		zz = z*z;
        zz = z * (((-25.44106049637 * zz + 41.39119773534) * zz + -18.61500062529) * zz + 2.50662823884) / 
        ((((3.13082909833 * zz + -21.06224101826) * zz + 23.08336743743) * zz + -8.4735109309) * zz + 1);
	}
    else
	{
        if (z > 0)
		{
			zz = log(-log(1 - y));
		}
        else
		{
			zz = log(-log(y));
		}
		double build = 2.888167364E-07 + zz * 3.960315187E-07;
		build = 3.21767881768E-05 + zz * build;
		build = 3.951896511919E-04 + zz * build;
		build = 3.8405729373609E-03 + zz * build;
		build = 2.76438810333863E-02 + zz * build;
		build = 0.160797971491821 + zz * build;
		build = 0.976169019091719 + zz * build;
		zz = 0.337475482272615 + zz * build;
        if (z <= 0)
		{
			zz = -zz;
		}
	}
    return zz;
}
