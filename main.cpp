#include <soup/console.hpp>
#include <soup/DummyTask.hpp>
#include <soup/format.hpp>
#include <soup/log.hpp>
#include <soup/MouseButton.hpp>
#include <soup/Scheduler.hpp>
#include <soup/Socket.hpp>
#include <soup/Thread.hpp>

#include "common.hpp"
#include "PluginMgr.hpp"
#include "ServiceMeta.hpp"
#include "Ui.hpp"

using namespace MetaMsg;
using namespace soup;

struct LogSink : public logSink
{
	void write(std::string&& message) final
	{
		g_meta->log_channel->addMessage(Message{ "LogSink", std::move(message) });
	}
};

int main()
{
	g_meta = static_cast<ServiceMeta*>(g_services.emplace_back(soup::make_unique<ServiceMeta>()).get());
	g_meta->log_channel->messages = {
		{ "MetaMsg", "Welcome to MetaMsg!" },
		{ "MetaMsg", "To add Discord, use /discord <token>" },
		{ "MetaMsg", "To quit the program, use /quit" },
		{ "MetaMsg", "Note: These commands are only available in this guild." },
	};

	logSetSink(soup::make_unique<LogSink>());

	g_ui.guild = g_meta->system_guild;

	g_sched.on_work_done = [](soup::Worker& w, soup::Scheduler&)
	{
		if (w.type == WORKER_TYPE_SOCKET)
		{
			g_meta->log_channel->addMessage({ "Scheduler", format("{} - work done", w.toString()) });
		}
	};
	g_sched.on_connection_lost = [](soup::Socket& s, soup::Scheduler&)
	{
		if (s.remote_closed)
		{
			g_meta->log_channel->addMessage({ "Scheduler", format("{} - connection lost", s.toString()) });
		}
		else
		{
			g_meta->log_channel->addMessage({ "Scheduler", format("{} - connection closed", s.toString()) });
		}
	};
	g_sched.on_exception = [](soup::Worker& w, const std::exception& e, soup::Scheduler&)
	{
		g_meta->log_channel->addMessage({ "Scheduler", format("{} - exception: {}", w.toString(), e.what()) });
	};

	console.init(true);
	Thread input_thrd([](Capture&&)
	{
		console.enableSizeTracking([](unsigned int width, unsigned int height, const Capture&)
		{
			g_ui.inthr_resize(width, height);
		});
		console.onMouseClick([](MouseButton btn, unsigned int x, unsigned int y, const Capture&)
		{
			if (btn == LMB)
			{
				g_ui.inthr_click(x, y);
			}
		});
		console.char_handler.fp = [](char32_t c, const Capture&)
		{
			g_ui.inthr_char(c);
		};
		console.control_handler.fp = [](ControlInput c, const Capture&)
		{
			g_ui.inthr_control(c);
		};
		console.run();
	});

	g_sched.add<DummyTask>();
	g_sched.run();
}

void onPostInitialDraw()
{
	PluginMgr::init();
}
