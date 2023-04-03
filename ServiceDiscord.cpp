#include "ServiceDiscord.hpp"

#include <soup/joaat.hpp>
#include <soup/json.hpp>
#include <soup/netConnectTask.hpp>
#include <soup/WebSocketConnection.hpp>
#include <soup/WebSocketMessage.hpp>

#include "common.hpp"
#include "DiscordChannel.hpp"
#include "DiscordGuild.hpp"
#include "DiscordHeartbeatTask.hpp"
#include "HttpRequestWrapperTask.hpp"
#include "ServiceMeta.hpp"

namespace MetaMsg
{
	using namespace soup;

	static void recvLoop(ServiceDiscord* serv, WebSocketConnection& ws)
	{
		ws.wsRecv([](WebSocketConnection& ws, WebSocketMessage&& msg, Capture&& cap)
		{
			auto serv = cap.get<ServiceDiscord*>();

			serv->log_channel->addMessage(Message{ "Gateway", msg.data });

			auto root = json::decode(msg.data);
			SOUP_ASSERT(root);
			if (root->asObj().at("op").asInt() == 0)
			{
				auto type = joaat::hash(root->asObj().at("t").asStr());
				if (type == joaat::hash("READY"))
				{
					serv->username = root->asObj().at("d").asObj().at("user").asObj().at("username").asStr();
					serv->is_bot = root->asObj().at("d").asObj().at("user").asObj().contains("bot");

					serv->dm_guild = (DiscordGuild*)serv->addGuild(soup::make_unique<DiscordGuild>(serv, "Direct Messages"));
					serv->dm_guild->username = serv->username;

					for (const auto& guild : root->asObj().at("d").asObj().at("guilds").asArr())
					{
						if (!guild.asObj().contains("properties"))
						{
							// It may be unavailable, we'll get GUILD_CREATE later.
							continue;
						}
						serv->processGuildCreate(guild.asObj());
					}
				}
				else if (type == joaat::hash("GUILD_CREATE"))
				{
					serv->processGuildCreate(root->asObj().at("d").asObj());
				}
				else if (type == joaat::hash("CHANNEL_CREATE"))
				{
					auto g = serv->getGuild(root->asObj().at("d").asObj());
					g->addChannel(root->asObj().at("d").asObj());
					g->sortChannels();
				}
				else if (type == joaat::hash("MESSAGE_CREATE"))
				{
					auto g = serv->getGuild(root->asObj().at("d").asObj());
					Channel* chan = g->snowflake_map.at(root->asObj().at("d").asObj().at("channel_id").asStr());
					chan->addMessage(Message{
						root->asObj().at("d").asObj().at("author").asObj().at("username").asStr(),
						root->asObj().at("d").asObj().at("content").asStr()
					});
				}
			}
			if (root->asObj().contains("s")
				&& root->asObj().at("s").isInt()
				)
			{
				serv->heartbeater->last_seq = root->asObj().at("s").asInt();
			}

			recvLoop(serv, ws);
		}, serv);
	}

	struct DiscordInitGatewayTask : public Task
	{
		ServiceDiscord* serv;
		netConnectTask connector{ g_sched, "gateway.discord.gg", 443 };

		DiscordInitGatewayTask(ServiceDiscord* serv)
			: serv(serv)
		{
		}

		void onTick() final
		{
			if (connector.tickUntilDone())
			{
				auto s = connector.onDone(g_sched);
				static_assert(sizeof(WebSocketConnection) == sizeof(Socket)); // netConnectTask makes us a Socket, but we want a WebSocketConnection. This is fine as long as they're the same, data-wise.
				s->enableCryptoClient("gateway.discord.gg", [](Socket& s, Capture&& cap)
				{
					static_cast<WebSocketConnection&>(s).upgrade("gateway.discord.gg", "/", [](WebSocketConnection& ws, Capture&& cap)
					{
						ws.wsRecv([](WebSocketConnection& ws, WebSocketMessage&& msg, Capture&& cap)
						{
							auto serv = cap.get<ServiceDiscord*>();

							serv->log_channel->addMessage(Message{ "Gateway", msg.data });

							auto root = json::decode(msg.data);
							SOUP_ASSERT(root);
							serv->heartbeater = (DiscordHeartbeatTask*)Scheduler::get()->addWorker(soup::make_shared<DiscordHeartbeatTask>(
								Scheduler::get()->getShared(ws),
								root->asObj().at("d").asObj().at("heartbeat_interval").asInt()
							)).get();

							std::string identify = R"({"op":2,"d":{"token":")";
							identify.append(serv->token);
							identify.append(R"(","capabilities":4093,"properties":{"os":"Windows","browser":"Chrome","device":"","system_locale":"en-GB","browser_user_agent":"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/111.0.0.0 Safari/537.36","browser_version":"111.0.0.0","os_version":"10","referrer":"","referring_domain":"","referrer_current":"","referring_domain_current":"","release_channel":"stable","client_build_number":182983,"client_event_source":null,"design_id":0},"presence":{"status":"online","since":0,"activities":[],"afk":false},"compress":false,"client_state":{"guild_versions":{},"highest_last_message_id":"0","read_state_version":0,"user_guild_settings_version":-1,"user_settings_version":-1,"private_channels_version":"0","api_code_version":0}}})");
							ws.wsSend(std::move(identify), true);

							recvLoop(serv, ws);
						}, std::move(cap));
					}, std::move(cap));
				}, serv);
				setWorkDone();
			}
		}
	};

	ServiceDiscord::ServiceDiscord(std::string&& token)
		: Service("DISCORD"), token(std::move(token))
	{
		log_channel = g_meta->system_guild->addChannel(soup::make_unique<Channel>("discord-log"));

		g_sched.add<DiscordInitGatewayTask>(this);
	}

	void ServiceDiscord::processGuildCreate(const soup::JsonObject& guild)
	{
		DiscordGuild* g = (DiscordGuild*)addGuild(soup::make_unique<DiscordGuild>(
			this,
			guild.asObj().at("properties").asObj().at("name").asStr()
		));
		g->username = username;
		snowflake_map.emplace(
			guild.at("properties").asObj().at("id").asStr(),
			g
		);
		for (const auto& chan : guild.at("channels").asArr())
		{
			g->addChannel(chan.asObj());
		}
		g->sortChannels();
	}

	DiscordGuild* ServiceDiscord::getGuild(const soup::JsonObject& obj)
	{
		if (obj.contains("guild_id"))
		{
			return snowflake_map.at(obj.at("guild_id").asStr());
		}
		else
		{
			return dm_guild;
		}
	}

	void ServiceDiscord::submitMessage(Guild* g, Channel* chan, std::string&& message)
	{
		std::string path = "/api/v10/channels/";
		path.append(static_cast<DiscordChannel*>(chan)->id);
		path.append("/messages");

		soup::JsonObject obj;
		obj.add("content", std::move(message));

		sendRequest("POST", std::move(path), obj);
	}
	
	void ServiceDiscord::sendRequest(const char* method, std::string&& path, const soup::JsonObject& obj) const
	{
		HttpRequest hr(method, "discord.com", path);
		hr.header_fields.emplace("Authorization", getAuthorizationValue());
		
		// Discord's Cloudflare rules seem to require this UA format if "Authorization: Bot ..." 
		hr.header_fields.at("User-Agent") = "DiscordBot (Please momma no spaghetti)";

		hr.header_fields.emplace("Content-Type", "application/json");
		hr.setPayload(obj.encode());

		g_sched.add<HttpRequestWrapperTask>(log_channel, std::move(hr));
	}

	std::string ServiceDiscord::getAuthorizationValue() const
	{
		std::string val = token;
		if (is_bot)
		{
			val.insert(0, "Bot ");
		}
		return val;
	}
}
