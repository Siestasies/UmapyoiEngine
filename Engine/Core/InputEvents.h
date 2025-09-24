#pragma once

#include "EventType.h"

namespace Uma_Engine
{
    class KeyPressEvent : public Event
    {
    public:
        KeyPressEvent(int key, int action, int mods) : key(key), action(action), mods(mods) { priority = Priority::High; }

    public:
        int key, action, mods;
    };

    class KeyReleaseEvent : public Event
    {
    public:
        KeyReleaseEvent(int key, int mods) : key(key), mods(mods) { priority = Priority::High; }

    public:
        int key, mods;
    };

    class KeyRepeatEvent : public Event
    {
    public:
        KeyRepeatEvent(int key, int mods) : key(key), mods(mods) { priority = Priority::Normal; /*Repeats can be queued*/ }

    public:
        int key, mods;
    };

    class MouseButtonEvent : public Event
    {
    public:
        MouseButtonEvent(int button, int action, int mods, double x, double y) : button(button), action(action), mods(mods), x(x), y(y) { priority = Priority::High; }

    public:
        int button, action, mods;
        double x, y;
    };

    class MouseMoveEvent : public Event
    {
    public:
        MouseMoveEvent(double x, double y, double deltaX, double deltaY) : x(x), y(y), deltaX(deltaX), deltaY(deltaY) { priority = Priority::Normal; }

    public:
        double x, y, deltaX, deltaY;
    };

    class MouseScrollEvent : public Event
    {
    public:
        MouseScrollEvent(double xOffset, double yOffset) : xOffset(xOffset), yOffset(yOffset) { priority = Priority::Normal; }

    public:
        double xOffset, yOffset;
    };

    class TextInputEvent : public Event
    {
    public:
        TextInputEvent(unsigned int codepoint) : codepoint(codepoint) { priority = Priority::High; }

    public:
        unsigned int codepoint;
    };
}