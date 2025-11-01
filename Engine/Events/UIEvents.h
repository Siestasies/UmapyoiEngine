#pragma once

#include "EventType.h"
#include "../../Engine/ECS/Core/Types.hpp"
#include "Math/Math.h"

namespace Uma_Engine
{
    /**
     * \struct PointerClickEvent
     * \brief Emitted when a UI element is clicked
     */
    struct PointerClickEvent : public Event
    {
    public:
        PointerClickEvent(Uma_ECS::Entity entity, const Vec2& pos) : targetEntity(entity), screenPosition(pos)
        {
            priority = Priority::High;
        }

    public:
        Uma_ECS::Entity targetEntity;
        Vec2 screenPosition;
    };

    /**
     * \struct PointerEnterEvent
     * \brief Emitted when pointer enters a UI element's bounds
     */
    struct PointerEnterEvent : public Event
    {
    public:
        PointerEnterEvent(Uma_ECS::Entity entity, const Vec2& pos) : targetEntity(entity), screenPosition(pos)
        {
            priority = Priority::Normal;
        }

    public:
        Uma_ECS::Entity targetEntity;
        Vec2 screenPosition;
    };

    /**
     * \struct PointerExitEvent
     * \brief Emitted when pointer exits a UI element's bounds
     */
    struct PointerExitEvent : public Event
    {
    public:
        PointerExitEvent(Uma_ECS::Entity entity, const Vec2& pos) : targetEntity(entity), screenPosition(pos)
        {
            priority = Priority::Normal;
        }

    public:
        Uma_ECS::Entity targetEntity;
        Vec2 screenPosition;
    };

    /**
     * \struct PointerDownEvent
     * \brief Emitted when mouse button is pressed over UI element
     */
    struct PointerDownEvent : public Event
    {
    public:
        PointerDownEvent(Uma_ECS::Entity entity, const Vec2& pos) : targetEntity(entity), screenPosition(pos)
        {
            priority = Priority::High;
        }

    public:
        Uma_ECS::Entity targetEntity;
        Vec2 screenPosition;
    };

    /**
     * \struct PointerUpEvent
     * \brief Emitted when mouse button is released over UI element
     */
    struct PointerUpEvent : public Event
    {
        Uma_ECS::Entity targetEntity;
        Vec2 screenPosition;

        PointerUpEvent(Uma_ECS::Entity entity, const Vec2& pos)
            : targetEntity(entity), screenPosition(pos)
        {
            priority = Priority::High;
        }
    };
}