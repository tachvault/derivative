/*
Modified and ported: 2013, Nathan Muruganantha. All rights reserved.
*/

#include "VolatilitySmile.hpp"

namespace derivative
{
	VolatilitySmile::VolatilitySmile(const Array<int, 1>& K, const Array<double, 1>& vol)
		:m_strikes(K),
		m_vols(vol)
	{}
}