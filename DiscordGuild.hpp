#pragma once

#include "Guild.hpp"

#include <unordered_map>

namespace MetaMsg
{
	struct DiscordGuild : public Guild
	{
		std::unordered_map<std::string, struct DiscordChannel*> snowflake_map;

		using Guild::Guild;
	};
}
