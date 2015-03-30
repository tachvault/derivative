/*
Modified and ported: 2013, Nathan Muruganantha. All rights reserved.
*/

#include "daycount.hpp"
#include "Name.hpp"
#include "EntityManager.hpp"

namespace derivative 
{
	double DayCount::getPeriod(const dd::date& start, const dd::date& end, const DayCountType& countType)
	{
		double period = (end - start).days();
		double periodFraction = 0;
		switch (countType )
		{
		case _30_360:
			periodFraction = period/360;
			break;
		case actual_actual:
			{
				/// add the fraction year of start year
				periodFraction += (dd::date(start.year(), 12, 31) - start).days()/(dd::date(start.year(), 12, 31) - dd::date(start.year(), 1, 1)).days();
				dd::year_iterator y_itr(start, 1);
				/// move until the end year
				++y_itr;     
				dd::date temp = *y_itr;
				while (temp.year() < end.year())
				{
					periodFraction += 1;
				}
				/// add the last leg			
				if (temp.year() == end.year())
				{
					periodFraction += (end - dd::date(start.year(), 1, 1)).days()/(dd::date(end.year(), 12, 31) - dd::date(end.year(), 1, 1)).days();
				}
			}
			break;
		case actual_365:
			periodFraction = period/365.0;
			break;
		case actual_360:
			periodFraction = period/360.0;
			break;
		case _30_360_European:
			periodFraction = period/360.0;
			break;
		default:
			throw std::logic_error("Invalid case statement ");
		}
		return periodFraction;
	}	
}