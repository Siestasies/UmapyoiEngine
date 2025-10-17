/*!
\file   EventType.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Base class for all events in the Uma Engine event system.

Defines a common interface for events, including a priority level for queued processing
and a mutable handled flag to allow listeners to stop event propagation.
All custom event types must inherit from this class to be compatible with the EventSystem.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once
#include <string>
#include <typeindex>

namespace Uma_Engine
{
    // Base Event class, all events must inherit from this
    class Event
    {
    public:
        virtual ~Event() = default;

    public:
        mutable bool handled = false;

        // Event priority for processing order
        enum class Priority
        {
            Low = 0,
            Normal = 1,
            High = 2,
            Critical = 3
        };

        Priority priority = Priority::Normal;
    };
}