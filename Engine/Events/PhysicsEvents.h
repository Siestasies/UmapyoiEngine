/*!
\file   PhysicsEvents.h
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
    class CollisionBeginEvent : public Event
    {
    public:
        CollisionBeginEvent(Uma_ECS::Entity entityA, Uma_ECS::Entity entityB) : entityA(entityA), entityB(entityB) { priority = Priority::Normal; }

    public:
        Uma_ECS::Entity entityA, entityB;
    };

    class CollisionEndEvent : public Event
    {
    public:
        CollisionEndEvent(Uma_ECS::Entity entityA, Uma_ECS::Entity entityB) : entityA(entityA), entityB(entityB) { priority = Priority::Normal; }

    public:
        Uma_ECS::Entity entityA, entityB;
    };

    class TriggerEnterEvent : public Event
    {
    public:
        TriggerEnterEvent(Uma_ECS::Entity trigger, Uma_ECS::Entity entity) : trigger(trigger), entity(entity) { priority = Priority::Normal; }

    public:
        Uma_ECS::Entity trigger, entity;
    };

    class TriggerExitEvent : public Event
    {
    public:
        TriggerExitEvent(Uma_ECS::Entity trigger, Uma_ECS::Entity entity) : trigger(trigger), entity(entity) { priority = Priority::Normal; }

    public:
        Uma_ECS::Entity trigger, entity;
    };
}