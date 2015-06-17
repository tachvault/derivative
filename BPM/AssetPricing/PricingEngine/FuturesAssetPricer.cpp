/*
Copyright (C) Nathan Muruganantha 2013 - 2014
*/

#include <functional>
#include <random/normal.h>
#include <boost/math/distributions/normal.hpp>

#include "DException.hpp"
#include "SystemUtil.hpp"
#include "EntityMgrUtil.hpp"
#include "MsgRegister.hpp"
#include "GroupRegister.hpp"

#include "IStock.hpp"
#include "IStockValue.hpp"
#include "BlackScholesAssetAdapter.hpp"
#include "PrimaryAssetUtil.hpp"
#include "IRCurve.hpp"

#include "ConstVol.hpp"
#include "PiecewiseVol.hpp"
#include "EquityVolatilitySurface.hpp"

#include "GeometricBrownianMotion.hpp"
#include "Payoff.hpp"
#include "MCMapping.hpp"
#include "FiniteDifference.hpp"
#include "Binomial.hpp"
#include "LongstaffSchwartz.hpp"
#include "MCAmerican.hpp"
#include "MCGeneric.hpp"
#include "MExotics.hpp"
#include "QFRandom.hpp"

#include "ExchangeRateAssetPricer.hpp"

namespace derivative
{
	namespace ExchangeRateVanillaOptionPricer
	{
		double ValueAmericanWithBinomial(const std::shared_ptr<BlackScholesAssetAdapter>& futures, double rate, \
			dd::date maturity, double strike, int optType, int N)
		{
			try
			{
				double mat = (double((maturity - dd::day_clock::local_day()).days())) / 365;
				BinomialLattice btree(futures, rate, mat, N);
				Payoff optPayoff(strike, optType);
				std::function<double(double)> f;
				f = std::bind(std::mem_fun(&Payoff::operator()), &optPayoff, std::placeholders::_1);
				btree.apply_payoff(N - 1, f);
				EarlyExercise amOpt(optPayoff);
				std::function<double(double, double)> g;
				g = std::bind(std::mem_fn(&EarlyExercise::operator()), &amOpt, std::placeholders::_1, std::placeholders::_2);
				btree.set_CoxRossRubinstein();
				btree.apply_payoff(N - 1, f);
				btree.rollback(N - 1, 0, g);
				return btree.result();

			} // end of try block
			catch (std::logic_error& e)
			{
				LOG(ERROR) << e.what() << endl;
				throw e;
			}
			catch (std::runtime_error& e)
			{
				LOG(ERROR) << e.what() << endl;
				throw e;
			}
		};

		double ValueEuropeanWithBinomial(const std::shared_ptr<BlackScholesAssetAdapter>& futures, double rate, \
			dd::date maturity, double strike, int optType, int N)
		{
			try
			{
				double mat = (double((maturity - dd::day_clock::local_day()).days())) / 365;
				BinomialLattice btree(futures, rate, mat, N);
				Payoff optPayoff(strike, optType);
				std::function<double(double)> f;
				f = std::bind(std::mem_fun(&Payoff::operator()), &optPayoff, std::placeholders::_1);
				btree.apply_payoff(N - 1, f);
				btree.rollback(N - 1, 0);
				return btree.result();

			} // end of try block
			catch (std::logic_error& e)
			{
				LOG(ERROR) << e.what() << endl;
				throw e;
			}
			catch (std::runtime_error& e)
			{
				LOG(ERROR) << e.what() << endl;
				throw e;
			}
		};
	}
}