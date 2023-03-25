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

		static void onPreSendMessage(Guild* g, std::string& message);
	};
}
