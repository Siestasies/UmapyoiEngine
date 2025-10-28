/*!
\file   DebugEvents.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Defines a debug logging event for the Uma Engine event system.

This event encapsulates debug messages with an associated warning level,
allowing listeners to handle logs of varying severity. Critical level messages
are dispatched immediately with high priority, while others are queued normally.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include <string>
#include <vector>

#include "EventType.h"
#include "Debugging/Debugger.hpp"

namespace Uma_Engine
{
    class DebugLogEvent : public Event
    {
        public:
            DebugLogEvent(const std::string& message, WarningLevel level = WarningLevel::eInfo) : message(message), level(level) { priority = (level == WarningLevel::eCritical) ? Priority::Critical : Priority::Normal; }

        public:
            std::string message;
            WarningLevel level;
    };
}