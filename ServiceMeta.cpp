#include "ServiceMeta.hpp"

#include <soup/console.hpp>
#include <soup/log.hpp>

#include "common.hpp"
#include "PluginMgr.hpp"
#include "ServiceDiscord.hpp"
#include "ServiceGuilded.hpp"

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
		else if (message == "/reload")
		{
			PluginMgr::unloadPlugins();
			PluginMgr::loadPlugins();
			logWriteLine("Plugins reloaded");
		}
		else if (message.substr(0, 9) == "/discord ")
		{
			g_services.emplace_back(soup::make_unique<ServiceDiscord>(message.substr(9)));
		}
		else if (message.substr(0, 9) == "/guilded ")
		{
			g_services.emplace_back(soup::make_unique<ServiceGuilded>(message.substr(9)));
		}
	}
}
