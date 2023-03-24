#pragma once

#include "Service.hpp"

namespace MetaMsg
{
	struct ServiceMeta : public Service
	{
		Guild* system_guild;
		Channel* log_channel;

		ServiceMeta();

		void submitMessage(Guild*, Channel*, std::string&& message) final;
	};
}
