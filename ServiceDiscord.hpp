#pragma once

#include "Service.hpp"

#include <unordered_map>

namespace MetaMsg
{
	struct ServiceDiscord : public Service
	{
		std::string token;
		Channel* log_channel;
		struct DiscordHeartbeatTask* heartbeater;
		std::string username;
		std::unordered_map<std::string, struct DiscordGuild*> snowflake_map;

		ServiceDiscord(std::string&& token);
	};
}
