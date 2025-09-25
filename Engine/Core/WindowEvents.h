#pragma once

#include "EventType.h"

namespace Uma_Engine
{
    class WindowResizeEvent : public Event
    {
    public:
        WindowResizeEvent(int width, int height) : width(width), height(height) { priority = Priority::High; }

    public:
        int width, height;
    };

    class WindowCloseEvent : public Event
    {
    public:
        WindowCloseEvent() { priority = Priority::Critical; }
    };

    class WindowFocusEvent : public Event
    {
    public:
        WindowFocusEvent(bool focused) : focused(focused) { priority = Priority::High; }

    public:
        bool focused;
    };

    class WindowMoveEvent : public Event
    {
    public:
        WindowMoveEvent(int x, int y) : x(x), y(y) { priority = Priority::Normal; }

    public:
        int x, y;
    };

    class FramebufferResizeEvent : public Event
    {
    public:
        FramebufferResizeEvent(int width, int height) : width(width), height(height) { priority = Priority::High; }

    public:
        int width, height;
    };
}