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