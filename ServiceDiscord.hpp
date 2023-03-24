#pragma once

#include "Service.hpp"

#include <unordered_map>

#include <soup/fwd.hpp>

namespace MetaMsg
{
	struct ServiceDiscord : public Service
	{
		std::string token;
		Channel* log_channel;
		struct DiscordHeartbeatTask* heartbeater;
		std::string username;
		bool is_bot;
		std::unordered_map<std::string, struct DiscordGuild*> snowflake_map;

		ServiceDiscord(std::string&& token);

		void processGuildCreate(const soup::JsonObject& guild);
	};
}
