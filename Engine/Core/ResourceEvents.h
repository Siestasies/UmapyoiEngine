/*!
\file   ResourceEvents.h
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

namespace Uma_Engine
{
    class ResourceLoadedEvent : public Event
    {
    public:
        ResourceLoadedEvent(const std::string& resourcePath, const std::string& resourceType) : resourcePath(resourcePath), resourceType(resourceType) { priority = Priority::Normal; }

    public:
        std::string resourcePath;
        std::string resourceType;
    };

    class ResourceLoadFailedEvent : public Event
    {
    public:
        ResourceLoadFailedEvent(const std::string& resourcePath, const std::string& error)  : resourcePath(resourcePath), error(error) { priority = Priority::High; }

    public:
        std::string resourcePath;
        std::string error;
    };
}