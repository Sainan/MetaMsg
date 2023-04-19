#include "HttpRequestWrapperTask.hpp"

#include "common.hpp"
#include "Message.hpp"

namespace MetaMsg
{
	HttpRequestWrapperTask::HttpRequestWrapperTask(Channel* log_channel, HttpRequest&& hr)
		: Task(), log_channel(log_channel), hrt(std::move(hr))
	{
	}

	void HttpRequestWrapperTask::onTick()
	{
		if (hrt.tickUntilDone())
		{
			log_channel->addMessage(Message{ "HTTP", hrt.res->body });
			setWorkDone();
		}
	}
}
