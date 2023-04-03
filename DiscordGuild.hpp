#pragma once

#include "Guild.hpp"

#include <unordered_map>

#include <soup/fwd.hpp>

namespace MetaMsg
{
	struct DiscordGuild : public Guild
	{
		std::unordered_map<std::string, struct DiscordChannel*> snowflake_map;

		using Guild::Guild;
		
		using Guild::addChannel;
		void addChannel(const soup::JsonObject& chan);

		void sortChannels();
	};
}
