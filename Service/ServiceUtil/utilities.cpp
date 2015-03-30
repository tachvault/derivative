#include "utilities.h"

#undef min
#undef max

double max(double x, double y)
{
	return x < y ? y : x;
}

double min(double x, double y)
{
	return x < y ? x : y;
}

double bound(double a, double x, double b)
{
	return max(a,min(x,b));
}
