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