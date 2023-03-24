#include "Guild.hpp"

#include "Ui.hpp"

namespace MetaMsg
{
	Channel* Guild::addChannel(UniquePtr<Channel>&& channel)
	{
		Channel* chan = internal_addChannel(std::move(channel));
		g_ui.onNewChannel(*this);
		return chan;
	}

	Channel* Guild::internal_addChannel(UniquePtr<Channel>&& channel)
	{
		Channel* chan = channels.emplace_back(std::move(channel)).get();
		if (active_channel == nullptr
			&& !chan->is_divider
			)
		{
			active_channel = chan;
		}
		return chan;
	}
}
