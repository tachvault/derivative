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

#include "BlackScholesAssetAdapter.hpp"
#include "PrimaryAssetUtil.hpp"
#include "IRCurve.hpp"
#include "ConstVol.hpp"
#include "PiecewiseVol.hpp"

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
#include "PseudoRandomArray.hpp"

namespace derivative
{
	double GeneralMC(std::vector<std::shared_ptr<BlackScholesAssetAdapter> > & assets, MCPayoff& payoff, \
		std::shared_ptr<TermStructure> term, double tenor, int numeraire_index, size_t sim, size_t N, \
		double ci, size_t max_sim)
	{
		try
		{
			int numeraire_index = -1;
			Array<double, 1> T(N + 1);
			firstIndex idx;
			double dt = tenor / N;
			T = idx*dt;
			TermStructure& ts = *term;

			// instantiate stochastic process
			GeometricBrownianMotion gbm(assets);
			gbm.set_numeraire(numeraire_index);
			gbm.set_timeline(T);

			// instantiate random number generator
			PseudoRandomArray<PseudoRandom<ranlib::NormalUnit<double>, double>, double> random_container(gbm.factors(), gbm.number_of_steps());
			MCGatherer<double> mcgatherer;
			// instantiate MCMapping and bind to functor
			MCMapping<GeometricBrownianMotion, Array<double, 2> > mc_mapping(payoff, gbm, ts, numeraire_index);
			std::function<double(Array<double, 2>)> func = std::bind(&MCMapping<GeometricBrownianMotion, Array<double, 2> >::mapping, &mc_mapping, std::placeholders::_1);
			// instantiate generic Monte Carlo algorithm object
			MCGeneric<Array<double, 2>&, double, PseudoRandomArray<PseudoRandom<ranlib::NormalUnit<double>, double>, double> > mc(func, random_container, max_sim);
			// run Monte Carlo
			mc.simulate(mcgatherer, sim);
			return mcgatherer.mean();
		} // end of try block
		catch (std::logic_error& xcpt)
		{
			LOG(ERROR) << xcpt.what() << endl;
			throw xcpt;
		}
		catch (std::runtime_error xcpt)
		{
			LOG(ERROR) << xcpt.what() << endl;
			throw xcpt;
		}
		catch (...)
		{
			LOG(ERROR) << "Other exception caught" << endl;
			throw std::exception("unknown error in MC valuation");
		}
	}

	double AntitheticMC(std::vector<std::shared_ptr<BlackScholesAssetAdapter> > & assets, MCPayoff& payoff, \
		std::shared_ptr<TermStructure> term, double tenor, int numeraire_index, size_t sim, size_t N, \
		double ci, size_t max_sim)
	{
		try
		{
			Array<double, 1> T(N + 1);
			firstIndex idx;
			double dt = tenor / N;
			T = idx*dt;
			TermStructure& ts = *term;

			// instantiate stochastic process
			GeometricBrownianMotion gbm(assets);
			gbm.set_numeraire(numeraire_index);
			gbm.set_timeline(T);

			/// instantiate random number generator
			PseudoRandomArray<PseudoRandom<ranlib::NormalUnit<double>, double>, double> random_container(gbm.factors(), gbm.number_of_steps());
			//  convert random variates to their antithetics (instantiated from template)
			std::function<void(Array<double, 2>&, Array<double, 2>&)> antithetic = normal_antithetic_reference < Array<double, 2> >;
			// instantiate MCGatherer objects to collect simulation results
			MCGatherer<double> mcgatherer_antithetic;
			// instantiate MCMapping and bind to functor
			MCMapping<GeometricBrownianMotion, Array<double, 2> > mc_mapping(payoff, gbm, ts, numeraire_index);
			std::function<double(Array<double, 2>)> func = std::bind(&MCMapping<GeometricBrownianMotion, Array<double, 2> >::mapping, &mc_mapping, std::placeholders::_1);
			// instantiate generic Monte Carlo algorithm object
			MCGeneric<Array<double, 2>&, double, PseudoRandomArray<PseudoRandom<ranlib::NormalUnit<double>, double>, double> > mc_antithetic(func, random_container, antithetic, max_sim);
			// run Monte Carlo paths for antithetic
			mc_antithetic.simulate(mcgatherer_antithetic, sim);
			return mcgatherer_antithetic.mean();
		} // end of try block
		catch (std::logic_error& xcpt)
		{
			LOG(ERROR) << xcpt.what() << endl;
			throw xcpt;
		}
		catch (std::runtime_error xcpt)
		{
			LOG(ERROR) << xcpt.what() << endl;
			throw xcpt;
		}
		catch (...)
		{
			LOG(ERROR) << "Other exception caught" << endl;
			throw std::exception("unknown error in Antithetic MC valuation");
		}
	}

	double QRMC(std::vector<std::shared_ptr<BlackScholesAssetAdapter> > & assets, MCPayoff& payoff, \
		std::shared_ptr<TermStructure> term, double tenor, int numeraire_index, size_t sim, size_t N, \
		double ci, size_t max_sim)
	{
		try
		{
			Array<double, 1> T(N + 1);
			firstIndex idx;
			double dt = tenor / N;
			T = idx*dt;
			TermStructure& ts = *term;

			// instantiate stochastic process
			GeometricBrownianMotion gbm(assets);
			gbm.set_numeraire(numeraire_index);
			gbm.set_timeline(T);

			// quasi-random number generator
			std::shared_ptr<SobolArrayNormal> sobol = std::make_shared<SobolArrayNormal>(2, N, sim);
			RandomWrapper<SobolArrayNormal, Array<double, 2> >  randomWrapper(sobol);

			// MCMapping to map random numbers to asset price realisations to discounted payoffs
			MCMapping<GeometricBrownianMotion, Array<double, 2> > mc_mapping(payoff, gbm, ts, numeraire_index);
			// mapping functor
			std::function<double(Array<double, 2>)> func = std::bind(&MCMapping<GeometricBrownianMotion, Array<double, 2> >::mapping, &mc_mapping, std::placeholders::_1);
			// generic Monte Carlo algorithm object
			MCGeneric<Array<double, 2>&, double, RandomWrapper<SobolArrayNormal, Array<double, 2> > > mc_QR(func, randomWrapper, max_sim);
			MCGatherer<double> mcgathererQR;
			/// simulate
			mc_QR.simulate(mcgathererQR, sim);
			return mcgathererQR.mean();
		} // end of try block
		catch (std::logic_error& xcpt)
		{
			LOG(ERROR) << xcpt.what() << endl;
			throw xcpt;
		}
		catch (std::runtime_error xcpt)
		{
			LOG(ERROR) << xcpt.what() << endl;
			throw xcpt;
		}
		catch (...)
		{
			LOG(ERROR) << "Other exception caught" << endl;
			throw std::exception("unknown error in QR MC valuation");
		}
	}
}