#include "Service.hpp"

#include "Ui.hpp"

namespace MetaMsg
{
	Guild* Service::addGuild(UniquePtr<Guild>&& guild)
	{
		Guild* g = internal_addGuild(std::move(guild));
		if (g_ui.isGuildsView())
		{
			g_ui.draw();
		}
		return g;
	}

	Guild* Service::internal_addGuild(UniquePtr<Guild>&& guild)
	{
		return guilds.emplace_back(std::move(guild)).get();
	}
}
