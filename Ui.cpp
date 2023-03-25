#include "Ui.hpp"

#include <soup/console.hpp>
#include <soup/ControlInput.hpp>
#include <soup/Task.hpp>
#include <soup/unicode.hpp>

#include "common.hpp"

namespace MetaMsg
{
	using namespace soup;

	void Ui::draw() const
	{
		SOUP_ASSERT(width != 0);
		if (isMessengerView())
		{
			drawMessengerView();
		}
		else
		{
			drawGuildsView();
		}
		std::cout << std::flush;
	}

	bool Ui::isMessengerView() const noexcept
	{
		return view == V_MESSENGER;
	}

	bool Ui::isGuildActive(const Guild& g) const noexcept
	{
		return guild == &g;
	}

	bool Ui::isChannelActive(const Channel& chan) const noexcept
	{
		return guild->active_channel == &chan;
	}

	void Ui::redrawDraft() const
	{
		// Paint over with black
		console.setCursorPos(21, height - 1);
		console.setBackgroundColour(0x00, 0x00, 0x00);
		console.setForegroundColour(0x00, 0x00, 0x00);
		for (unsigned int x = 21; x < width; ++x)
		{
			console << "-";
		}

		drawDraft();

		std::cout << std::flush;
	}

	void Ui::drawMessengerView() const
	{
		console.fillScreen(0x00, 0x00, 0x00); // also sets background colour
		console.setForegroundColour(0xFF, 0xFF, 0xFF);

		// Guild Name
		console.setCursorPos(0, 0);
		console << guild->name.substr(0, 19);
		if (guild->name.size() < 19)
		{
			console << std::string(19 - guild->name.size(), ' ');
		}
		console << "▾";

		// Channels
		for (unsigned int i = 0; i != guild->channels.size(); ++i)
		{
			drawChannel(*guild->channels.at(i), 0, i + 2);
		}

		// Divider
		console.setBackgroundColour(0x10, 0x10, 0x10);
		console.setForegroundColour(0x10, 0x10, 0x10);
		for (unsigned int y = 0; y != height; ++y)
		{
			console.setCursorPos(20, y);
			console << "|";
		}

		// Messages
		console.setBackgroundColour(0x00, 0x00, 0x00);
		unsigned int i = 0;
		unsigned int yoff = 2;
		if (guild->active_channel)
		{
			for (auto it = guild->active_channel->messages.rbegin(); it != guild->active_channel->messages.rend(); ++it)
			{
				if (i >= guild->active_channel->yoff)
				{
					drawMessage(*it, 21, height - yoff, guild->active_channel->xoff);
					++yoff;
					if (yoff > height)
					{
						break;
					}
				}
				++i;
			}
		}

		drawDraft();
	}

	void Ui::onNewMessage(const Channel& chan) const
	{
		if (isMessengerView() && isChannelActive(chan))
		{
			draw();
		}
	}

	void Ui::onNewChannel(const Guild& g) const
	{
		if (isMessengerView() && isGuildActive(g))
		{
			draw();
		}
	}

	void Ui::drawChannel(const Channel& chan, unsigned int x, unsigned int y) const
	{
		console.setCursorPos(x, y);
		if (isChannelActive(chan))
		{
			console.setBackgroundColour(0x20, 0x20, 0x20);
			console << chan.name.substr(0, 20);
			console.setForegroundColour(0x20, 0x20, 0x20);
			if (chan.name.size() < 20)
			{
				console << std::string(20 - chan.name.size(), '-');
			}
		}
		else
		{
			console.setBackgroundColour(0x00, 0x00, 0x00);
			console.setForegroundColour(0xFF, 0xFF, 0xFF);
			console << chan.name.substr(0, 20);
		}
	}

	void Ui::drawMessage(const Message& msg, unsigned int x, unsigned int y, unsigned int xoff) const
	{
		console.setCursorPos(x, y);
		console.setForegroundColour(0xFF, 0xFF, 0xFF);
		console << msg.author;
		console.setForegroundColour(0xCC, 0xCC, 0xCC);
		console << "  ";
		const auto max_draw_width = (width - 21 - 2 - msg.author.size());
		if (xoff == -1)
		{
			if (msg.contents.size() > (max_draw_width - 1))
			{
				xoff = msg.contents.size() - (max_draw_width - 1);
			}
			else
			{
				xoff = 0;
			}
		}
		if (msg.contents.size() > xoff)
		{
			console << msg.contents.substr(xoff, max_draw_width);
		}
	}

	void Ui::drawDraft() const
	{
		if (guild->active_channel)
		{
			drawMessage(Message{ guild->username, unicode::utf32_to_utf8(guild->active_channel->draft) }, 21, height - 1, -1);
		}
	}

	bool Ui::isGuildsView() const noexcept
	{
		return view == V_GUILDS;
	}

	void Ui::drawGuildsView() const
	{
		console.fillScreen(0x00, 0x00, 0x00); // also sets background colour
		console.setForegroundColour(0xFF, 0xFF, 0xFF);

		console.setCursorPos(0, 0);
		for (const auto& serv : g_services)
		{
			console << serv->name << "\n";
			for (const auto& guild : serv->guilds)
			{
				console << "• " << guild->name << "\n";
			}
			console << "\n";
		}
	}

	struct ResizeTask : public Task
	{
		unsigned int width, height;

		ResizeTask(unsigned int width, unsigned int height)
			: width(width), height(height)
		{
		}

		void onTick() final
		{
			g_ui.width = width;
			g_ui.height = height;
			g_ui.draw();
			setWorkDone();
		}
	};

	void Ui::inthr_resize(unsigned int width, unsigned int height) const
	{
		g_sched.add<ResizeTask>(width, height);
	}

	struct ClickTask : public Task
	{
		unsigned int x, y;

		ClickTask(unsigned int x, unsigned int y)
			: x(x), y(y)
		{
		}

		void onTick() final
		{
			process();
			setWorkDone();
		}

		void process()
		{
			if (g_ui.isMessengerView())
			{
				if (x < 20) // Left side?
				{
					if (y == 0) // Guild switcher?
					{
						g_ui.view = V_GUILDS;
						g_ui.draw();
						return;
					}
					for (unsigned int i = 0; i != g_ui.guild->channels.size(); ++i)
					{
						if (y == (i + 2))
						{
							if (!g_ui.guild->channels.at(i)->is_divider)
							{
								if (!g_ui.isChannelActive(*g_ui.guild->channels.at(i)))
								{
									g_ui.guild->active_channel = g_ui.guild->channels.at(i);
									g_ui.draw();
								}
							}
							else
							{
								console.bell();
							}
							return;
						}
					}
				}
			}
			else
			{
				unsigned int _y = 0;
				for (const auto& serv : g_services)
				{
					++_y;
					for (const auto& guild : serv->guilds)
					{
						if (_y == y)
						{
							g_ui.view = V_MESSENGER;
							g_ui.guild = guild.get();
							g_ui.draw();
							return;
						}
						++_y;
					}
					++_y;
				}
			}
		}
	};

	void Ui::inthr_click(unsigned int x, unsigned int y) const
	{
		g_sched.add<ClickTask>(x, y);
	}

	struct CharTask : public Task
	{
		char32_t c;

		CharTask(char32_t c)
			: c(c)
		{
		}

		void onTick() final
		{
			if (g_ui.isMessengerView() && g_ui.guild->active_channel)
			{
				g_ui.guild->active_channel->draft.push_back(c);
				g_ui.redrawDraft();
			}
			setWorkDone();
		}
	};

	void Ui::inthr_char(char32_t c) const
	{
		g_sched.add<CharTask>(c);
	}

	struct ControlTask : public Task
	{
		soup::ControlInput c;

		ControlTask(soup::ControlInput c)
			: c(c)
		{
		}

		void onTick() final
		{
			if (g_ui.isMessengerView() && g_ui.guild->active_channel)
			{
				if (c == soup::UP)
				{
					++g_ui.guild->active_channel->yoff;
					g_ui.draw();
				}
				else if (c == soup::DOWN)
				{
					if (g_ui.guild->active_channel->yoff != 0)
					{
						--g_ui.guild->active_channel->yoff;
						g_ui.draw();
					}
					else
					{
						console.bell();
					}
				}
				else if (c == soup::RIGHT)
				{
					++g_ui.guild->active_channel->xoff;
					g_ui.draw();
				}
				else if (c == soup::LEFT)
				{
					if (g_ui.guild->active_channel->xoff != 0)
					{
						--g_ui.guild->active_channel->xoff;
						g_ui.draw();
					}
					else
					{
						console.bell();
					}
				}
				else if (c == soup::NEW_LINE)
				{
					std::string msg = unicode::utf32_to_utf8(g_ui.guild->active_channel->draft);
					g_ui.guild->active_channel->draft.clear();
					g_ui.redrawDraft();
					g_ui.guild->service->submitMessage(g_ui.guild, g_ui.guild->active_channel, std::move(msg));
				}
				else if (c == soup::BACKSPACE)
				{
					if (!g_ui.guild->active_channel->draft.empty())
					{
						g_ui.guild->active_channel->draft.pop_back();
						g_ui.redrawDraft();
					}
					else
					{
						console.bell();
					}
				}
			}
			setWorkDone();
		}
	};

	void Ui::inthr_control(soup::ControlInput c) const
	{
		g_sched.add<ControlTask>(c);
	}
}
