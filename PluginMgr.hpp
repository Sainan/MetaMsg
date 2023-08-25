#pragma once

#include <vector>

#include <soup/UniquePtr.hpp>

#include "Plugin.hpp"

namespace MetaMsg
{
	using namespace soup;

	struct PluginMgr
	{
		inline static std::vector<UniquePtr<Plugin>> plugins{};

		static void init();
		static void loadPlugins();
		static void unloadPlugins();

		static void onPreSendMessage(const Guild& g, std::string& message);
		static void onNewMessage(const Guild& g, const Channel& chan, const Message& msg);
	};
}
