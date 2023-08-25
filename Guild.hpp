#pragma once

#include <string>
#include <vector>

#include <soup/UniquePtr.hpp>

#include "fwd.hpp"
#include "Channel.hpp"

namespace MetaMsg
{
	using namespace soup;

	struct Guild
	{
		Service* const service;
		const std::string name;
		std::vector<UniquePtr<Channel>> channels{}; // do not manually modify

		// UI
		Channel* active_channel = nullptr;
		std::string username = "User";

		Guild(Service* service, std::string name)
			: service(service), name(std::move(name))
		{
		}

		[[nodiscard]] Channel* findChannelByName(const std::string& name) const noexcept
		{
			for (const auto& chan : channels)
			{
				if (chan->name == name)
				{
					return chan.get();
				}
			}
			return nullptr;
		}

		Channel* addChannel(UniquePtr<Channel>&& channel);
		Channel* internal_addChannel(UniquePtr<Channel>&& channel);
	};
}
