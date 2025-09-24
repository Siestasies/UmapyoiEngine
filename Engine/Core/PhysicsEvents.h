#pragma once

#include "EventType.h"

namespace Uma_Engine
{
    class CollisionBeginEvent : public Event
    {
    public:
        CollisionBeginEvent(uint32_t entityA, uint32_t entityB) : entityA(entityA), entityB(entityB) { priority = Priority::Normal; }

    public:
        uint32_t entityA, entityB;
    };

    class CollisionEndEvent : public Event
    {
    public:
        CollisionEndEvent(uint32_t entityA, uint32_t entityB) : entityA(entityA), entityB(entityB) { priority = Priority::Normal; }

    public:
        uint32_t entityA, entityB;
    };

    class TriggerEnterEvent : public Event
    {
    public:
        TriggerEnterEvent(uint32_t trigger, uint32_t entity) : trigger(trigger), entity(entity) { priority = Priority::Normal; }

    public:
        uint32_t trigger, entity;
    };

    class TriggerExitEvent : public Event
    {
    public:
        TriggerExitEvent(uint32_t trigger, uint32_t entity) : trigger(trigger), entity(entity) { priority = Priority::Normal; }

    public:
        uint32_t trigger, entity;
    };
}