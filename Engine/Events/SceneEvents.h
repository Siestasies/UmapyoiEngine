/*!
\file   SceneEvents.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Defines events related to scene lifecycle management within the engine.

Includes notifications for scene changes, successful loads, and unloads,
each carrying the associated scene name and assigned appropriate priority.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "EventType.h"

namespace Uma_Engine
{
    class SceneChangeEvent : public Event
    {
    public:
        SceneChangeEvent(const std::string& sceneName) : sceneName(sceneName) { priority = Priority::High; }

    public:
        std::string sceneName;
    };

    class SceneLoadedEvent : public Event
    {
    public:
        SceneLoadedEvent(const std::string& sceneName) : sceneName(sceneName) { priority = Priority::High; }

    public:
        std::string sceneName;
    };

    class SceneUnloadedEvent : public Event
    {
    public:
        SceneUnloadedEvent(const std::string& sceneName) : sceneName(sceneName) { priority = Priority::Normal; }

    public:
        std::string sceneName;
    };
}