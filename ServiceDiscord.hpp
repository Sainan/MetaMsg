#pragma once

#include "Service.hpp"

#include <unordered_map>

#include <soup/fwd.hpp>

namespace MetaMsg
{
	struct DiscordGuild;

	struct ServiceDiscord : public Service
	{
		std::string token;
		Channel* log_channel;
		struct DiscordHeartbeatTask* heartbeater;
		std::string username;
		bool is_bot;
		DiscordGuild* dm_guild = nullptr;
		std::unordered_map<std::string, DiscordGuild*> snowflake_map;

		ServiceDiscord(std::string&& token);

		void processGuildCreate(const soup::JsonObject& guild);
		[[nodiscard]] DiscordGuild* getGuild(const soup::JsonObject& obj);

		void submitMessage(Guild* g, Channel* chan, std::string&& message) final;

		void sendRequest(const char* method, std::string&& path, const soup::JsonObject& obj) const;
		[[nodiscard]] std::string getAuthorizationValue() const;
	};
}
