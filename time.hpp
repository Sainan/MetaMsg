#pragma once

#include <soup/time.hpp>

#define GET_MILLIS_SINCE(since) (::soup::time::millis() - (since))
#define IS_DEADLINE_REACHED(deadline) (::soup::time::millis() >= deadline)
