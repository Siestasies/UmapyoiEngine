/*!
\file   CollisionEvent.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Defines physics-related events for collision and trigger interactions in the ECS.

Includes events signaling the beginning and end of collisions between entities,
as well as when entities enter or exit trigger volumes. Each event carries the
relevant entity IDs and uses normal priority for standard processing order.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "EventType.h"
#include "../ECS/Core/Types.hpp"

namespace Uma_Engine
{
    /**
     * @brief Event fired when two entities start colliding (Unity: OnCollisionEnter)
     */
    class OnCollisionEnterEvent : public Event
    {
    public:
        OnCollisionEnterEvent(Uma_ECS::Entity entityA, Uma_ECS::Entity entityB)
            : entityA(entityA)
            , entityB(entityB)
        {
            priority = Priority::Normal;
        }

        Uma_ECS::Entity entityA;
        Uma_ECS::Entity entityB;
    };

    /**
     * @brief Event fired continuously while two entities are colliding (Unity: OnCollisionStay)
     */
    class OnCollisionEvent : public Event
    {
    public:
        OnCollisionEvent(Uma_ECS::Entity entityA, Uma_ECS::Entity entityB)
            : entityA(entityA)
            , entityB(entityB)
        {
            priority = Priority::Normal;
        }

        Uma_ECS::Entity entityA;
        Uma_ECS::Entity entityB;
    };

    /**
     * @brief Event fired when two entities stop colliding (Unity: OnCollisionExit)
     */
    class OnCollisionExitEvent : public Event
    {
    public:
        OnCollisionExitEvent(Uma_ECS::Entity entityA, Uma_ECS::Entity entityB)
            : entityA(entityA)
            , entityB(entityB)
        {
            priority = Priority::Normal;
        }

        Uma_ECS::Entity entityA;
        Uma_ECS::Entity entityB;
    };

    /**
     * @brief Event fired when an entity enters a trigger volume (Unity: OnTriggerEnter)
     */
    class OnTriggerEnterEvent : public Event
    {
    public:
        OnTriggerEnterEvent(Uma_ECS::Entity trigger, Uma_ECS::Entity entity)
            : trigger(trigger)
            , entity(entity)
        {
            priority = Priority::Normal;
        }

        Uma_ECS::Entity trigger;
        Uma_ECS::Entity entity;
    };

    /**
     * @brief Event fired continuously while an entity is inside a trigger volume (Unity: OnTriggerStay)
     */
    class OnTriggerEvent : public Event
    {
    public:
        OnTriggerEvent(Uma_ECS::Entity trigger, Uma_ECS::Entity entity)
            : trigger(trigger)
            , entity(entity)
        {
            priority = Priority::Normal;
        }

        Uma_ECS::Entity trigger;
        Uma_ECS::Entity entity;
    };

    /**
     * @brief Event fired when an entity exits a trigger volume (Unity: OnTriggerExit)
     */
    class OnTriggerExitEvent : public Event
    {
    public:
        OnTriggerExitEvent(Uma_ECS::Entity trigger, Uma_ECS::Entity entity)
            : trigger(trigger)
            , entity(entity)
        {
            priority = Priority::Normal;
        }

        Uma_ECS::Entity trigger;
        Uma_ECS::Entity entity;
    };
}