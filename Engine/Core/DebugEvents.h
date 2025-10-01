#pragma once

#include <string>
#include <vector>

#include "EventType.h"
#include "Debugger.hpp"

namespace Uma_Engine
{
    struct DebugLogEvent : public Event
    {
    public:
        DebugLogEvent(const std::string& message, WarningLevel level = WarningLevel::eInfo) : message(message), level(level) { priority = (level == WarningLevel::eCritical) ? Priority::Critical : Priority::Normal; }

    public:
        std::string message;
        WarningLevel level;
    };
}