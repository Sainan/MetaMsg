#pragma once

#include "Channel.hpp"

namespace MetaMsg
{
	struct DiscordChannel : public Channel
	{
		int type;
		int position;
		std::string parent;

		DiscordChannel(std::string name, int type, int position)
			: Channel(std::move(name), type == 4), type(type), position(position)
		{
		}
	};
}
