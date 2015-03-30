#include "cumnorm2.h"
#include <math.h>
#include "utilities.h"

const double XX[10][3] =
{
	-0.932469514203152,-0.981560634246719,-0.993128599185095,
	-0.661209386466265,-0.904117256370475,-0.963971927277914,
	-0.238619186083197,-0.769902674194305,-0.912234428251326,
	0.,-0.587317954286617,-0.839116971822219,
	0.,-0.36783149899818,-0.746331906460151,
	0.,-0.125233408511469,-0.636053680726515,
	0.,0.,-0.510867001950827,
	0.,0.,-0.37370608871542,
	0.,0.,-0.227785851141645,
	0.,0.,-0.0765265211334973
};

const double W[10][3] =
{
	0.17132449237917,0.0471753363865118,0.0176140071391521,
	0.360761573048138,0.106939325995318,0.0406014298003869,
	0.46791393457269,0.160078328543346,0.0626720483341091,
	0.,0.203167426723066,0.0832767415767048,
	0.,0.233492536538355,0.10193011981724,
	0.,0.249147045813403,0.118194531961518,
	0.,0.,0.131688638449177,
	0.,0.,0.142096109318382,
	0.,0.,0.149172986472604,
	0.,0.,0.152753387130726
};

double bivarcumnorm(double x, double y, double correlation)
{

	int NG;
	int LG;
	
	if (fabs(correlation) < 0.3)
	{
		NG = 1;
		LG = 3;
	}
	else if (fabs(correlation) < 0.75)
	{
		NG = 2;
		LG = 6;
	}
	else 
	{
		NG = 3;
		LG = 10;
	}

	double h = -x;
	double k = -y;
	double hk = h * k;
	double BVN = 0;

	if (fabs(correlation) < 0.925)
	{
		if (fabs(correlation) > 0)
		{
		    double hs = (h * h + k * k) / 2;
		    double asr = asin(correlation);
			for (int i = 1; i <= LG; ++i)
			{
				for (int iss = -1; iss <=1; iss += 2)
				{
					double sn = sin(asr * (iss * XX[i-1][NG-1] + 1) * 0.5);
					BVN = BVN + W[i-1][NG-1] * exp((sn * hk - hs) / (1.0 - sn * sn));
				}
			}
			BVN = BVN * asr * 0.795774715459476678e-1;
		}
		BVN = BVN + cumnorm(-h) * cumnorm(-k);
	}
	else
	{
		if (correlation < 0) 
		{
			k *= -1;
			hk *= -1;
		}
		if (fabs(correlation) < 1)
		{
		    double Ass = (1 - correlation) * (1 + correlation);
			double a = sqrt(Ass);
			double bs = (h-k)*(h-k);
			double c = (4 - hk) / 8;
			double d = (12 - hk) / 16;
			double asr = -(bs / Ass + hk) / 2;
			if (asr > -100)
			{
				BVN = a * exp(asr) * (1 - c * (bs - Ass) * (1 - d * bs / 5) / 3 + c * d * Ass * Ass / 5);
			}
			if (-hk < 100)
			{
				double B = sqrt(bs);
				BVN = BVN - exp(-hk / 2) * 2.506628274631 * cumnorm(-B / a) * B * (1 - c * bs * (1 - d * bs / 5) / 3);
			}
			a /= 2;
			for (int i = 1; i <= LG; ++i)
			{
				for (int iss = -1; iss <= 1; iss += 2)
				{
					double xs = a * (iss * XX[i-1][NG-1] + 1);	
					xs = fabs(xs*xs);
					double rs = sqrt(1 - xs);
					asr = -(bs / xs + hk) / 2;
					if (asr > -100)
					{
						BVN = BVN + a * W[i-1][NG-1] * exp(asr) * (exp(-hk * (1 - rs) / (2 * (1 + rs))) / rs - (1 + c * xs * (1 + d * xs)));
					}	
				}
			}
			BVN *= - 0.159154943091895336;
		}
		if (correlation > 0)
		{
			BVN = BVN + cumnorm(-max(h, k));
		}
		else
		{
			BVN *= -1;
			if (k > h)
			{
				BVN = BVN + cumnorm(k) - cumnorm(h);
			}
		}
	}	 
	return BVN;
}
