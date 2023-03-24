#pragma once

#include "Channel.hpp"

namespace MetaMsg
{
	struct DiscordChannel : public Channel
	{
		std::string id;
		int type;
		int position;
		std::string parent;

		DiscordChannel(std::string name, std::string id, int type, int position)
			: Channel(std::move(name), type == 4), id(std::move(id)), type(type), position(position)
		{
		}
	};
}
