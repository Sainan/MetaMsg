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

		static void push(lua_State* L, const Guild& g);
		static void push(lua_State* L, const Channel& chan);

		std::vector<uint64_t> on_pre_send_message{};
		void onPreSendMessage(const Guild& g, std::string& message);

		std::vector<uint64_t> on_new_message{};
		void onNewMessage(const Guild& g, const Channel& chan, const Message& msg);

	private:
		static Plugin* self(lua_State* L);

		static int lua_log(lua_State* L);

		static int lua_onPreSendMessage(lua_State* L);
		static int lua_onNewMessage(lua_State* L);
	};
}
