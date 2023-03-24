#include "ServiceMeta.hpp"

namespace MetaMsg
{
	ServiceMeta::ServiceMeta()
		: Service("METAMSG")
	{
		system_guild = internal_addGuild(soup::make_unique<Guild>("System Guild"));
		log_channel = system_guild->internal_addChannel(soup::make_unique<Channel>("log"));
	}
}
