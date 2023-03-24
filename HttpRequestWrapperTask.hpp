#pragma once

#include <soup/HttpRequestTask.hpp>

#include "fwd.hpp"

namespace MetaMsg
{
	using namespace soup;

	struct HttpRequestWrapperTask : public Task
	{
		Channel* log_channel;
		HttpRequestTask hrt;

		HttpRequestWrapperTask(Channel* log_channel, HttpRequest&& hr);

		void onTick() final;
	};
}
