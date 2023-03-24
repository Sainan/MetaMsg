#include "Channel.hpp"

#include <soup/string.hpp>

#include "Ui.hpp"

namespace MetaMsg
{
	Channel::Channel(std::string name, bool is_divider)
		: name(std::move(name)), is_divider(is_divider)
	{
		if (is_divider)
		{
			soup::string::upper(this->name);
		}
	}

	void Channel::addMessage(Message&& msg)
	{
		messages.emplace_back(std::move(msg));
		g_ui.onNewMessage(*this);
	}
}
