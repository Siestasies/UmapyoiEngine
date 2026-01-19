/*!
\file   LuaScriptingEvents.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

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
        ButtonOnClickedEvent(Uma_ECS::Entity entity, size_t scriptIndex)
            : entity(entity), scriptIndex(scriptIndex)
        {
            priority = Priority::Normal;
        }

    public:
        Uma_ECS::Entity entity;
        size_t scriptIndex;
    };

    // cant pass in parameter yet
    class CallLuaFunction : public Event
    {
    public:
        CallLuaFunction(Entity entity, std::string scriptName, std::string functionName)
            : entity(entity)
            , scriptName(scriptName)
            , functionName(functionName)
        {
            priority = Priority::Normal;
        }
    public:
        Entity entity;
        std::string scriptName;
        std::string functionName;
        
    };
}