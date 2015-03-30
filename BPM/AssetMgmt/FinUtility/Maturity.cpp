/*
Modified and ported: 2013, Nathan Muruganantha. All rights reserved.
*/

#include "Daycount.hpp"
#include "Maturity.hpp"

namespace derivative 
{
	dd::date  Maturity::getNextDate(const dd::date& start, const MaturityType& maturity)
	{
		dd::date retDate;
		switch (maturity)
		{
		case O:
			retDate = start + dd::date_duration(1);
			break;
		case W1:
			retDate = start + dd::weeks_duration(1);
			break;
		case W2:
			retDate = start + dd::weeks_duration(2);
			break;
		case W3:
			retDate = start + dd::weeks_duration(3);
			break;
		case W4:
			retDate = start + dd::weeks_duration(4);
			break;
		case M1:
			{
				dd::month_iterator itr(start, 1);
				++itr;
				retDate = *itr; 
			}
			break;
		case M2:
			{
				dd::month_iterator itr(start, 2);
				++itr;
				retDate = *itr; 
			}
			break;
		case M3:
			{
				dd::month_iterator itr(start, 3);
				++itr;
				retDate = *itr; 
			}
			break;
		case M4:
			{
				dd::month_iterator itr(start, 4);
				++itr;
				retDate = *itr; 
			}
			break;
		case M5:
			{
				dd::month_iterator itr(start, 5);
				++itr;
				retDate = *itr; 
			}
			break;
		case M6:
			{
				dd::month_iterator itr(start, 6);
				++itr;
				retDate = *itr;
			}
			break;
		case M7:
			{
				dd::month_iterator itr(start, 7);
				++itr;
				retDate = *itr;
			}
			break;
		case M8:
			{
				dd::month_iterator itr(start, 8);
				++itr;
				retDate = *itr; 
			}
			break;
		case M9:
			{
				dd::month_iterator itr(start, 9);
				++itr;
				retDate = *itr; 
			}
			break;
		case M10:
			{
				dd::month_iterator itr(start, 10);
				++itr;
				retDate = *itr;
			}
			break;
		case M11:
			{
				dd::month_iterator itr(start, 11);
				++itr;
				retDate = *itr; 
			}
			break;
		case Y1:
			{
				dd::year_iterator itr(start, 1);
				++itr;
				retDate = *itr; 
			}
			break;
		case Y2:
			{
				dd::year_iterator itr(start, 2);
				++itr;
				retDate = *itr; 
			}
			break;
		case Y3:
			{
				dd::year_iterator itr(start, 3);
				++itr;
				retDate = *itr; 
			}
			break;
		case Y4:
			{
				dd::year_iterator itr(start, 4);
				++itr;
				retDate = *itr;
			}
			break;
		case Y5:
			{
				dd::year_iterator itr(start, 5);
				++itr;
				retDate = *itr; 
			}
			break;
		case Y6:
			{
				dd::year_iterator itr(start, 6);
				++itr;
				retDate = *itr; 
			}
			break;
		case Y7:
			{
				dd::year_iterator itr(start, 7);
				++itr;
				retDate = *itr; 
			}
			break;
		case Y8:
			{
				dd::year_iterator itr(start, 8);
				++itr;
				retDate = *itr; 
			}
			break;
		case Y9:
			{
				dd::year_iterator itr(start, 9);
				++itr;
				retDate = *itr; 
			}
			break;
		case Y10:
			{
				dd::year_iterator itr(start, 10);
				++itr;
				retDate = *itr;  
			}
			break;
		case Y20:
			{
				dd::year_iterator itr(start, 20);
				++itr;
				retDate = *itr; 
			}
			break;
		case Y30:
			{
				dd::year_iterator itr(start, 30);
				++itr;
				retDate = *itr; 
			}
			break;
		default:
			throw std::logic_error("Invalid case statement ");
		}	
		return retDate;
	}

	Maturity::MaturityType Maturity::getMaturity(int days)
	{
		if (days < 7) return Maturity::W1;
		if (days < 14) return Maturity::W1;
		if (days < 31) return  Maturity::M1;
		if (days < 182) return  Maturity::M6;
		if (days < 365) return  Maturity::Y1;
		if (days < 365*2) return  Maturity::Y2;
		if (days < 365*3) return  Maturity::Y3;
		if (days < 365*4) return  Maturity::Y4;
		if (days < 365*5) return  Maturity::Y5;
		if (days < 365*6) return  Maturity::Y6;
		if (days < 365*7) return  Maturity::Y7;
		if (days < 365*8) return  Maturity::Y8;
		if (days < 365*9) return  Maturity::Y9;
		if (days < 365*10) return  Maturity::Y10;
		if (days < 365*20) return  Maturity::Y20;
		if (days <= 365 *30) return  Maturity::Y30;
	}
}