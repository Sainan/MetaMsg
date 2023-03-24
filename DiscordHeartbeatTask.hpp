#pragma once

#include <soup/Task.hpp>

#include <soup/SharedPtr.hpp>
#include <soup/WebSocketConnection.hpp>

#include "time.hpp"

namespace MetaMsg
{
	using namespace soup;

	struct DiscordHeartbeatTask : public Task
	{
		SharedPtr<WebSocketConnection> s;
		time_t interval; // From Hello Event
		time_t last_sent = 0;
		size_t last_seq = 0; // Last received non-null "s"

		DiscordHeartbeatTask(SharedPtr<WebSocketConnection>&& s, time_t interval)
			: s(std::move(s)), interval(interval)
		{
		}

		void onTick() final;
	};
}
