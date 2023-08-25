#include "PluginMgr.hpp"

#include <filesystem>

namespace MetaMsg
{
	void PluginMgr::init()
	{
		if (!std::filesystem::is_directory("plugins"))
		{
			std::filesystem::create_directory("plugins");
		}
		loadPlugins();
	}

	void PluginMgr::loadPlugins()
	{
		for (const auto& f : std::filesystem::directory_iterator("plugins"))
		{
			if (f.is_regular_file())
			{
				auto p = f.path();
				auto n = p.filename().u8string();
				if ((n.length() > 4 && n.substr(n.length() - 4) == u8".lua")
					|| (n.length() > 6 && n.substr(n.length() - 6) == u8".pluto")
					)
				{
					plugins.emplace_back(soup::make_unique<Plugin>(std::move(p)));
				}
			}
		}
	}

	void PluginMgr::unloadPlugins()
	{
		for (const auto& p : plugins)
		{
			p->unload();
		}
		plugins.clear();
	}

	void PluginMgr::onPreSendMessage(const Guild& g, std::string& message)
	{
		for (const auto& p : plugins)
		{
			p->onPreSendMessage(g, message);
		}
	}

	void PluginMgr::onNewMessage(const Guild& g, const Channel& chan, const Message& msg)
	{
		for (const auto& p : plugins)
		{
			p->onNewMessage(g, chan, msg);
		}
	}
}
