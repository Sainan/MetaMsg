#pragma once

#include "Service.hpp"

#include <unordered_map>

namespace MetaMsg
{
	struct ServiceGuilded : public Service
	{
		std::string token;
		Channel* log_channel;
		std::unordered_map<std::string, Guild*> team_id_map;

		ServiceGuilded(std::string&& token);

		void submitMessage(Guild* g, Channel* chan, std::string&& message) final;
	};
}
