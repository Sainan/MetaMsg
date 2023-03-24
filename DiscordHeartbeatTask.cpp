#include "DiscordHeartbeatTask.hpp"

#include <soup/JsonNull.hpp>
#include <soup/JsonObject.hpp>
#include <soup/JsonString.hpp>

namespace MetaMsg
{
	void DiscordHeartbeatTask::onTick()
	{
		SOUP_IF_UNLIKELY (GET_MILLIS_SINCE(last_sent) > interval)
		{
			JsonObject obj;
			obj.add("op", 1);
			SOUP_IF_UNLIKELY (last_seq == 0) // Seq numbers begin at 1, so 0 means we have not received one yet.
			{
				obj.add("d", soup::make_unique<JsonNull>());
			}
			else
			{
				obj.add("d", (intptr_t)last_seq);
			}
			s->wsSend(obj.encode());

			last_sent = time::millis();
		}
	}
}
