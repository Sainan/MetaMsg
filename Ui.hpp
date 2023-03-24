#pragma once

#include <string>
#include <vector>

#include <soup/fwd.hpp>

#include "fwd.hpp"

namespace MetaMsg
{
	enum View : uint8_t
	{
		V_MESSENGER,
		V_GUILDS,
	};

	class Ui
	{
	public:
		unsigned int width = 0;
		unsigned int height;
		View view = V_MESSENGER;

		// Messenger View
		Guild* guild;

	public:
		void draw() const;
		void onNewMessage(const Channel& chan) const;
		void onNewChannel(const Guild& g) const;

	public:
		[[nodiscard]] bool isMessengerView() const noexcept;
		[[nodiscard]] bool isGuildActive(const Guild& g) const noexcept;
		[[nodiscard]] bool isChannelActive(const Channel& chan) const noexcept;
		void redrawDraft() const;
	protected:
		void drawMessengerView() const;
		void drawChannel(const Channel& chan, unsigned int x, unsigned int y) const;
		void drawMessage(const Message& msg, unsigned int x, unsigned int y, unsigned int xoff = 0) const;
		void drawDraft() const;
	
	public:
		[[nodiscard]] bool isGuildsView() const noexcept;
	protected:
		void drawGuildsView() const;

	public:
		void inthr_resize(unsigned int width, unsigned int height) const;
		void inthr_click(unsigned int x, unsigned int y) const;
		void inthr_char(char32_t c) const;
		void inthr_control(soup::ControlInput c) const;
	};
	inline Ui g_ui;
}
