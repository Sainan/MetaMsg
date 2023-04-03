#include "DiscordGuild.hpp"

#include <algorithm>

#include <soup/json.hpp>

#include "DiscordChannel.hpp"

namespace MetaMsg
{
	void DiscordGuild::addChannel(const soup::JsonObject& chan)
	{
		if (snowflake_map.find(chan.at("id").asStr()) != snowflake_map.end())
		{
			// We may receive DM channels multiple times.
			return;
		}

		std::string name{};
		if (chan.contains("name"))
		{
			name = chan.at("name").asStr();
		}
		else
		{
			name = chan.at("recipients").asArr().at(0).asObj().at("username").asStr();
		}

		DiscordChannel* c = (DiscordChannel*)this->addChannel(soup::make_unique<DiscordChannel>(
			std::move(name),
			chan.at("id").asStr(),
			chan.at("type").asInt()
		));
		if (chan.contains("position"))
		{
			c->position = chan.at("position").asInt();
		}
		if (chan.contains("parent_id")
			&& chan.at("parent_id").isStr() // Sometimes it's null for some reason...
			)
		{
			c->parent = chan.at("parent_id").asStr();
		}
		this->snowflake_map.emplace(
			c->id,
			c
		);
	}

	void DiscordGuild::sortChannels()
	{
		std::sort(this->channels.begin(), this->channels.end(), [this](const UniquePtr<Channel>& _a, const UniquePtr<Channel>& _b)
		{
			DiscordChannel& a = *static_cast<DiscordChannel*>(_a.get());
			DiscordChannel& b = *static_cast<DiscordChannel*>(_b.get());

			// Get parent positions
			int a_parent_pos = -1;
			int b_parent_pos = -1;
			if (!a.parent.empty())
			{
				a_parent_pos = this->snowflake_map.at(a.parent)->position;
			}
			if (!b.parent.empty())
			{
				b_parent_pos = this->snowflake_map.at(b.parent)->position;
			}

			// Get group
			int a_group = a_parent_pos;
			int b_group = b_parent_pos;
			if (a.type == 4)
			{
				a_group = a.position;
			}
			if (b.type == 4)
			{
				b_group = b.position;
			}

			// Compare group
			if (a_group < b_group)
			{
				return true;
			}
			if (b_group < a_group)
			{
				return false;
			}

			// Demote voice channels
			if (a.type != 2
				&& b.type == 2
				)
			{
				return true;
			}
			if (b.type != 2
				&& a.type == 2
				)
			{
				return false;
			}

			return a.position < b.position;
		});
	}
}
