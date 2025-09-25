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