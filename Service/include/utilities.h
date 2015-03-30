#ifndef GUARD_utilities_h
#define GUARD_utilities_h

#undef min
#undef max

double max(double x, double y);
double min(double x, double y);
double bound(double a, double x, double b);

#define min(a,b)  ((a < b) ? a : b)
#define max(a,b)  ((a > b) ? a : b)

#endif
