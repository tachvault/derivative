#include "cumnorm3.h"
#include <math.h>

#include "utilities.h"

const double epsilon = pow(10.0,-15);

double pntgnd(double ba, double bb, double bc, double ra, double rb, double r, double rr)
{
	//Computes Plackett formula integrand
	double pnt = 0;
	double dt = rr * (rr - pow(ra - rb,2) - 2 * ra * rb * (1 - r));
	if (dt > 0)
	{
	double bt = (bc * rr + ba * (r * rb - ra) + bb * (r * ra - rb)) / sqrt(dt);
		double ft = pow(ba - r * bb,2) / rr + pow(bb,2);
		if (bt > -10 && ft < 100)
		{
			pnt = exp(-ft / 2);
			if (bt < 10)
			{
				pnt = pnt * cumnorm(bt);
			}
		}
	}
	return pnt;
}

double Tvtmfn(double x, double l1, double l2, double l3, double correl21, double correl31, double correl32)
{
	//Computes Plackett formula integrands
	double TV = 0;
	double rua = asin(correl21);
	double rub = asin(correl31);
	double y[5];
	y[1] = sin(rua * x);
	y[2] = pow(cos(rua * x), 2);
	y[3] = sin(rub * x);
	y[4] = pow(cos(rub * x),2);
	if (fabs(rua) > 0)
	{
		TV = rua * pntgnd(l1, l2, l3, y[3], correl32, y[1], y[2]);
	}
	if (fabs(rub) > 0)
	{
		TV = TV + rub * pntgnd(l1, l3, l2, y[1], correl32, y[3], y[4]);
	}
	return TV;
}

void Kronrod(double lower, double upper, double l1, double l2, double l3, double correl21, double correl31, double correl32, double *WG, double *WGK, double *XGK, double &Kronrodint, double &Kronroderr)
{
	//Kronrod Rule on interval [lower,upper]
	double Halflength = (upper - lower) / 2;
	double Centre = (lower + upper) / 2;
	double fc = Tvtmfn(Centre, l1, l2, l3, correl21, correl31, correl32);
	double Resltg = fc * WG[0];
	double Resltk = fc * WGK[0];
	int vector;
	for (vector = 1; vector <=11; ++vector)
	{
    double Abscis = Halflength * XGK[vector];
		double FunSum = Tvtmfn(Centre - Abscis, l1, l2, l3, correl21, correl31, correl32) + Tvtmfn(Centre + Abscis, l1, l2, l3, correl21, correl31, correl32);
		Resltk = Resltk + WGK[vector] * FunSum;
		if (0 == vector % 2 )
		{
			Resltg = Resltg + WG[vector / 2] * FunSum;
		}
	}
	Kronrodint = Resltk * Halflength;
	Kronroderr = fabs(Resltg - Resltk) * Halflength;
}


double trivarcumnorm(double x, double y, double z, double correl12, double correl13, double correl23)
{
	double WG[6];
	double WGK[12];
	double XGK[12];

	WG[0] = 0.272925086777901;
	WG[1] = 5.56685671161745E-02;
	WG[2] = 0.125580369464905;
	WG[3] = 0.186290210927735;
	WG[4] = 0.233193764591991;
	WG[5] = 0.262804544510248;
	WGK[0] = 0.136577794711118;
	WGK[1] = 9.76544104596129E-03;
	WGK[2] = 2.71565546821044E-02;
	WGK[3] = 4.58293785644267E-02;
	WGK[4] = 6.30974247503748E-02;
	WGK[5] = 7.86645719322276E-02;
	WGK[6] = 9.29530985969007E-02;
	WGK[7] = 0.105872074481389;
	WGK[8] = 0.116739502461047;
	WGK[9] = 0.125158799100319;
	WGK[10] = 0.131280684229806;
	WGK[11] = 0.135193572799885;
	XGK[0] = 0.0;
	XGK[1] = 0.996369613889543;
	XGK[2] = 0.978228658146057;
	XGK[3] = 0.941677108578068;
	XGK[4] = 0.887062599768095;
	XGK[5] = 0.816057456656221;
	XGK[6] = 0.730152005574049;
	XGK[7] = 0.630599520161965;
	XGK[8] = 0.519096129206812;
	XGK[9] = 0.397944140952378;
	XGK[10] = 0.269543155952345;
	XGK[11] = 0.136113000799362;

	
	double TVCN = 0;
	double limit[4];
	limit[1] = x;
	limit[2] = y;
	limit[3] = z;
	double correlation[4][4];
	correlation[2][1] = correl12;
	correlation[3][1] = correl13;
	correlation[3][2] = correl23;

	if (fabs(correlation[2][1]) > fabs(correlation[3][1]))
	{
		limit[2] = z;
		limit[3] = y;
		correlation[2][1] = correl13;
		correlation[3][1] = correl12;
	}
	if (fabs(correlation[3][1]) > fabs(correlation[3][2]))
	{
		limit[1] = limit[2];
		limit[2] = x;
		correlation[3][2] = correlation[3][1];
		correlation[3][1] = correl23;
	}
	TVCN = 0;

	if (fabs(limit[1]) + fabs(limit[2]) + fabs(limit[3]) < epsilon)
	{
		TVCN = (1 + (asin(correlation[2][1]) + asin(correlation[3][1]) + asin(correlation[3][2])) / asin(1.0)) / 8;
	}
	else if (fabs(correlation[2][1]) + fabs(correlation[3][1]) < epsilon)
	{
		TVCN = cumnorm(limit[1]) * bivarcumnorm(limit[2], limit[3], correlation[3][2]);
	}
	else if (fabs(correlation[3][1]) + fabs(correlation[3][2]) < epsilon)
	{
		TVCN = cumnorm(limit[3]) * bivarcumnorm(limit[1], limit[2], correlation[2][1]);
	}
	else if (fabs(correlation[2][1]) + fabs(correlation[3][2]) < epsilon)
	{
		TVCN = cumnorm(limit[2]) * bivarcumnorm(limit[1], limit[3], correlation[3][1]);
	}
	else if (1 - correlation[3][2] < epsilon)
	{
		TVCN = bivarcumnorm(limit[1], min(limit[2], limit[3]), correlation[2][1]);
	}
	else if (correlation[3][2] + 1 < epsilon)
	{
		if (limit[2] > -limit[3])
		{
			TVCN = bivarcumnorm(limit[1], limit[2], correlation[2][1]) - bivarcumnorm(limit[1], -limit[3], correlation[2][1]);
		}
		else
		{
			TVCN = 0;
		}
	}
	else
	{
	    TVCN = bivarcumnorm(limit[2], limit[3], correlation[3][2]) * cumnorm(limit[1]);
		double A[11];
		double B[11];
		double kf[11];
		double ke[11];
		ke[0] = 0.0;
		int k = 1;
		int j = 1;
		A[1] = 0.0;
		B[1] = 1.0;
		double Kronrodint;
		double Kronroderr;
		double Err;
		double Fin;
		int i;

		do
		{
		    j = ++j;
			B[j] = B[k];
			A[j] = (A[k] + B[k]) / 2;
			B[k] = A[j];
			Kronrod(A[k], B[k], limit[1], limit[2], limit[3], correlation[2][1], correlation[3][1], correlation[3][2], WG, WGK, XGK, Kronrodint, Kronroderr);
			kf[k] = Kronrodint;
			ke[k] = Kronroderr;
			Kronrod(A[j], B[j], limit[1], limit[2], limit[3], correlation[2][1], correlation[3][1], correlation[3][2], WG, WGK, XGK, Kronrodint, Kronroderr);
			kf[j] = Kronrodint;
			ke[j] = Kronroderr;
			Err = 0;
			Fin = 0;
			for (i = 1; i<=j; ++i)
			{
				if (ke[i] > ke[k])
				{
					k = i;
				}
				Fin = Fin + kf[i];
				Err = Err + pow(ke[i],2);
			}
			Err = sqrt(Err);
		}
		while (4 * Err > epsilon && j < 10);
		TVCN = TVCN + Fin / (4 * asin(1.0));
	}
	return TVCN;
}
