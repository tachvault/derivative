/*
Copyright (C) Nathan Muruganantha 2013 - 2014
*/

#include <memory>

#include "WebServiceUtil.hpp"
#include "EquityVanillaOptMessage.hpp"
#include "FuturesVanillaOptMessage.hpp"
#include "ESBManager.hpp"

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

		web::json::value HandleEquityOption(const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings)
		{
			if (!paths[1].empty() && paths[1].compare(U("Vanilla")) == 0)
			{
				/// now add request parameters
				EquityVanillaOptMessage::Request req;
				if (query_strings.find(U("_option")) != query_strings.end())
				{
					if (query_strings.at(U("_option")).compare(U("call")) == 0)
					{
						req.option = EquityVanillaOptMessage::CALL;
						return HandleEquityVanillaOption(EquityVanillaOptMessage::CALL, paths, query_strings);
					}
					else if (query_strings.at(U("_option")).compare(U("put")) == 0)
					{
						req.option = EquityVanillaOptMessage::PUT;
						return HandleEquityVanillaOption(EquityVanillaOptMessage::PUT, paths, query_strings);
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

		web::json::value HandleEquityVanillaOption(EquityVanillaOptMessage::OptionTypeEnum opt, const std::vector<string_t>& paths, const std::map<string_t, string_t>& query_strings)
		{
			/// now add request parameters
			std::shared_ptr<EquityVanillaOptMessage> msg = std::make_shared<EquityVanillaOptMessage>();
			EquityVanillaOptMessage::Request req;
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
					else if (query_strings.at(U("_method")).compare(U("montecarlo")) == 0)
					{
						req.method = EquityVanillaOptMessage::MONTE_CARLO;
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
						throw std::invalid_argument("Invalid Style parameter");
					}
				}
				else
				{
					req.rateType = EquityVanillaOptMessage::YIELD;
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

				if (query_strings.find(U("_strike")) != query_strings.end())
				{
					auto strike = conversions::to_utf8string(query_strings.at(U("_strike")));
					req.strike = stod(strike);
				}
				else
				{
					throw std::invalid_argument("No strike value");
				}
			}
			catch (std::exception& e)
			{
				return SendError<EquityVanillaOptMessage, VanillaOptMessage::Request>(msg, req, e.what());
			}

			return ProcessMsg<EquityVanillaOptMessage, VanillaOptMessage::Request>(msg, req);
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
						throw std::invalid_argument("Invalid Style parameter");
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
			}
			catch (std::exception& e)
			{
				return SendError<FuturesVanillaOptMessage, FuturesVanillaOptMessage::FuturesRequest>(msg, req, e.what());
			}

			return ProcessMsg<FuturesVanillaOptMessage, FuturesVanillaOptMessage::FuturesRequest>(msg, req);
		}
	}
}