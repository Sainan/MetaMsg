#pragma once

#include <deque>
#include <string>

#include "Message.hpp"

namespace MetaMsg
{
	struct Channel
	{
		std::string name;
		bool is_divider;
		std::deque<Message> messages;

		// UI
		std::u32string draft;
		unsigned int xoff = 0;
		unsigned int yoff = 0;

		Channel(std::string name, bool is_divider = false);

		void addMessage(Message&& msg);
	};
}
