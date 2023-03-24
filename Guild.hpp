#pragma once

#include <string>
#include <vector>

#include <soup/UniquePtr.hpp>

#include "Channel.hpp"

namespace MetaMsg
{
	using namespace soup;

	struct Guild
	{
		const std::string name;
		std::vector<UniquePtr<Channel>> channels{}; // do not manually modify

		// UI
		Channel* active_channel = nullptr;
		std::string username = "User";

		Guild(std::string name)
			: name(std::move(name))
		{
		}

		Channel* addChannel(UniquePtr<Channel>&& channel);
		Channel* internal_addChannel(UniquePtr<Channel>&& channel);
	};
}
