#include "ServiceGuilded.hpp"

#include <soup/HttpRequestTask.hpp>
#include <soup/json.hpp>
#include <soup/StateMachineTask.hpp>

#include "common.hpp"
#include "ServiceMeta.hpp"

namespace MetaMsg
{
	struct GuildedInitTask : public StateMachineTask
	{
		enum State : state_t
		{
			REQ_ME = 0,
			REQ_TEAMS,
		};

		ServiceGuilded* serv;
		DelayedCtor<HttpRequestTask> hrt;
		std::vector<std::string> pending_teams{};

		GuildedInitTask(ServiceGuilded* serv)
			: serv(serv)
		{
			HttpRequest hr(Uri("https://www.guilded.gg/api/me?isLogin=false&v2=true"));
			hr.header_fields.emplace("Cookie", "hmac_signed_session=" + serv->token);
			hrt.construct(std::move(hr));
		}

		void onTick() final
		{
			switch (state)
			{
			case REQ_ME:
				if (hrt->tickUntilDone())
				{
					serv->log_channel->addMessage(Message{ "HTTP", hrt->res->body });

					auto root = soup::json::decode(hrt->res->body);
					SOUP_ASSERT(root);
					for (const auto& team : root->asObj().at("teams").asArr())
					{
						auto g = serv->addGuild(soup::make_unique<Guild>(serv, team.asObj().at("name").asStr()));
						g->username = root->asObj().at("user").asObj().at("name").asStr();
						
						serv->team_id_map.emplace(team.asObj().at("id").asStr(), g);

						pending_teams.emplace_back(team.asObj().at("id").asStr());
					}

					hrt.destroy();
					setState(REQ_TEAMS);
				}
				break;

			case REQ_TEAMS:
				if (hrt.isConstructed())
				{
					if (hrt->tickUntilDone())
					{
						serv->log_channel->addMessage(Message{ "HTTP", hrt->res->body });

						Guild* g = serv->team_id_map.at(pending_teams.front());
						pending_teams.erase(pending_teams.begin());

						auto root = soup::json::decode(hrt->res->body);
						SOUP_ASSERT(root);
						for (const auto& chan : root->asObj().at("channels").asArr())
						{
							g->addChannel(soup::make_unique<Channel>(chan.asObj().at("name").asStr()));
						}

						hrt.destroy();
						setState(REQ_TEAMS);
					}
				}
				else
				{
					if (!pending_teams.empty())
					{
						HttpRequest hr(Uri("https://www.guilded.gg/api/teams/" + pending_teams.front() + "/channels?excludeBadgedContent=true"));
						hr.header_fields.emplace("Cookie", "hmac_signed_session=" + serv->token);
						hrt.construct(std::move(hr));
					}
					else
					{
						// TODO: Establish WS
						setWorkDone();
					}
				}
				break;
			}
		}
	};

	ServiceGuilded::ServiceGuilded(std::string&& token)
		: Service("GUILDED"), token(std::move(token))
	{
		log_channel = g_meta->system_guild->addChannel(soup::make_unique<Channel>("guilded-log"));

		g_sched.add<GuildedInitTask>(this);
	}

	void ServiceGuilded::submitMessage(Guild* g, Channel* chan, std::string&& message)
	{
		// TODO
	}
}
