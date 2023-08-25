#include "Plugin.hpp"

#include <soup/base.hpp>
#include <soup/log.hpp>

#include <lauxlib.h>
#include <lstate.h>
#include <lualib.h>
#include "lutil.hpp"

#include "common.hpp"
#include "Guild.hpp"
#include "Service.hpp"

namespace MetaMsg
{
	void Plugin::load(std::filesystem::path f)
	{
		SOUP_ASSERT(L == nullptr);

		// Init
		L = luaL_newstate();
		luaL_openlibs(L);
		L->l_G->user_data = this;

		// Expose stuff
		{
			lua_pushcfunction(L, lua_log);
			lua_setglobal(L, "log");

			lua_getglobal(L, "log");
			lua_setglobal(L, "print");

			lua_pushcfunction(L, lua_submitMessage);
			lua_setglobal(L, "submitMessage");

			lua_pushcfunction(L, lua_onPreSendMessage);
			lua_setglobal(L, "onPreSendMessage");

			lua_pushcfunction(L, lua_onNewMessage);
			lua_setglobal(L, "onNewMessage");
		}

		// Run
		luaL_dofile(L, reinterpret_cast<const char*>(f.u8string().c_str()));
	}

	void Plugin::unload()
	{
		SOUP_ASSERT(L != nullptr);
		lua_close(L);
		L = nullptr;
	}

	void Plugin::push(lua_State* L, const Guild& g)
	{
		lua_newtable(L);
		{
			pluto_pushstring(L, g.name);
			lua_setfield(L, -2, "name");
		}
		{
			pluto_pushstring(L, g.username);
			lua_setfield(L, -2, "username");
		}
		{
			lua_newtable(L);
			{
				lua_pushstring(L, g.service->name);
				lua_setfield(L, -2, "name");
			}
			lua_setfield(L, -2, "service");
		}
	}

	void Plugin::push(lua_State* L, const Channel& chan)
	{
		lua_newtable(L);
		{
			pluto_pushstring(L, chan.name);
			lua_setfield(L, -2, "name");
		}
	}

	void Plugin::onPreSendMessage(const Guild& g, std::string& message)
	{
		lua_newtable(L);
		{
			pluto_pushstring(L, message);
			lua_setfield(L, -2, "message");
		}
		{
			push(L, g);
			lua_setfield(L, -2, "guild");
		}
		lua_setglobal(L, "argtmp");

		for (const auto& ref : on_pre_send_message)
		{
			lutil::pushFunctionKey(L, ref);
			lua_gettable(L, LUA_REGISTRYINDEX);

			lua_getglobal(L, "argtmp");

			lua_call(L, 1, 0);
		}

		lua_getglobal(L, "argtmp");
		lua_getfield(L, -1, "message");
		message = lua_tostring(L, -1);
		lua_pop(L, 2);

		lua_pushnil(L);
		lua_setglobal(L, "argtmp");
	}

	void Plugin::onNewMessage(const Guild& g, const Channel& chan, const Message& msg)
	{
		for (const auto& ref : on_new_message)
		{
			lutil::pushFunctionKey(L, ref);
			lua_gettable(L, LUA_REGISTRYINDEX);

			lua_newtable(L);
			{
				push(L, g);
				lua_setfield(L, -2, "guild");
			}
			{
				push(L, chan);
				lua_setfield(L, -2, "channel");
			}
			{
				pluto_pushstring(L, msg.author);
				lua_setfield(L, -2, "author");
			}
			{
				pluto_pushstring(L, msg.contents);
				lua_setfield(L, -2, "contents");
			}

			lua_call(L, 1, 0);
		}
	}

	Plugin* Plugin::self(lua_State* L)
	{
		return reinterpret_cast<Plugin*>(L->l_G->user_data);
	}

	int Plugin::lua_log(lua_State* L)
	{
		soup::logWriteLine(lua_tostring(L, 1));
		return 0;
	}

	int Plugin::lua_submitMessage(lua_State* L)
	{
		const char* service_name = luaL_checkstring(L, 1);
		const char* guild_name = luaL_checkstring(L, 2);
		const char* channel_name = luaL_checkstring(L, 3);
		const char* contents = luaL_checkstring(L, 4);

		if (auto serv = findServiceByName(service_name))
		{
			if (auto g = serv->findGuildByName(guild_name))
			{
				if (auto chan = g->findChannelByName(channel_name))
				{
					serv->submitMessage(g, chan, contents);

					lua_pushboolean(L, 1);
					return 1;
				}
			}
		}

		lua_pushboolean(L, 0);
		return 1;
	}

	int Plugin::lua_onPreSendMessage(lua_State* L)
	{
		self(L)->on_pre_send_message.emplace_back(lutil::checkFunction(L, 1));
		return 0;
	}

	int Plugin::lua_onNewMessage(lua_State* L)
	{
		self(L)->on_new_message.emplace_back(lutil::checkFunction(L, 1));
		return 0;
	}
}
