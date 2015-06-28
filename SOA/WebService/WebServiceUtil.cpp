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
#include "FuturesBarrierOptMessage.hpp"
#include "FuturesAverageOptMessage.hpp"
#include "ExchangeRateBarrierOptMessage.hpp"
#include "ExchangeRateAverageOptMessage.hpp"
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
				return HandleEquityAverageOption(paths, query_strings);
			}
			else if (!paths[1].empty() && paths[1].compare(U("LookBack")) == 0)
			{
				return HandleEquityLookBackOption(paths, query_strings);
			}
			else if (!paths[1].empty() && paths[1].compare(U("Chooser")) == 0)
			{
				return HandleEquityChooserOption(paths, query_strings);
			}
			else if (!paths[1].empty() && paths[1].compare(U("Margrabe")) == 0)
			{
				return HandleEquityMargrabeOption(paths, query_strings);
			}
			else
			{
				throw std::invalid_argument("Invalid option type");
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
				if (req.style == VanillaOptMessage::STYLE_UNKNOWN)
				{
					req.style = VanillaOptMessage::EUROPEAN;
				}

				req.method = msg->ParsePricingMethod(query_strings);
				if (req.method == VanillaOptMessage::METHOD_UNKNOWN)
				{
					req.method = VanillaOptMessage::LATTICE;
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
				if (req.style == VanillaOptMessage::STYLE_UNKNOWN)
				{
					req.style = VanillaOptMessage::EUROPEAN;
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
				req.averageType = msg->ParseAverageType(query_strings);
				msg->ParseMaturity(req, query_strings);
				req.option = msg->ParseOptionType(query_strings);
				if (req.option == VanillaOptMessage::TYPE_UNKNOWN)
				{
					req.option = VanillaOptMessage::CALL;
				};
				if (req.averageType == EquityAverageOptMessage::FIXED_STRIKE)
				{
					msg->ParseStrike(req, query_strings);
				}
				msg->ParseVol(req, query_strings);				
				req.style = msg->ParseOptionStyle(query_strings);
				if (req.style == VanillaOptMessage::STYLE_UNKNOWN)
				{
					req.style = VanillaOptMessage::EUROPEAN;
				}			
				req.method = msg->ParsePricingMethod(query_strings);
				if (req.method == VanillaOptMessage::METHOD_UNKNOWN)
				{
					req.method = VanillaOptMessage::MONTE_CARLO;
				}
				else if (req.method != VanillaOptMessage::MONTE_CARLO)
				{
					throw std::invalid_argument("Only monte carlo valuation is supported");
				}
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
			EquityLookBackOptMessage::LookBackOptRequest req;
			try
			{
				msg->ParseSymbol(req, query_strings);
				req.lookBackType = msg->ParseLookBackType(query_strings);
				msg->ParseMaturity(req, query_strings);
				if (req.lookBackType == EquityLookBackOptMessage::FIXED_STRIKE)
				{
					msg->ParseStrike(req, query_strings);
				}
				msg->ParseVol(req, query_strings);
				if (req.option == VanillaOptMessage::TYPE_UNKNOWN)
				{
					req.option = VanillaOptMessage::CALL;
				};
				req.style = msg->ParseOptionStyle(query_strings);
				if (req.style == VanillaOptMessage::STYLE_UNKNOWN)
				{
					req.style = VanillaOptMessage::EUROPEAN;
				}
				else if (req.style == VanillaOptMessage::AMERICAN)
				{
					throw std::invalid_argument("Only European style lookback options are supported");
				}
				req.method = msg->ParsePricingMethod(query_strings);
				if (req.method == VanillaOptMessage::METHOD_UNKNOWN)
				{
					req.method = VanillaOptMessage::MONTE_CARLO;
				}
				else if (req.method != VanillaOptMessage::MONTE_CARLO)
				{
					throw std::invalid_argument("Only monte carlo valuation is supported");
				}
				req.rateType = msg->ParseRateType(query_strings);
				req.volType = msg->ParseVolType(query_strings);
			}
			catch (std::exception& e)
			{
				return SendError<EquityLookBackOptMessage, EquityLookBackOptMessage::LookBackOptRequest>(msg, req, e.what());
			}
			return ProcessMsg<EquityLookBackOptMessage, EquityLookBackOptMessage::LookBackOptRequest>(msg, req);
		}

		web::json::value HandleEquityChooserOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings)
		{
			/// now add request parameters
			std::shared_ptr<EquityChooserOptMessage> msg = std::make_shared<EquityChooserOptMessage>();
			EquityChooserOptMessage::Request req;
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
				if (req.style == VanillaOptMessage::STYLE_UNKNOWN)
				{
					req.style = VanillaOptMessage::EUROPEAN;
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
					req.numeraire = conversions::to_utf8string(query_strings.at(U("_numeraire")));
				}
				else
				{
					throw std::invalid_argument("No numeraire symbol");
				}
				req.option = msg->ParseOptionType(query_strings);
				if (req.option == VanillaOptMessage::TYPE_UNKNOWN)
				{
					req.option = VanillaOptMessage::CALL;
				};
				req.style = msg->ParseOptionStyle(query_strings);
				if (req.style == VanillaOptMessage::STYLE_UNKNOWN)
				{
					req.style = VanillaOptMessage::EUROPEAN;
				}
				req.method = msg->ParsePricingMethod(query_strings);
				if (req.method == VanillaOptMessage::METHOD_UNKNOWN)
				{
					if (req.style == VanillaOptMessage::EUROPEAN)
					{
						req.method = VanillaOptMessage::CLOSED;
					}
					else
					{
						req.method = VanillaOptMessage::MONTE_CARLO;
					}
				}
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
				return HandleFuturesVanillaOption(paths, query_strings);
			}
			else if (!paths[1].empty() && paths[1].compare(U("Barrier")) == 0)
			{
				return HandleFuturesBarrierOption(paths, query_strings);
			}
			else if (!paths[1].empty() && paths[1].compare(U("Average")) == 0)
			{
				return HandleFuturesAverageOption(paths, query_strings);
			}			
			else
			{
				throw std::invalid_argument("Invalid option type");
			}
		}

		web::json::value HandleFuturesVanillaOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings)
		{
			/// now add request parameters
			std::shared_ptr<FuturesVanillaOptMessage> msg = std::make_shared<FuturesVanillaOptMessage>();
			FuturesVanillaOptMessage::FuturesRequest req;
			try
			{
				msg->ParseSymbol(req, query_strings);
				msg->ParseMaturity(req, query_strings);
				msg->ParseDeliveryDate(req, query_strings);
				msg->ParseStrike(req, query_strings);
				msg->ParseVol(req, query_strings);
				req.option = msg->ParseOptionType(query_strings);
				if (req.option == VanillaOptMessage::TYPE_UNKNOWN)
				{
					req.option = VanillaOptMessage::CALL;
				};

				req.style = msg->ParseOptionStyle(query_strings);
				if (req.style == VanillaOptMessage::STYLE_UNKNOWN)
				{
					req.style = VanillaOptMessage::EUROPEAN;
				}

				req.method = msg->ParsePricingMethod(query_strings);
				if (req.method == VanillaOptMessage::METHOD_UNKNOWN)
				{
					req.method = VanillaOptMessage::LATTICE;
				}
				req.rateType = msg->ParseRateType(query_strings);
				req.volType = VanillaOptMessage::IV;
			}
			catch (std::exception& e)
			{
				return SendError<FuturesVanillaOptMessage, VanillaOptMessage::Request>(msg, req, e.what());
			}

			return ProcessMsg<FuturesVanillaOptMessage, FuturesVanillaOptMessage::FuturesRequest>(msg, req);
		}

		web::json::value HandleFuturesBarrierOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings)
		{
			/// now add request parameters
			std::shared_ptr<FuturesBarrierOptMessage> msg = std::make_shared<FuturesBarrierOptMessage>();
			FuturesBarrierOptMessage::BarrierRequest req;
			try
			{
				msg->ParseSymbol(req, query_strings);
				msg->ParseMaturity(req, query_strings);
				msg->ParseDeliveryDate(req, query_strings);
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
				if (req.style == VanillaOptMessage::STYLE_UNKNOWN)
				{
					req.style = VanillaOptMessage::EUROPEAN;
				}

				req.barrierType = msg->ParseBarrierType(query_strings);
				req.method = VanillaOptMessage::MONTE_CARLO;
				req.rateType = msg->ParseRateType(query_strings);
				req.volType = VanillaOptMessage::IV;
			}
			catch (std::exception& e)
			{
				return SendError<FuturesBarrierOptMessage, FuturesBarrierOptMessage::BarrierRequest>(msg, req, e.what());
			}

			return ProcessMsg<FuturesBarrierOptMessage, FuturesBarrierOptMessage::BarrierRequest>(msg, req);
		}

		web::json::value HandleFuturesAverageOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings)
		{
			/// now add request parameters
			std::shared_ptr<FuturesAverageOptMessage> msg = std::make_shared<FuturesAverageOptMessage>();
			FuturesAverageOptMessage::AverageOptRequest req;
			try
			{
				msg->ParseSymbol(req, query_strings);
				req.averageType = msg->ParseAverageType(query_strings);
				msg->ParseMaturity(req, query_strings);
				msg->ParseDeliveryDate(req, query_strings);
				req.option = msg->ParseOptionType(query_strings);
				if (req.option == VanillaOptMessage::TYPE_UNKNOWN)
				{
					req.option = VanillaOptMessage::CALL;
				};
				if (req.averageType == FuturesAverageOptMessage::FIXED_STRIKE)
				{
					msg->ParseStrike(req, query_strings);
				}
				msg->ParseVol(req, query_strings);
				req.style = msg->ParseOptionStyle(query_strings);
				if (req.style == VanillaOptMessage::STYLE_UNKNOWN)
				{
					req.style = VanillaOptMessage::EUROPEAN;
				}
				req.method = msg->ParsePricingMethod(query_strings);
				if (req.method == VanillaOptMessage::METHOD_UNKNOWN)
				{
					req.method = VanillaOptMessage::MONTE_CARLO;
				}
				else if (req.method != VanillaOptMessage::MONTE_CARLO)
				{
					throw std::invalid_argument("Only monte carlo valuation is supported");
				}
				req.rateType = msg->ParseRateType(query_strings);
				req.volType = VanillaOptMessage::IV;
			}
			catch (std::exception& e)
			{
				return SendError<FuturesAverageOptMessage, FuturesAverageOptMessage::AverageOptRequest>(msg, req, e.what());
			}

			return ProcessMsg<FuturesAverageOptMessage, FuturesAverageOptMessage::AverageOptRequest>(msg, req);
		}
	
		web::json::value HandleFXOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings)
		{
			if (!paths[1].empty() && paths[1].compare(U("Vanilla")) == 0)
			{
				return HandleFXVanillaOption(paths, query_strings);
			}
			else if (!paths[1].empty() && paths[1].compare(U("Barrier")) == 0)
			{
				return HandleFXBarrierOption(paths, query_strings);
			}
			else if (!paths[1].empty() && paths[1].compare(U("Average")) == 0)
			{
				return HandleFXAverageOption(paths, query_strings);
			}
			else
			{
				throw std::invalid_argument("Invalid option type");
			}
		}

		web::json::value HandleFXVanillaOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings)
		{
			/// now add request parameters
			std::shared_ptr<ExchangeRateVanillaOptMessage> msg = std::make_shared<ExchangeRateVanillaOptMessage>();
			ExchangeRateVanillaOptMessage::ExchangeRateRequest req;
			try
			{
				msg->ParseDomestic(req, query_strings);
				msg->ParseForeign(req, query_strings);
				msg->ParseMaturity(req, query_strings);
				msg->ParseStrike(req, query_strings);
				msg->ParseVol(req, query_strings);
				req.option = msg->ParseOptionType(query_strings);
				if (req.option == VanillaOptMessage::TYPE_UNKNOWN)
				{
					req.option = VanillaOptMessage::CALL;
				};

				req.style = msg->ParseOptionStyle(query_strings);
				if (req.style == VanillaOptMessage::STYLE_UNKNOWN)
				{
					req.style = VanillaOptMessage::EUROPEAN;
				}

				req.method = msg->ParsePricingMethod(query_strings);
				if (req.method == VanillaOptMessage::METHOD_UNKNOWN)
				{
					req.method = VanillaOptMessage::LATTICE;
				}
				req.rateType = msg->ParseRateType(query_strings);
				req.volType = VanillaOptMessage::IV;
			}
			catch (std::exception& e)
			{
				return SendError<ExchangeRateVanillaOptMessage, VanillaOptMessage::Request>(msg, req, e.what());
			}

			return ProcessMsg<ExchangeRateVanillaOptMessage, ExchangeRateVanillaOptMessage::ExchangeRateRequest>(msg, req);
		}

		web::json::value HandleFXBarrierOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings)
		{
			/// now add request parameters
			std::shared_ptr<ExchangeRateBarrierOptMessage> msg = std::make_shared<ExchangeRateBarrierOptMessage>();
			ExchangeRateBarrierOptMessage::BarrierRequest req;
			try
			{
				msg->ParseDomestic(req, query_strings);
				msg->ParseForeign(req, query_strings);
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
				if (req.style == VanillaOptMessage::STYLE_UNKNOWN)
				{
					req.style = VanillaOptMessage::EUROPEAN;
				}

				req.barrierType = msg->ParseBarrierType(query_strings);
				req.method = VanillaOptMessage::MONTE_CARLO;
				req.rateType = msg->ParseRateType(query_strings);
				req.volType = VanillaOptMessage::IV;
			}
			catch (std::exception& e)
			{
				return SendError<ExchangeRateBarrierOptMessage, ExchangeRateBarrierOptMessage::BarrierRequest>(msg, req, e.what());
			}

			return ProcessMsg<ExchangeRateBarrierOptMessage, ExchangeRateBarrierOptMessage::BarrierRequest>(msg, req);
		}

		web::json::value HandleFXAverageOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings)
		{
			/// now add request parameters
			std::shared_ptr<ExchangeRateAverageOptMessage> msg = std::make_shared<ExchangeRateAverageOptMessage>();
			ExchangeRateAverageOptMessage::AverageOptRequest req;
			try
			{
				msg->ParseDomestic(req, query_strings);
				msg->ParseForeign(req, query_strings);
				msg->ParseMaturity(req, query_strings);
				req.averageType = msg->ParseAverageType(query_strings);
				req.option = msg->ParseOptionType(query_strings);
				if (req.option == VanillaOptMessage::TYPE_UNKNOWN)
				{
					req.option = VanillaOptMessage::CALL;
				};
				if (req.averageType == ExchangeRateAverageOptMessage::FIXED_STRIKE)
				{
					msg->ParseStrike(req, query_strings);
				}
				msg->ParseVol(req, query_strings);
				req.style = msg->ParseOptionStyle(query_strings);
				if (req.style == VanillaOptMessage::STYLE_UNKNOWN)
				{
					req.style = VanillaOptMessage::EUROPEAN;
				}
				req.method = msg->ParsePricingMethod(query_strings);
				if (req.method == VanillaOptMessage::METHOD_UNKNOWN)
				{
					req.method = VanillaOptMessage::MONTE_CARLO;
				}
				else if (req.method != VanillaOptMessage::MONTE_CARLO)
				{
					throw std::invalid_argument("Only monte carlo valuation is supported");
				}
				req.rateType = msg->ParseRateType(query_strings);
				req.volType = VanillaOptMessage::IV;
			}
			catch (std::exception& e)
			{
				return SendError<ExchangeRateAverageOptMessage, ExchangeRateAverageOptMessage::AverageOptRequest>(msg, req, e.what());
			}

			return ProcessMsg<ExchangeRateAverageOptMessage, ExchangeRateAverageOptMessage::AverageOptRequest>(msg, req);
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