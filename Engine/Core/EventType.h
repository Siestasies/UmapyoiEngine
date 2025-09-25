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
        bool handled = false;

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