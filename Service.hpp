#pragma once

#include <vector>

#include <soup/UniquePtr.hpp>

#include "Guild.hpp"

namespace MetaMsg
{
	using namespace soup;

	struct Service
	{
		const char* const name;
		std::vector<UniquePtr<Guild>> guilds{};

		explicit Service(const char* name)
			: name(name)
		{
		}

		virtual ~Service() = default;

		[[nodiscard]] Guild* findGuildByName(const std::string& name) const noexcept
		{
			for (const auto& guild : guilds)
			{
				if (guild->name == name)
				{
					return guild.get();
				}
			}
			return nullptr;
		}

		Guild* addGuild(UniquePtr<Guild>&& guild);
		Guild* internal_addGuild(UniquePtr<Guild>&& guild);

		virtual void submitMessage(Guild* g, Channel* chan, std::string&& message) = 0;
	};
}
