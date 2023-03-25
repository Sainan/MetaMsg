#include "Plugin.hpp"

#include <soup/base.hpp>
#include <soup/log.hpp>

#include <lauxlib.h>
#include <lstate.h>
#include <lualib.h>
#include "lutil.hpp"

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

			lua_pushcfunction(L, lua_onPreSendMessage);
			lua_setglobal(L, "onPreSendMessage");
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

	void Plugin::onPreSendMessage(Guild* g, std::string& message)
	{
		lua_newtable(L);
		{
			lua_pushstring(L, message);
			lua_setfield(L, -2, "message");
		}
		{
			lua_newtable(L);
			{
				lua_pushstring(L, g->name);
				lua_setfield(L, -2, "name");
			}
			{
				lua_newtable(L);
				{
					lua_pushstring(L, g->service->name);
					lua_setfield(L, -2, "name");
				}
				lua_setfield(L, -2, "service");
			}
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

	Plugin* Plugin::self(lua_State* L)
	{
		return reinterpret_cast<Plugin*>(L->l_G->user_data);
	}

	int Plugin::lua_log(lua_State* L)
	{
		soup::logWriteLine(lua_tostring(L, 1));
		return 0;
	}

	int Plugin::lua_onPreSendMessage(lua_State* L)
	{
		self(L)->on_pre_send_message.emplace_back(lutil::checkFunction(L, 1));
		return 0;
	}
}
