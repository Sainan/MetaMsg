#include "ServiceDiscord.hpp"

#include <soup/HttpRequest.hpp>
#include <soup/HttpRequestTask.hpp>
#include <soup/joaat.hpp>
#include <soup/json.hpp>
#include <soup/JsonArray.hpp>
#include <soup/JsonInt.hpp>
#include <soup/JsonObject.hpp>
#include <soup/JsonString.hpp>
#include <soup/netConnectTask.hpp>
#include <soup/Task.hpp>
#include <soup/WebSocketConnection.hpp>
#include <soup/WebSocketMessage.hpp>

#include "common.hpp"
#include "DiscordChannel.hpp"
#include "DiscordGuild.hpp"
#include "DiscordHeartbeatTask.hpp"
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
				else if (type == joaat::hash("MESSAGE_CREATE"))
				{
					Guild* g = serv->snowflake_map.at(root->asObj().at("d").asObj().at("guild_id").asStr());
					Channel* chan = static_cast<DiscordGuild*>(g)->snowflake_map.at(root->asObj().at("d").asObj().at("channel_id").asStr());
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

		//addGuild(soup::make_unique<Guild>("Direct Messages"));

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
			DiscordChannel* c = (DiscordChannel*)g->addChannel(soup::make_unique<DiscordChannel>(
				chan.asObj().at("name").asStr(),
				chan.asObj().at("id").asStr(),
				chan.asObj().at("type").asInt(),
				chan.asObj().at("position").asInt()
			));
			if (chan.asObj().contains("parent_id")
				&& chan.asObj().at("parent_id").isStr() // Sometimes it's null for some reason...
				)
			{
				c->parent = chan.asObj().at("parent_id").asStr();
			}
			g->snowflake_map.emplace(
				c->id,
				c
			);
		}
		std::sort(g->channels.begin(), g->channels.end(), [g](const UniquePtr<Channel>& _a, const UniquePtr<Channel>& _b)
		{
			DiscordChannel& a = *static_cast<DiscordChannel*>(_a.get());
			DiscordChannel& b = *static_cast<DiscordChannel*>(_b.get());

			// Get parent positions
			int a_parent_pos = -1;
			int b_parent_pos = -1;
			if (!a.parent.empty())
			{
				a_parent_pos = g->snowflake_map.at(a.parent)->position;
			}
			if (!b.parent.empty())
			{
				b_parent_pos = g->snowflake_map.at(b.parent)->position;
			}

			// Get group
			int a_group = a_parent_pos;
			int b_group = b_parent_pos;
			if (a.type == 4)
			{
				a_group = a.position;
			}
			if (b.type == 4)
			{
				b_group = b.position;
			}

			// Compare group
			if (a_group < b_group)
			{
				return true;
			}
			if (b_group < a_group)
			{
				return false;
			}

			// Demote voice channels
			if (a.type != 2
				&& b.type == 2
				)
			{
				return true;
			}
			if (b.type != 2
				&& a.type == 2
				)
			{
				return false;
			}

			return a.position < b.position;
		});
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
	
	struct HttpRequestWrapperTask : public Task
	{
		const ServiceDiscord* serv;
		HttpRequestTask hrt;

		HttpRequestWrapperTask(const ServiceDiscord* serv, HttpRequest&& hr)
			: Task(), serv(serv), hrt(g_sched, std::move(hr))
		{
		}

		void onTick() final
		{
			if (hrt.tickUntilDone())
			{
				serv->log_channel->addMessage(Message{ "HTTP", hrt.res->body });
				setWorkDone();
			}
		}
	};

	void ServiceDiscord::sendRequest(const char* method, std::string&& path, const soup::JsonObject& obj) const
	{
		HttpRequest hr(method, "discord.com", path);
		hr.header_fields.emplace("Authorization", getAuthorizationValue());
		
		// Discord's Cloudflare rules seem to require this UA format if "Authorization: Bot ..." 
		hr.header_fields.at("User-Agent") = "DiscordBot (Please momma no spaghetti)";

		hr.header_fields.emplace("Content-Type", "application/json");
		hr.setPayload(obj.encode());

		g_sched.add<HttpRequestWrapperTask>(this, std::move(hr));
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
