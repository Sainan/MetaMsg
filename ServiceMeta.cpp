#include "ServiceMeta.hpp"

#include <soup/console.hpp>

#include "common.hpp"
#include "ServiceDiscord.hpp"

namespace MetaMsg
{
	ServiceMeta::ServiceMeta()
		: Service("METAMSG")
	{
		system_guild = internal_addGuild(soup::make_unique<Guild>(this, "System Guild"));
		log_channel = system_guild->internal_addChannel(soup::make_unique<Channel>("log"));
	}

	void ServiceMeta::submitMessage(Guild*, Channel*, std::string&& message)
	{
		if (message == "/quit")
		{
			console.cleanup();
			exit(0);
		}
		else if (message.substr(0, 9) == "/discord ")
		{
			g_services.emplace_back(soup::make_unique<ServiceDiscord>(message.substr(9)));
		}
	}
}
