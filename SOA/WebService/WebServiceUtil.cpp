/*
Copyright (C) Nathan Muruganantha 2013 - 2014
*/

#include <memory>

#include "WebServiceUtil.hpp"
#include "EquityVanillaOptMessage.hpp"
#include "FuturesVanillaOptMessage.hpp"
#include "EquityBarrierOptMessage.hpp"
#include "EquityAverageOptMessage.hpp"
#include "EquityChooserOptMessage.hpp"
#include "EquityLookBackOptMessage.hpp"
#include "EquityMargrabeOptMessage.hpp"
#include "EquityOptionSpreadMessage.hpp"
#include "FuturesOptionSpreadMessage.hpp"
#include "ExchangeRateVanillaOptMessage.hpp"
#include "ExchangeRateOptionSpreadMessage.hpp"
#include "ESBManager.hpp"
#include "QFUtil.hpp"

namespace derivative
{
	namespace WebServiceUtil
	{
		template <typename T1, typename T2>
		web::json::value SendError(const std::shared_ptr<T1>& msg, const T2& req, const std::string& error)
		{
			/// update the message with the error condition
			IMessage::SystemResponse sysRes;
			sysRes.outcome = IMessage::RequestError;
			sysRes.outText = error;
			msg->SetSystemResponse(sysRes);

			/// construct JSON string from message
			web::json::value jsonMsg = msg->AsJSON();
			return jsonMsg;
		}

		template <typename T1, typename T2>
		web::json::value ProcessMsg(const std::shared_ptr<T1>& msg, const T2& req)
		{
			/// now set the request
			msg->SetRequest(req);

			/// Vanilla ESB to passs the request to FuturesVanillaOption processor
			ESBManager& esb = ESBManager::getInstance();
			esb.HandleRequest(msg);

			/// construct JSON string from message
			web::json::value jsonMsg = msg->AsJSON();
			return jsonMsg;
		}

		void decodeOption(const std::string& opt, EquityOptionSpreadMessage::Leg& leg)
		{
			std::vector<std::string> vec;
			splitLine(opt, vec, ',');
			for (auto &value : vec)
			{
				std::vector<std::string> p;
				splitLine(value, p, '=');
				if (p[0].compare("_option") == 0)
				{
					if (p[1].compare("call") == 0)
					{
						leg.option = EquityOptionSpreadMessage::CALL;
					}
					else if (p[1].compare("put") == 0)
					{
						leg.option = EquityOptionSpreadMessage::PUT;
					}
					else
					{
						throw std::invalid_argument("Invalid option type");
					}
				}
				if (p[0].compare("_position") == 0)
				{
					if (p[1].compare("long") == 0)
					{
						leg.pos = EquityOptionSpreadMessage::LONG;
					}
					else if (p[1].compare("short") == 0)
					{
						leg.pos = EquityOptionSpreadMessage::SHORT;
					}
					else
					{
						throw std::invalid_argument("Invalid long/short position");
					}
				}
				else if (p[0].compare("_strike") == 0)
				{
					leg.strike = atof(p[1].c_str());
				}
				else if (p[0].compare("_maturity") == 0)
				{
					leg.maturity = dd::from_string(p[1]);
				}
				else if (p[0].compare("_units") == 0)
				{
					leg.units = atoi(p[1].c_str());
				}
			}
		}

		void decodeLegs(const std::string& line, std::vector<EquityOptionSpreadMessage::Leg>& legs)
		{
			auto it = line.cbegin();
			bool start = false;
			for (; it != line.cend(); ++it)
			{
				if (*it == '(')
				{
					start = true;
					auto beg = it;
					for (; it != line.cend(); ++it)
					{
						if (*it == ')')
						{
							start = false;
							EquityOptionSpreadMessage::Leg leg;
							std::string opt(beg + 1, it);
							decodeOption(opt, leg);
							if (leg.validate() == false) throw std::invalid_argument("Invalid option data");
							legs.push_back(leg);
							break;
						}
					}
				}
			}
			if (start || legs.empty()) throw std::invalid_argument("Invalid option data data");
		}

		web::json::value HandleEquityOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings)
		{
			if (!paths[1].empty() && paths[1].compare(U("Vanilla")) == 0)
			{
				return HandleEquityVanillaOption(paths, query_strings);
			}
			else if (!paths[1].empty() && paths[1].compare(U("Barrier")) == 0)
			{
				return HandleEquityBarrierOption(paths, query_strings);
			}
			else if (!paths[1].empty() && paths[1].compare(U("Average")) == 0)
			{
				return HandleEquityVanillaOption(paths, query_strings);
			}
			else if (!paths[1].empty() && paths[1].compare(U("Barrier")) == 0)
			{
				return HandleEquityVanillaOption(paths, query_strings);
			}
			else if (!paths[1].empty() && paths[1].compare(U("Barrier")) == 0)
			{
				return HandleEquityVanillaOption(paths, query_strings);
			}
		}

		web::json::value HandleEquityVanillaOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings)
		{
			/// now add request parameters
			std::shared_ptr<EquityVanillaOptMessage> msg = std::make_shared<EquityVanillaOptMessage>();
			EquityVanillaOptMessage::Request req;
			try
			{	
				msg->ParseSymbol(req, query_strings);
				msg->ParseMaturity(req, query_strings);
				msg->ParseStrike(req, query_strings);
				msg->ParseVol(req, query_strings);
				req.option = msg->ParseOptionType(query_strings);
				if (req.option == VanillaOptMessage::TYPE_UNKNOWN)
				{
					req.option = VanillaOptMessage::CALL;
				};

				req.style = msg->ParseOptionStyle(query_strings);
				if (req.option == VanillaOptMessage::STYLE_UNKNOWN)
				{
					req.style == VanillaOptMessage::EUROPEAN;
				}

				req.method = msg->ParsePricingMethod(query_strings);
				if (req.method == VanillaOptMessage::METHOD_UNKNOWN)
				{
					req.method = VanillaOptMessage::CLOSED;
				}
				req.rateType = msg->ParseRateType(query_strings);
				req.volType = msg->ParseVolType(query_strings);
			}
			catch (std::exception& e)
			{
				return SendError<EquityVanillaOptMessage, VanillaOptMessage::Request>(msg, req, e.what());
			}

			return ProcessMsg<EquityVanillaOptMessage, VanillaOptMessage::Request>(msg, req);
		}
		
		web::json::value HandleEquityBarrierOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings)
		{
			/// now add request parameters
			std::shared_ptr<EquityBarrierOptMessage> msg = std::make_shared<EquityBarrierOptMessage>();
			EquityBarrierOptMessage::BarrierRequest req;
			try
			{
				msg->ParseSymbol(req, query_strings);
				msg->ParseMaturity(req, query_strings);
				msg->ParseStrike(req, query_strings);
				msg->ParseVol(req, query_strings);

				if (query_strings.find(U("_barrier")) != query_strings.end())
				{
					auto barrier = conversions::to_utf8string(query_strings.at(U("_barrier")));
					req.barrier = stod(barrier);
				}
				else
				{
					throw std::invalid_argument("No barrier value");
				}
				req.option = msg->ParseOptionType(query_strings);
				if (req.option == VanillaOptMessage::TYPE_UNKNOWN)
				{
					req.option = VanillaOptMessage::CALL;
				};
				req.style = msg->ParseOptionStyle(query_strings);
				if (req.option == VanillaOptMessage::STYLE_UNKNOWN)
				{
					req.style == VanillaOptMessage::EUROPEAN;
				}

				req.barrierType = msg->ParseBarrierType(query_strings);
				req.method = VanillaOptMessage::MONTE_CARLO;
				req.rateType = msg->ParseRateType(query_strings);
				req.volType = msg->ParseVolType(query_strings);
			}
			catch (std::exception& e)
			{
				return SendError<EquityBarrierOptMessage, EquityBarrierOptMessage::BarrierRequest>(msg, req, e.what());
			}

			return ProcessMsg<EquityBarrierOptMessage, EquityBarrierOptMessage::BarrierRequest>(msg, req);
		}

   	    web::json::value HandleEquityAverageOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings)
		{
			/// now add request parameters
			std::shared_ptr<EquityAverageOptMessage> msg = std::make_shared<EquityAverageOptMessage>();
			EquityAverageOptMessage::AverageOptRequest req;
			try
			{
				msg->ParseSymbol(req, query_strings);
				msg->ParseMaturity(req, query_strings);
				msg->ParseStrike(req, query_strings);
				msg->ParseVol(req, query_strings);
				req.option = msg->ParseOptionType(query_strings);
				if (req.option == VanillaOptMessage::TYPE_UNKNOWN)
				{
					req.option = VanillaOptMessage::CALL;
				};
				req.style = msg->ParseOptionStyle(query_strings);
				if (req.option == VanillaOptMessage::STYLE_UNKNOWN)
				{
					req.style == VanillaOptMessage::EUROPEAN;
				}

				req.averageType = msg->ParseAverageType(query_strings);
				req.method = VanillaOptMessage::MONTE_CARLO;
				req.rateType = msg->ParseRateType(query_strings);
				req.volType = msg->ParseVolType(query_strings);
			}
			catch (std::exception& e)
			{
				return SendError<EquityAverageOptMessage, EquityAverageOptMessage::AverageOptRequest>(msg, req, e.what());
			}

			return ProcessMsg<EquityAverageOptMessage, EquityAverageOptMessage::AverageOptRequest>(msg, req);
		}

		web::json::value HandleEquityLookBackOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings)
		{
			/// now add request parameters
			std::shared_ptr<EquityLookBackOptMessage> msg = std::make_shared<EquityLookBackOptMessage>();
			EquityLookBackOptMessage::Request req;
			msg->ParseSymbol(req, query_strings);
			msg->ParseMaturity(req, query_strings);
			msg->ParseStrike(req, query_strings);
			msg->ParseVol(req, query_strings);
			try
			{
				msg->ParseSymbol(req, query_strings);
				msg->ParseMaturity(req, query_strings);
				msg->ParseStrike(req, query_strings);
				msg->ParseVol(req, query_strings);
				if (req.option == VanillaOptMessage::TYPE_UNKNOWN)
				{
					req.option = VanillaOptMessage::CALL;
				};
				req.style = msg->ParseOptionStyle(query_strings);
				if (req.option == VanillaOptMessage::STYLE_UNKNOWN)
				{
					req.style == VanillaOptMessage::EUROPEAN;
				}

				req.method = VanillaOptMessage::MONTE_CARLO;
				req.rateType = msg->ParseRateType(query_strings);
				req.volType = msg->ParseVolType(query_strings);
			}
			catch (std::exception& e)
			{
				return SendError<EquityLookBackOptMessage, EquityLookBackOptMessage::Request>(msg, req, e.what());
			}

			return ProcessMsg<EquityLookBackOptMessage, EquityLookBackOptMessage::Request>(msg, req);
		}

		web::json::value HandleEquityChooserOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings)
		{
			/// now add request parameters
			std::shared_ptr<EquityChooserOptMessage> msg = std::make_shared<EquityChooserOptMessage>();
			EquityChooserOptMessage::Request req;
			msg->ParseSymbol(req, query_strings);
			msg->ParseMaturity(req, query_strings);
			msg->ParseStrike(req, query_strings);
			msg->ParseVol(req, query_strings);
			try
			{
				msg->ParseSymbol(req, query_strings);
				msg->ParseMaturity(req, query_strings);
				msg->ParseStrike(req, query_strings);
				msg->ParseVol(req, query_strings); 
				if (req.option == VanillaOptMessage::TYPE_UNKNOWN)
				{
					req.option = VanillaOptMessage::CALL;
				};
				req.style = msg->ParseOptionStyle(query_strings);
				if (req.option == VanillaOptMessage::STYLE_UNKNOWN)
				{
					req.style == VanillaOptMessage::EUROPEAN;
				}

				req.method = VanillaOptMessage::MONTE_CARLO;
				req.rateType = msg->ParseRateType(query_strings);
				req.volType = msg->ParseVolType(query_strings);
			}
			catch (std::exception& e)
			{
				return SendError<EquityChooserOptMessage, EquityChooserOptMessage::Request>(msg, req, e.what());
			}

			return ProcessMsg<EquityChooserOptMessage, EquityChooserOptMessage::Request>(msg, req);
		}

		web::json::value HandleEquityMargrabeOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings)
		{
			/// now add request parameters
			std::shared_ptr<EquityMargrabeOptMessage> msg = std::make_shared<EquityMargrabeOptMessage>();
			EquityMargrabeOptMessage::MargrabeOptRequest req;
			try
			{
				msg->ParseSymbol(req, query_strings);
				msg->ParseMaturity(req, query_strings);
				msg->ParseStrike(req, query_strings);
				msg->ParseVol(req, query_strings);
				if (query_strings.find(U("_numeraire")) != query_strings.end())
				{
					req.exchanged = conversions::to_utf8string(query_strings.at(U("_numeraire")));
				}
				else
				{
					throw std::invalid_argument("No numeraire symbol");
				}
				if (req.option == VanillaOptMessage::TYPE_UNKNOWN)
				{
					req.option = VanillaOptMessage::CALL;
				};
				req.style = msg->ParseOptionStyle(query_strings);
				if (req.option == VanillaOptMessage::STYLE_UNKNOWN)
				{
					req.style == VanillaOptMessage::EUROPEAN;
				}

				req.method = VanillaOptMessage::MONTE_CARLO;
				req.rateType = msg->ParseRateType(query_strings);
				req.volType = msg->ParseVolType(query_strings);
			}
			catch (std::exception& e)
			{
				return SendError<EquityMargrabeOptMessage, EquityMargrabeOptMessage::Request>(msg, req, e.what());
			}

			return ProcessMsg<EquityMargrabeOptMessage, EquityMargrabeOptMessage::MargrabeOptRequest>(msg, req);
		}

		web::json::value HandleFuturesOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings)
		{
			if (!paths[1].empty() && paths[1].compare(U("Vanilla")) == 0)
			{
				/// now add request parameters
				FuturesVanillaOptMessage::Request req;
				if (query_strings.find(U("_option")) != query_strings.end())
				{
					if (query_strings.at(U("_option")).compare(U("call")) == 0)
					{
						req.option = FuturesVanillaOptMessage::CALL;
						return HandleFuturesVanillaOption(FuturesVanillaOptMessage::CALL, paths, query_strings);
					}
					else if (query_strings.at(U("_option")).compare(U("put")) == 0)
					{
						req.option = FuturesVanillaOptMessage::PUT;
						return HandleFuturesVanillaOption(FuturesVanillaOptMessage::PUT, paths, query_strings);
					}
					else
					{
						throw std::invalid_argument("Only plain vanilla options are supported");
					}
				}
				else
				{
					throw std::invalid_argument("No option parameter (call/put)");
				}
			}
			else
			{
				throw std::logic_error("Only Plain Vanilla options are supported");
			}
		}

		web::json::value HandleFuturesVanillaOption(FuturesVanillaOptMessage::OptionTypeEnum opt, const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings)
		{
			/// now add request parameters
			std::shared_ptr<FuturesVanillaOptMessage> msg = std::make_shared<FuturesVanillaOptMessage>();
			FuturesVanillaOptMessage::FuturesRequest req;
			req.option = opt;
			try
			{
				if (query_strings.find(U("_style")) != query_strings.end())
				{
					if (query_strings.at(U("_style")).compare(U("european")) == 0)
					{
						req.style = FuturesVanillaOptMessage::EUROPEAN;
					}
					else if (query_strings.at(U("_style")).compare(U("american")) == 0)
					{
						req.style = FuturesVanillaOptMessage::AMERICAN;
					}
					else
					{
						throw std::invalid_argument("Invalid Style parameter");
					}
				}
				else
				{
					req.style = FuturesVanillaOptMessage::AMERICAN;
				}

				if (query_strings.find(U("_method")) != query_strings.end())
				{
					if (query_strings.at(U("_method")).compare(U("closed")) == 0)
					{
						if (req.style == FuturesVanillaOptMessage::EUROPEAN)
						{
							req.method = FuturesVanillaOptMessage::CLOSED;
						}
						else
						{
							throw std::invalid_argument("American open cannot be evaluated with closed form");
						}
					}
					else if (query_strings.at(U("_method")).compare(U("lattice")) == 0)
					{
						req.method = FuturesVanillaOptMessage::LATTICE;
					}
					else
					{
						throw std::invalid_argument("Invalid pricing method parameter");
					}
				}
				else
				{
					req.method = FuturesVanillaOptMessage::LATTICE;
				}

				if (query_strings.find(U("_ratetype")) != query_strings.end())
				{
					if (query_strings.at(U("_ratetype")).compare(U("Yield")) == 0)
					{
						req.rateType = FuturesVanillaOptMessage::YIELD;
					}
					else if (query_strings.at(U("_ratetype")).compare(U("LIBOR")) == 0)
					{
						req.rateType = FuturesVanillaOptMessage::LIBOR;
					}
					else
					{
						throw std::invalid_argument("Invalid rate type parameter");
					}
				}
				else
				{
					req.rateType = FuturesVanillaOptMessage::YIELD;
				}

				if (query_strings.find(U("_symbol")) != query_strings.end())
				{
					req.underlying = conversions::to_utf8string(query_strings.at(U("_symbol")));
				}
				else
				{
					throw std::invalid_argument("No underlying symbol");
				}

				if (query_strings.find(U("_maturity")) != query_strings.end())
				{
					auto mat = conversions::to_utf8string(query_strings.at(U("_maturity")));
					req.maturity = dd::from_string(mat);
				}
				else
				{
					throw std::invalid_argument("No maturity date");
				}
				if (query_strings.find(U("_delivery")) != query_strings.end())
				{
					auto ddate = conversions::to_utf8string(query_strings.at(U("_delivery")));
					req.deliveryDate = dd::from_string(ddate);
				}
				else
				{
					throw std::invalid_argument("No delivery date");
				}

				if (query_strings.find(U("_strike")) != query_strings.end())
				{
					auto strike = conversions::to_utf8string(query_strings.at(U("_strike")));
					req.strike = stod(strike);
				}
				else
				{
					throw std::invalid_argument("No strike value");
				}

				if (query_strings.find(U("_vol")) != query_strings.end())
				{
					auto vol = conversions::to_utf8string(query_strings.at(U("_vol")));
					req.vol = stod(vol);
				}
			}
			catch (std::exception& e)
			{
				return SendError<FuturesVanillaOptMessage, FuturesVanillaOptMessage::FuturesRequest>(msg, req, e.what());
			}

			return ProcessMsg<FuturesVanillaOptMessage, FuturesVanillaOptMessage::FuturesRequest>(msg, req);
		}

		web::json::value HandleFXOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings)
		{
			if (!paths[1].empty() && paths[1].compare(U("Vanilla")) == 0)
			{
				/// now add request parameters
				ExchangeRateVanillaOptMessage::Request req;
				if (query_strings.find(U("_option")) != query_strings.end())
				{
					if (query_strings.at(U("_option")).compare(U("call")) == 0)
					{
						req.option = ExchangeRateVanillaOptMessage::CALL;
						return HandleFXVanillaOption(ExchangeRateVanillaOptMessage::CALL, paths, query_strings);
					}
					else if (query_strings.at(U("_option")).compare(U("put")) == 0)
					{
						req.option = ExchangeRateVanillaOptMessage::PUT;
						return HandleFXVanillaOption(ExchangeRateVanillaOptMessage::PUT, paths, query_strings);
					}
					else
					{
						throw std::invalid_argument("Only plain vanilla options are supported");
					}
				}
				else
				{
					throw std::invalid_argument("No option parameter (call/put)");
				}
			}
			else
			{
				throw std::logic_error("Only Plain Vanilla options are supported");
			}
		}

		web::json::value HandleFXVanillaOption(ExchangeRateVanillaOptMessage::OptionTypeEnum opt, const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings)
		{
			/// now add request parameters
			std::shared_ptr<ExchangeRateVanillaOptMessage> msg = std::make_shared<ExchangeRateVanillaOptMessage>();
			ExchangeRateVanillaOptMessage::ExchangeRateRequest req;
			req.option = opt;
			try
			{
				if (query_strings.find(U("_style")) != query_strings.end())
				{
					if (query_strings.at(U("_style")).compare(U("european")) == 0)
					{
						req.style = EquityVanillaOptMessage::EUROPEAN;
					}
					else if (query_strings.at(U("_style")).compare(U("american")) == 0)
					{
						req.style = EquityVanillaOptMessage::AMERICAN;
					}
					else
					{
						throw std::invalid_argument("Invalid Style parameter");
					}
				}
				else
				{
					req.style = EquityVanillaOptMessage::AMERICAN;
				}

				if (query_strings.find(U("_method")) != query_strings.end())
				{
					if (query_strings.at(U("_method")).compare(U("closed")) == 0)
					{
						if (req.style == EquityVanillaOptMessage::EUROPEAN)
						{
							req.method = EquityVanillaOptMessage::CLOSED;
						}
						else
						{
							throw std::invalid_argument("American open cannot be evaluated with closed form");
						}
					}
					else if (query_strings.at(U("_method")).compare(U("lattice")) == 0)
					{
						req.method = EquityVanillaOptMessage::LATTICE;
					}
					else
					{
						throw std::invalid_argument("Invalid pricing method parameter");
					}
				}
				else
				{
					req.method = EquityVanillaOptMessage::LATTICE;
				}

				if (query_strings.find(U("_ratetype")) != query_strings.end())
				{
					if (query_strings.at(U("_ratetype")).compare(U("Yield")) == 0)
					{
						req.rateType = EquityVanillaOptMessage::YIELD;
					}
					else if (query_strings.at(U("_ratetype")).compare(U("LIBOR")) == 0)
					{
						req.rateType = EquityVanillaOptMessage::LIBOR;
					}
					else
					{
						throw std::invalid_argument("Invalid rate type parameter");
					}
				}
				else
				{
					req.rateType = EquityVanillaOptMessage::YIELD;
				}

				if (query_strings.find(U("_domestic")) != query_strings.end())
				{
					req.domestic = conversions::to_utf8string(query_strings.at(U("_domestic")));
				}
				else
				{
					throw std::invalid_argument("No domestic currency symbol");
				}

				if (query_strings.find(U("_foreign")) != query_strings.end())
				{
					req.foreign = conversions::to_utf8string(query_strings.at(U("_foreign")));
				}
				else
				{
					throw std::invalid_argument("No foreign currency symbol");
				}

				if (query_strings.find(U("_maturity")) != query_strings.end())
				{
					auto mat = conversions::to_utf8string(query_strings.at(U("_maturity")));
					req.maturity = dd::from_string(mat);
				}
				else
				{
					throw std::invalid_argument("No maturity date");
				}

				if (query_strings.find(U("_strike")) != query_strings.end())
				{
					auto strike = conversions::to_utf8string(query_strings.at(U("_strike")));
					req.strike = stod(strike);
				}
				else
				{
					throw std::invalid_argument("No strike value");
				}

				if (query_strings.find(U("_voltype")) != query_strings.end())
				{
					if (query_strings.at(U("_voltype")).compare(U("HV")) == 0)
					{
						req.volType = EquityVanillaOptMessage::HV;
					}
					else
					{
						throw std::invalid_argument("Invalid volatility type parameter");
					}
				}
				else
				{
					req.volType = EquityVanillaOptMessage::HV;
				}

				if (query_strings.find(U("_vol")) != query_strings.end())
				{
					auto vol = conversions::to_utf8string(query_strings.at(U("_vol")));
					req.vol = stod(vol);
				}
			}
			catch (std::exception& e)
			{
				return SendError<ExchangeRateVanillaOptMessage, VanillaOptMessage::Request>(msg, req, e.what());
			}

			return ProcessMsg<ExchangeRateVanillaOptMessage, ExchangeRateVanillaOptMessage::ExchangeRateRequest>(msg, req);
		}

		web::json::value HandleEquityOptionSpread(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings)
		{
			/// now add request parameters
			std::shared_ptr<EquityOptionSpreadMessage> msg = std::make_shared<EquityOptionSpreadMessage>();
			EquityOptionSpreadMessage::Request req;
			try
			{
				if (query_strings.find(U("_style")) != query_strings.end())
				{
					if (query_strings.at(U("_style")).compare(U("european")) == 0)
					{
						req.style = EquityOptionSpreadMessage::EUROPEAN;
					}
					else if (query_strings.at(U("_style")).compare(U("american")) == 0)
					{
						req.style = EquityOptionSpreadMessage::AMERICAN;
					}
					else
					{
						throw std::invalid_argument("Invalid Style parameter");
					}
				}
				else
				{
					req.style = EquityOptionSpreadMessage::AMERICAN;
				}

				if (query_strings.find(U("_method")) != query_strings.end())
				{
					if (query_strings.at(U("_method")).compare(U("closed")) == 0)
					{
						if (req.style == EquityOptionSpreadMessage::EUROPEAN)
						{
							req.method = EquityOptionSpreadMessage::CLOSED;
						}
						else
						{
							throw std::invalid_argument("American open cannot be evaluated with closed form");
						}
					}
					else if (query_strings.at(U("_method")).compare(U("lattice")) == 0)
					{
						req.method = EquityOptionSpreadMessage::LATTICE;
					}
					else if (query_strings.at(U("_method")).compare(U("montecarlo")) == 0)
					{
						req.method = EquityOptionSpreadMessage::MONTE_CARLO;
					}
					else
					{
						throw std::invalid_argument("Invalid pricing method parameter");
					}
				}
				else
				{
					req.method = EquityOptionSpreadMessage::LATTICE;
				}
				if (query_strings.find(U("_ratetype")) != query_strings.end())
				{
					if (query_strings.at(U("_ratetype")).compare(U("Yield")) == 0)
					{
						req.rateType = EquityOptionSpreadMessage::YIELD;
					}
					else if (query_strings.at(U("_ratetype")).compare(U("LIBOR")) == 0)
					{
						req.rateType = EquityOptionSpreadMessage::LIBOR;
					}
					else
					{
						throw std::invalid_argument("Invalid Rate type parameter");
					}
				}
				else
				{
					req.rateType = EquityOptionSpreadMessage::YIELD;
				}

				if (query_strings.find(U("_voltype")) != query_strings.end())
				{
					if (query_strings.at(U("_voltype")).compare(U("IV")) == 0)
					{
						req.volType = EquityOptionSpreadMessage::IV;
					}
					else if (query_strings.at(U("_voltype")).compare(U("HV")) == 0)
					{
						req.volType = EquityOptionSpreadMessage::HV;
					}
					else
					{
						throw std::invalid_argument("Invalid Volatility type parameter");
					}
				}
				if (query_strings.find(U("_symbol")) != query_strings.end())
				{
					req.underlying = conversions::to_utf8string(query_strings.at(U("_symbol")));
				}
				else
				{
					throw std::invalid_argument("No underlying symbol");
				}

				if (query_strings.find(U("_vol")) != query_strings.end())
				{
					auto vol = conversions::to_utf8string(query_strings.at(U("_vol")));
					req.vol = stod(vol);
				}

				if (query_strings.find(U("_legs")) != query_strings.end())
				{
					std::string legs = conversions::to_utf8string(query_strings.at(U("_legs")));
					decodeLegs(legs, req.legs);
				}

				if (query_strings.find(U("_naked")) != query_strings.end())
				{
					if (query_strings.at(U("_naked")).compare(U("long")) == 0)
					{
						req.equityPos.pos = EquityOptionSpreadMessage::LONG;
						if (query_strings.find(U("_units")) != query_strings.end())
						{
							req.equityPos.units = atoi(conversions::to_utf8string(query_strings.at(U("_units"))).c_str());
						}
					}
					else if (query_strings.at(U("_naked")).compare(U("short")) == 0)
					{
						req.equityPos.pos = EquityOptionSpreadMessage::SHORT;
						if (query_strings.find(U("_units")) != query_strings.end())
						{
							req.equityPos.units = atoi(conversions::to_utf8string(query_strings.at(U("_units"))).c_str());
						}
					}
					else
					{
						throw std::invalid_argument("Invalid Style parameter");
					}
				}
			}
			catch (std::exception& e)
			{
				return SendError<EquityOptionSpreadMessage, EquityOptionSpreadMessage::Request>(msg, req, e.what());
			}

			return ProcessMsg<EquityOptionSpreadMessage, EquityOptionSpreadMessage::Request>(msg, req);
		}

		web::json::value HandleFuturesOptionSpread(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings)
		{
			/// now add request parameters
			std::shared_ptr<FuturesOptionSpreadMessage> msg = std::make_shared<FuturesOptionSpreadMessage>();
			FuturesOptionSpreadMessage::FuturesRequest req;
			try
			{
				if (query_strings.find(U("_style")) != query_strings.end())
				{
					if (query_strings.at(U("_style")).compare(U("european")) == 0)
					{
						req.style = FuturesOptionSpreadMessage::EUROPEAN;
					}
					else if (query_strings.at(U("_style")).compare(U("american")) == 0)
					{
						req.style = FuturesOptionSpreadMessage::AMERICAN;
					}
					else
					{
						throw std::invalid_argument("Invalid Style parameter");
					}
				}
				else
				{
					req.style = FuturesOptionSpreadMessage::AMERICAN;
				}

				if (query_strings.find(U("_method")) != query_strings.end())
				{
					if (query_strings.at(U("_method")).compare(U("closed")) == 0)
					{
						if (req.style == FuturesOptionSpreadMessage::EUROPEAN)
						{
							req.method = FuturesOptionSpreadMessage::CLOSED;
						}
						else
						{
							throw std::invalid_argument("American open cannot be evaluated with closed form");
						}
					}
					else if (query_strings.at(U("_method")).compare(U("lattice")) == 0)
					{
						req.method = FuturesOptionSpreadMessage::LATTICE;
					}
					else
					{
						throw std::invalid_argument("Invalid pricing method parameter");
					}
				}
				else
				{
					req.method = FuturesOptionSpreadMessage::LATTICE;
				}
				if (query_strings.find(U("_ratetype")) != query_strings.end())
				{
					if (query_strings.at(U("_ratetype")).compare(U("Yield")) == 0)
					{
						req.rateType = FuturesOptionSpreadMessage::YIELD;
					}
					else if (query_strings.at(U("_ratetype")).compare(U("LIBOR")) == 0)
					{
						req.rateType = FuturesOptionSpreadMessage::LIBOR;
					}
					else
					{
						throw std::invalid_argument("Invalid Style parameter");
					}
				}
				else
				{
					req.rateType = FuturesOptionSpreadMessage::YIELD;
				}
				if (query_strings.find(U("_symbol")) != query_strings.end())
				{
					req.underlying = conversions::to_utf8string(query_strings.at(U("_symbol")));
				}
				else
				{
					throw std::invalid_argument("No underlying symbol");
				}

				if (query_strings.find(U("_delivery")) != query_strings.end())
				{
					auto ddate = conversions::to_utf8string(query_strings.at(U("_delivery")));
					req.deliveryDate = dd::from_string(ddate);
				}
				else
				{
					throw std::invalid_argument("No delivery date");
				}

				if (query_strings.find(U("_vol")) != query_strings.end())
				{
					auto vol = conversions::to_utf8string(query_strings.at(U("_vol")));
					req.vol = stod(vol);
				}

				if (query_strings.find(U("_legs")) != query_strings.end())
				{
					std::string legs = conversions::to_utf8string(query_strings.at(U("_legs")));
					decodeLegs(legs, req.legs);
				}

				if (query_strings.find(U("_naked")) != query_strings.end())
				{
					if (query_strings.at(U("_naked")).compare(U("long")) == 0)
					{
						req.equityPos.pos = FuturesOptionSpreadMessage::LONG;
						if (query_strings.find(U("_units")) != query_strings.end())
						{
							req.equityPos.units = atoi(conversions::to_utf8string(query_strings.at(U("_units"))).c_str());
						}
					}
					else if (query_strings.at(U("_naked")).compare(U("short")) == 0)
					{
						req.equityPos.pos = FuturesOptionSpreadMessage::SHORT;
						if (query_strings.find(U("_units")) != query_strings.end())
						{
							req.equityPos.units = atoi(conversions::to_utf8string(query_strings.at(U("_units"))).c_str());
						}
					}
					else
					{
						throw std::invalid_argument("Invalid Style parameter");
					}
				}
			}
			catch (std::exception& e)
			{
				return SendError<FuturesOptionSpreadMessage, FuturesOptionSpreadMessage::FuturesRequest>(msg, req, e.what());
			}

			return ProcessMsg<FuturesOptionSpreadMessage, FuturesOptionSpreadMessage::FuturesRequest>(msg, req);
		}

		web::json::value HandleFXOptionSpread(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings)
		{
			/// now add request parameters
			std::shared_ptr<ExchangeRateOptionSpreadMessage> msg = std::make_shared<ExchangeRateOptionSpreadMessage>();
			ExchangeRateOptionSpreadMessage::ExchangeRateRequest req;
			try
			{
				if (query_strings.find(U("_style")) != query_strings.end())
				{
					if (query_strings.at(U("_style")).compare(U("european")) == 0)
					{
						req.style = ExchangeRateOptionSpreadMessage::EUROPEAN;
					}
					else if (query_strings.at(U("_style")).compare(U("american")) == 0)
					{
						req.style = ExchangeRateOptionSpreadMessage::AMERICAN;
					}
					else
					{
						throw std::invalid_argument("Invalid Style parameter");
					}
				}
				else
				{
					req.style = ExchangeRateOptionSpreadMessage::AMERICAN;
				}

				if (query_strings.find(U("_method")) != query_strings.end())
				{
					if (query_strings.at(U("_method")).compare(U("closed")) == 0)
					{
						if (req.style == ExchangeRateOptionSpreadMessage::EUROPEAN)
						{
							req.method = ExchangeRateOptionSpreadMessage::CLOSED;
						}
						else
						{
							throw std::invalid_argument("American open cannot be evaluated with closed form");
						}
					}
					else if (query_strings.at(U("_method")).compare(U("lattice")) == 0)
					{
						req.method = ExchangeRateOptionSpreadMessage::LATTICE;
					}					
					else
					{
						throw std::invalid_argument("Invalid pricing method parameter");
					}
				}
				else
				{
					req.method = ExchangeRateOptionSpreadMessage::LATTICE;
				}
				if (query_strings.find(U("_ratetype")) != query_strings.end())
				{
					if (query_strings.at(U("_ratetype")).compare(U("Yield")) == 0)
					{
						req.rateType = ExchangeRateOptionSpreadMessage::YIELD;
					}
					else if (query_strings.at(U("_ratetype")).compare(U("LIBOR")) == 0)
					{
						req.rateType = ExchangeRateOptionSpreadMessage::LIBOR;
					}
					else
					{
						throw std::invalid_argument("Invalid Rate type parameter");
					}
				}
				else
				{
					req.rateType = ExchangeRateOptionSpreadMessage::YIELD;
				}

				if (query_strings.find(U("_voltype")) != query_strings.end())
				{
					if (query_strings.at(U("_voltype")).compare(U("HV")) == 0)
					{
						req.volType = ExchangeRateOptionSpreadMessage::HV;
					}
					else
					{
						throw std::invalid_argument("Invalid Volatility type parameter");
					}
				}
				else
				{
					req.volType = ExchangeRateOptionSpreadMessage::HV;
				}
				if (query_strings.find(U("_domestic")) != query_strings.end())
				{
					req.domestic = conversions::to_utf8string(query_strings.at(U("_domestic")));
				}
				else
				{
					throw std::invalid_argument("No domestic currency symbol");
				}

				if (query_strings.find(U("_foreign")) != query_strings.end())
				{
					req.foreign = conversions::to_utf8string(query_strings.at(U("_foreign")));
				}
				else
				{
					throw std::invalid_argument("No domestic currency symbol");
				}

				if (query_strings.find(U("_vol")) != query_strings.end())
				{
					auto vol = conversions::to_utf8string(query_strings.at(U("_vol")));
					req.vol = stod(vol);
				}

				if (query_strings.find(U("_legs")) != query_strings.end())
				{
					std::string legs = conversions::to_utf8string(query_strings.at(U("_legs")));
					decodeLegs(legs, req.legs);
				}

				if (query_strings.find(U("_naked")) != query_strings.end())
				{
					if (query_strings.at(U("_naked")).compare(U("long")) == 0)
					{
						req.equityPos.pos = ExchangeRateOptionSpreadMessage::LONG;
						if (query_strings.find(U("_units")) != query_strings.end())
						{
							req.equityPos.units = atoi(conversions::to_utf8string(query_strings.at(U("_units"))).c_str());
						}
					}
					else if (query_strings.at(U("_naked")).compare(U("short")) == 0)
					{
						req.equityPos.pos = ExchangeRateOptionSpreadMessage::SHORT;
						if (query_strings.find(U("_units")) != query_strings.end())
						{
							req.equityPos.units = atoi(conversions::to_utf8string(query_strings.at(U("_units"))).c_str());
						}
					}
					else
					{
						throw std::invalid_argument("Invalid Style parameter");
					}
				}
			}
			catch (std::exception& e)
			{
				return SendError<ExchangeRateOptionSpreadMessage, ExchangeRateOptionSpreadMessage::ExchangeRateRequest>(msg, req, e.what());
			}

			return ProcessMsg<ExchangeRateOptionSpreadMessage, ExchangeRateOptionSpreadMessage::ExchangeRateRequest>(msg, req);
		}

	}
}