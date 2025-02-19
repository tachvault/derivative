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

#include "Payoff.hpp"
#include "FiniteDifference.hpp"
#include "Binomial.hpp"
#include "MCPayoff.hpp"
#include "MCAmerican.hpp"
#include "MCGeneric.hpp"
#include "MExotics.hpp"
#include "QFRandom.hpp"
#include "GeometricBrownianMotion.hpp"

#include "EquityAssetPricer.hpp"
#include "MCAssetPricer.hpp"
#include "PseudoRandomArray.hpp"

namespace derivative
{
	namespace VanillaOptionPricer
	{
		double ValueAmericanWithBinomial(const std::shared_ptr<BlackScholesAssetAdapter>& stock, double rate, \
			dd::date maturity, double strike, VanillaOptionType optType, int N)
		{
			try
			{
				double mat = (double((maturity - dd::day_clock::local_day()).days())) / 365;
				BinomialLattice btree(stock, rate, mat, N);
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

		double ValueEuropeanWithBinomial(const std::shared_ptr<BlackScholesAssetAdapter>& stock, double rate, \
			dd::date maturity, double strike, VanillaOptionType optType, int N)
		{
			try
			{
				double mat = (double((maturity - dd::day_clock::local_day()).days())) / 365;
				BinomialLattice btree(stock, rate, mat, N);
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

		double ValueEuropeanWithMC(const std::shared_ptr<BlackScholesAssetAdapter>& stock, \
			std::shared_ptr<TermStructure> term, dd::date maturity, double strike, VanillaOptionType optType, int sim, \
			size_t N, double ci)
		{
			auto dayCount = (maturity - dd::day_clock::local_day()).days();
			double mat = (double(dayCount)) / 365;
			// instantiate MCPayoff object
			MCEuropeanVanilla option(0, mat, 0, strike, optType);
			std::vector<std::shared_ptr<BlackScholesAssetAdapter> > assets;
			assets.push_back(stock);
			//return AntitheticMC(assets, option, term, mat, -1, sim, N, ci, 100000);
			return QRMC(assets, option, term, mat, -1, sim, N, ci, 100000);
		}

		double ValueAmericanWithMC(const std::shared_ptr<BlackScholesAssetAdapter>& stock, \
			std::shared_ptr<TermStructure> term, dd::date maturity, double strike, VanillaOptionType optType, \
			size_t sim, size_t N, size_t train, int degree, double ci)
		{
			try
			{
				auto dayCount = (maturity - dd::day_clock::local_day()).days();
				double mat = (double(dayCount)) / 365;
				N = (dayCount > N) ? N : dayCount;
				std::vector<std::shared_ptr<BlackScholesAssetAdapter> > underlying;
				underlying.push_back(stock);
				TermStructure& ts = *term;

				int numeraire_index = -1;
				Array<double, 1> T(N + 1);
				firstIndex idx;
				double dt = (double)(long long((mat / N) * std::pow(10, 15))) / std::pow(10, 15);
				T = idx*dt;
				GeometricBrownianMotion gbm(underlying);
				gbm.set_timeline(T);

				/// instantiate random number generator
				PseudoRandomArray<PseudoRandom<ranlib::NormalUnit<double>, double>, double> random_container(gbm.factors(), gbm.number_of_steps());
				
				MCTrainingPaths<GeometricBrownianMotion, PseudoRandomArray<PseudoRandom<ranlib::NormalUnit<double>, double>, double> >
					training_paths(gbm, T, train, random_container, ts, numeraire_index);
				Payoff option(strike, optType);
				std::function<double(double)> f;
				f = std::bind(&Payoff::operator(), &option, std::placeholders::_1);
				std::function<double(double, const Array<double, 1>&)> payoff = std::bind(LSArrayAdapter, std::placeholders::_1, std::placeholders::_2, f, 0);
				std::vector<std::function<double(double, const Array<double, 1>&)> > basisfunctions;
				Array<int, 1> p(1);
				for (int i = 0; i <= degree; i++)
				{
					p(0) = i;
					add_polynomial_basis_function(basisfunctions, p);
				}

				LongstaffSchwartzExerciseBoundary boundary(T, training_paths.state_variables(), training_paths.numeraires(), payoff, basisfunctions);
				LSExerciseStrategy<LongstaffSchwartzExerciseBoundary> exercise_strategy(boundary);
				MCMapping<GeometricBrownianMotion, Array<double, 2> > mc_mapping(exercise_strategy, gbm, ts, numeraire_index);
				std::function<double(Array<double, 2>)> func = std::bind(&MCMapping<GeometricBrownianMotion, Array<double, 2> >::mapping, &mc_mapping, std::placeholders::_1);
				MCGeneric<Array<double, 2>&, double, PseudoRandomArray<PseudoRandom<ranlib::NormalUnit<double>, double>, double> > mc(func, random_container, 25000);
				MCGatherer<double> mcgatherer;
				boost::math::normal normal;
				double d = boost::math::quantile(normal, ci);
				mc.simulate(mcgatherer, sim);
				return mcgatherer.mean();
			} // end of try block
			catch (std::logic_error xcpt)
			{
				std::cerr << xcpt.what() << endl;
			}
			catch (std::runtime_error xcpt)
			{
				std::cerr << xcpt.what() << endl;
			}
			catch (...)
			{
				std::cerr << "Other exception caught" << endl;
			}
		}
	}

	namespace BarrierOptionPricer
	{
		double ValueWithMC(const std::shared_ptr<BlackScholesAssetAdapter>& stock, std::shared_ptr<TermStructure> term, \
			int btype, dd::date maturity, double strike, double barrier, int optType, size_t sim, size_t N, double ci)
		{

			BarrierOptionTypeEnum barrierType = static_cast<BarrierOptionTypeEnum>(btype);
			auto dayCount = (maturity - dd::day_clock::local_day()).days();
			double mat = (double(dayCount)) / 365;
			N = (dayCount > N) ? N : dayCount;
			TermStructure& ts = *term;

			// set up timeline
			Array<double, 1> T(N + 1);
			firstIndex idx;
			double dt = (double)(long long((mat / N) * std::pow(10, 15))) / std::pow(10, 15);
			T = idx*dt;

			std::shared_ptr<MCPayoff> barrierOption;
			if (barrierType == KDI)
			{
				barrierOption = std::make_shared<BarrierIn>(T, 0, BarrierIn::DOWN, strike, barrier, optType);

			}
			else if (barrierType == KUI)
			{
				barrierOption = std::make_shared<BarrierIn>(T, 0, BarrierIn::UP, strike, barrier, optType);

			}
			else if (barrierType == KDO)
			{
				barrierOption = std::make_shared<BarrierOut>(T, 0, BarrierOut::DOWN, strike, barrier, optType);

			}
			else if (barrierType == KUO)
			{
				barrierOption = std::make_shared<BarrierOut>(T, 0, BarrierOut::UP, strike, barrier, optType);
			}
			std::vector<std::shared_ptr<BlackScholesAssetAdapter> > assets;
			assets.push_back(stock);
			//return AntitheticMC(assets, *barrierOption, term, mat, -1, sim, N, ci, 50000);
			return QRMC(assets, *barrierOption, term, mat, -1, sim, N, ci, 25000);
		}
	}

	namespace AverageOptionPricer
	{
		double ValueWithMC(const std::shared_ptr<BlackScholesAssetAdapter>& stock, std::shared_ptr<TermStructure> term, \
			int atype, dd::date maturity, double strike, int optType, size_t sim, size_t N, double ci)
		{
			AverageOptionTypeEnum averageType = static_cast<AverageOptionTypeEnum>(atype);
			auto dayCount = (maturity - dd::day_clock::local_day()).days();
			double mat = (double(dayCount)) / 365;
			N = (dayCount > N) ? N : dayCount;
			// set up timeline
			Array<double, 1> T(N + 1);
			firstIndex idx;
			double dt = (double)(long long((mat / N) * std::pow(10, 15))) / std::pow(10, 15);
			T = idx*dt;
			std::shared_ptr<MCPayoff> avgpayoff;
			if (averageType == FIXED_STRIKE)
			{
				avgpayoff = std::make_shared<MCDiscreteArithmeticMeanFixedStrike>(0, T, strike, 1, stock->GetAssetValue()->GetTradePrice(), optType);
			}
			else if (averageType == FLOATING_STRIKE)
			{
				avgpayoff = std::make_shared<MCDiscreteArithmeticMeanFloatingStrike>(0, T, 1, stock->GetAssetValue()->GetTradePrice(), optType);
			}
			else
			{
				throw std::logic_error("Invalid option type");
			}
			std::vector<std::shared_ptr<BlackScholesAssetAdapter> > assets;
			assets.push_back(stock);
			//return AntitheticMC(assets, *avgpayoff, term, mat, -1, sim, N, ci, 25000);
			return QRMC(assets, *avgpayoff, term, mat, -1, sim, N, ci, 25000);
		}
	}

	namespace LookBackOptionPricer
	{
		double ValueWithMC(const std::shared_ptr<BlackScholesAssetAdapter>& stock, std::shared_ptr<TermStructure> term, \
			dd::date maturity, double strike, int lbType, int optType, size_t sim, size_t N, double ci)
		{
			LookBackOptionTypeEnum averageType = static_cast<LookBackOptionTypeEnum>(lbType);
			auto dayCount = (maturity - dd::day_clock::local_day()).days();
			double mat = (double(dayCount)) / 365;
			N = (dayCount > N) ? N : dayCount;
			// set up timeline
			Array<double, 1> T(N + 1);
			firstIndex idx;
			double dt = (double)(long long((mat / N) * std::pow(10, 15))) / std::pow(10, 15);
			T = idx*dt;
			std::shared_ptr<MCPayoff> lbpayoff;
			if (lbType == FIXED_STRIKE)
			{
				lbpayoff = std::make_shared<MCDiscreteFixedLookBack>(0, stock->GetAssetValue()->GetTradePrice(), T, strike, optType);
			}
			else if (averageType == FLOATING_STRIKE)
			{
				lbpayoff = std::make_shared<MCDiscreteFloatingLookBack>(0, stock->GetAssetValue()->GetTradePrice(), T, optType);
			}
			else
			{
				throw std::logic_error("Invalid option type");
			}
			std::vector<std::shared_ptr<BlackScholesAssetAdapter> > assets;
			assets.push_back(stock);
			//return AntitheticMC(assets, *lbpayoff, term, mat, -1, sim, N, ci, 50000);
			return QRMC(assets, *lbpayoff, term, mat, -1, sim, N, ci, 25000);
		}
	}

	namespace MargrabeOptionPricer
	{
		double ValueEuropeanClosedForm(const std::shared_ptr<BlackScholesAssetAdapter>& stock1, const std::shared_ptr<BlackScholesAssetAdapter>& stock2, \
			std::shared_ptr<TermStructure> term, dd::date maturity, double strike, int optType)
		{
			double mat = (double((maturity - dd::day_clock::local_day()).days())) / 365;
			return stock1->Margrabe(std::const_pointer_cast<BlackScholesAssetAdapter>(stock2), mat, strike, optType);
		}

		double ValueEuropeanWithMC(const std::shared_ptr<BlackScholesAssetAdapter>& stock1, const std::shared_ptr<BlackScholesAssetAdapter>& stock2, \
			std::shared_ptr<TermStructure> term, dd::date maturity, double strike, int optType, size_t sim, size_t N, double ci)
		{
			auto dayCount = (maturity - dd::day_clock::local_day()).days();
			double mat = (double(dayCount)) / 365;
			exotics::Margrabe margrabe(stock1, stock2, 0.0, mat, 1.0, strike, *term);
			std::shared_ptr<MCPayoffList> payoffs = margrabe.get_payoff();
			std::vector<std::shared_ptr<BlackScholesAssetAdapter> > assets;
			assets.push_back(stock1);
			assets.push_back(stock2);
			return QRMC(assets, *payoffs, term, mat, -1, sim, N, ci, 100000);
			// return AntitheticMC(assets, *payoffs, term, mat, 1, sim, N, ci, 50000);
			//return GeneralMC(assets, *payoffs, term, mat, -1, sim, N, ci, 50000);
		}

		double ValueAmericanWithMC(const std::shared_ptr<BlackScholesAssetAdapter>& stock1, const std::shared_ptr<BlackScholesAssetAdapter>& stock2, \
			std::shared_ptr<TermStructure> term, dd::date maturity, double strike, int optType, size_t sim, size_t N, size_t train, int degree, double ci)
		{
			try
			{
				auto dayCount = (maturity - dd::day_clock::local_day()).days();
				double mat = (double(dayCount)) / 365;
				N = (dayCount > N) ? N : dayCount;
				int numeraire_index = -1;
				Array<double, 1> T(N + 1);
				firstIndex idx;
				double dt = (double)(long long((mat / N) * std::pow(10, 15))) / std::pow(10, 15);
				T = idx*dt;

				TermStructure& ts = *term;
				std::vector<std::shared_ptr<BlackScholesAssetAdapter> > underlying;
				underlying.push_back(stock1);
				underlying.push_back(stock2);

				exotics::Margrabe Mopt(stock1, stock2, T(0), mat, 1.0, strike, *term);
				/// American option by Monte Carlo
				GeometricBrownianMotion gbm(underlying);
				gbm.set_timeline(T);

				/// instantiate random number generator
				PseudoRandomArray<PseudoRandom<ranlib::NormalUnit<double>, double>, double> random_container(gbm.factors(), gbm.number_of_steps());
				MCTrainingPaths<GeometricBrownianMotion, PseudoRandomArray<PseudoRandom<ranlib::NormalUnit<double>, double>, double> >
					training_paths(gbm, T, train, random_container, ts, numeraire_index);
				cout << "Training paths created." << endl;
				// payoff requires (time points) x (state variables) Array as second argument
				std::function<double(const Array<double, 1>&, const Array<double, 2>&)> payoff = std::bind(&exotics::Margrabe::early_exercise_payoff, &Mopt, std::placeholders::_1, std::placeholders::_2);
				std::vector<std::function<double(const Array<double, 1>&, const Array<double, 2>&)> > basisfunctions;
				Array<int, 1> p(2);
				p(1) = 0.0;
				for (int i = 0; i <= degree; i++)
				{
					p(0) = i;
					add_polynomial_basis_function(basisfunctions, p);
				}
				p(0) = 0.0;
				for (int i = 1; i <= degree; i++)
				{
					p(1) = i;
					add_polynomial_basis_function(basisfunctions, p);
				}

				// training_paths is currently a paths x (time points) x (state variables) Array
				RegressionExerciseBoundary boundary(T, training_paths.state_variables(), training_paths.numeraires(), payoff, basisfunctions);
				LSExerciseStrategy<RegressionExerciseBoundary> exercise_strategy(boundary);
				MCMapping<GeometricBrownianMotion, Array<double, 2> > mc_mapping(exercise_strategy, gbm, ts, numeraire_index);
				std::function<double(Array<double, 2>)> func = std::bind(&MCMapping<GeometricBrownianMotion, Array<double, 2> >::mapping, &mc_mapping, std::placeholders::_1);
				MCGeneric<Array<double, 2>&, double, PseudoRandomArray<PseudoRandom<ranlib::NormalUnit<double>, double>, double> > mc(func, random_container, 5000);
				MCGatherer<double> mcgatherer;
				boost::math::normal normal;
				double d = boost::math::quantile(normal, ci);
				mc.simulate(mcgatherer, sim);
				return mcgatherer.mean();
			} // end of try block

			catch (std::logic_error xcpt)
			{
				std::cerr << xcpt.what() << endl;
			}
			catch (std::runtime_error xcpt)
			{
				std::cerr << xcpt.what() << endl;
			}
			catch (...)
			{
				std::cerr << "Other exception caught" << endl;
			}
		}
	}

	namespace ChooserOptionPricer
	{
		double ValueWithMC(const std::shared_ptr<BlackScholesAssetAdapter>& stock, std::shared_ptr<TermStructure> term, \
			dd::date maturity, double strike, size_t sim, size_t N, double ci)
		{
			double mat = (double((maturity - dd::day_clock::local_day()).days())) / 365;
			std::shared_ptr<MCPayoff> payoff = std::make_shared<MCChooser>(0, mat, 0, strike);
			std::vector<std::shared_ptr<BlackScholesAssetAdapter> > assets;
			assets.push_back(stock);
			//return AntitheticMC(assets, *payoff, term, mat, -1, sim, N, ci, 50000);
			return QRMC(assets, *payoff, term, mat, -1, sim, N, ci, 25000);
		}
	}
}