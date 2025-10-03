/*!
\file   WindowEvents.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Defines window and framebuffer event types for the engine's event system.

Includes notifications for window resizing, closing, focus changes,
movement, and framebuffer resizing, each with appropriate event priority.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

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