#pragma once

#include <cstdint>
#include <string>

struct lutil
{
	inline static uint64_t func_id = 0;

	static void pushFunctionKey(lua_State* L, uint64_t ref)
	{
		std::string key("FuncRef");
		key.append(std::to_string(ref));
		lua_pushstring(L, key.c_str());
	}

	[[nodiscard]] static uint64_t checkFunction(lua_State* L, int i)
	{
		if (lua_gettop(L) < i)
		{
			luaL_error(L, std::string("Missing parameter ").append(std::to_string(i)).append(" of type function").c_str());
		}
		if (!lua_isfunction(L, i))
		{
			luaL_error(L, std::string("Expected parameter ").append(std::to_string(i)).append(" to be a function").c_str());
		}
		return refFunction(L, i);
	}

	[[nodiscard]] static uint64_t refFunction(lua_State* L, int i)
	{
		pushFunctionKey(L, func_id);
		lua_pushvalue(L, i);
		lua_settable(L, LUA_REGISTRYINDEX);
		return func_id++;
	}
};
