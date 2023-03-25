#pragma once

#include <filesystem>

#include <lua.h>

#include "fwd.hpp"

namespace MetaMsg
{
	class Plugin
	{
	public:
		lua_State* L = nullptr;

		Plugin(std::filesystem::path f)
		{
			load(std::move(f));
		}

		void load(std::filesystem::path f);
		void unload();

		std::vector<uint64_t> on_pre_send_message{};
		void onPreSendMessage(Guild* g, std::string& message);

	private:
		static Plugin* self(lua_State* L);

		static int lua_log(lua_State* L);

		static int lua_onPreSendMessage(lua_State* L);
	};
}
