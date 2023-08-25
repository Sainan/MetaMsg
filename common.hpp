#pragma once

#include <vector>

#include <soup/Scheduler.hpp>

#include "fwd.hpp"
#include "Service.hpp"

namespace MetaMsg
{
	using namespace soup;

	inline Scheduler g_sched;
	inline std::vector<UniquePtr<Service>> g_services;
	inline ServiceMeta* g_meta;

	[[nodiscard]] inline Service* findServiceByName(const std::string& name) noexcept
	{
		for (const auto& serv : g_services)
		{
			if (serv->name == name)
			{
				return serv.get();
			}
		}
		return nullptr;
	}
}
