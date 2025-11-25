/*!
\file   UIToLuaEvents.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Defines events related to resource loading status within the engine.

Includes notifications for successful resource loads and failures,
carrying relevant resource identifiers and error details.
Priorities are set to ensure proper handling of load failures.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "EventType.h"
#include "ECS/Core/Types.hpp"

namespace Uma_Engine
{
    class ButtonOnClickedEvent : public Event
    {
    public:
        ButtonOnClickedEvent(const Uma_ECS::Entity& entity, size_t script_index) : en(entity), scriptIndex(script_index) { priority = Priority::Normal; }

    public:
        Uma_ECS::Entity en;
        size_t scriptIndex;
    };

    
}