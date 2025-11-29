#pragma once
/*!
\file   PointerEvents.hpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Declares a set of UI pointer-related event structs used by the engine’s
event system. These events represent pointer interactions with UI elements,
including clicking, entering, exiting, button press, and button release.

Each event stores the target UI entity and the pointer's screen position.
Events also initialize their dispatch priority level upon construction.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "EventType.h"
#include "../../Engine/ECS/Core/Types.hpp"
#include "Math/Math.h"

namespace Uma_Engine
{
    /**
     * \struct PointerClickEvent
     * \brief Emitted when a UI element is clicked.
     *
     * \param entity The UI entity receiving the click.
     * \param pos The pointer's screen-space coordinates.
     */
    struct PointerClickEvent : public Event
    {
    public:
        PointerClickEvent(Uma_ECS::Entity entity, const Vec2& pos)
            : targetEntity(entity), screenPosition(pos)
        {
            priority = Priority::High;
        }

    public:
        Uma_ECS::Entity targetEntity;   //!< Entity clicked by the pointer
        Vec2 screenPosition;            //!< Screen position at click time
    };

    /**
     * \struct PointerEnterEvent
     * \brief Emitted when the pointer enters a UI element's bounds.
     *
     * \param entity The entity whose bounds were entered.
     * \param pos The pointer's screen-space coordinates.
     */
    struct PointerEnterEvent : public Event
    {
    public:
        PointerEnterEvent(Uma_ECS::Entity entity, const Vec2& pos)
            : targetEntity(entity), screenPosition(pos)
        {
            priority = Priority::Normal;
        }

    public:
        Uma_ECS::Entity targetEntity;   //!< Entity whose bounds were entered
        Vec2 screenPosition;            //!< Pointer position at entry
    };

    /**
     * \struct PointerExitEvent
     * \brief Emitted when the pointer exits a UI element's bounds.
     *
     * \param entity The entity whose bounds were exited.
     * \param pos The pointer's screen-space coordinates.
     */
    struct PointerExitEvent : public Event
    {
    public:
        PointerExitEvent(Uma_ECS::Entity entity, const Vec2& pos)
            : targetEntity(entity), screenPosition(pos)
        {
            priority = Priority::Normal;
        }

    public:
        Uma_ECS::Entity targetEntity;   //!< Entity whose bounds were exited
        Vec2 screenPosition;            //!< Pointer position at exit
    };

    /**
     * \struct PointerDownEvent
     * \brief Emitted when a mouse button is pressed over a UI element.
     *
     * \param entity The entity pressed on.
     * \param pos The pointer's screen-space coordinates.
     */
    struct PointerDownEvent : public Event
    {
    public:
        PointerDownEvent(Uma_ECS::Entity entity, const Vec2& pos)
            : targetEntity(entity), screenPosition(pos)
        {
            priority = Priority::High;
        }

    public:
        Uma_ECS::Entity targetEntity;   //!< Entity pressed by pointer
        Vec2 screenPosition;            //!< Pointer position at press time
    };

    /**
     * \struct PointerUpEvent
     * \brief Emitted when a mouse button is released over a UI element.
     *
     * \param entity The entity released over.
     * \param pos The pointer's screen-space coordinates.
     */
    struct PointerUpEvent : public Event
    {
    public:
        PointerUpEvent(Uma_ECS::Entity entity, const Vec2& pos)
            : targetEntity(entity), screenPosition(pos)
        {
            priority = Priority::High;
        }

    public:
        Uma_ECS::Entity targetEntity;   //!< Entity released over
        Vec2 screenPosition;            //!< Pointer position at release
    };
}
