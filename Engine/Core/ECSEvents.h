#pragma once

#include "EventType.h"
#include "../ECS/Core/Types.hpp"

namespace Uma_Engine
{
    class EntityCreatedEvent : public Event
    {
    public:
        EntityCreatedEvent(Uma_ECS::Entity entityId) : entityId(entityId) { priority = Priority::Normal; /* Safe to queue */ }

    public:
        Uma_ECS::Entity entityId;
    };

    class EntityDestroyedEvent : public Event
    {
    public:
        EntityDestroyedEvent(Uma_ECS::Entity entityId) : entityId(entityId) { priority = Priority::High; }

    public:
        Uma_ECS::Entity entityId;
    };

    class ComponentAddedEvent : public Event
    {
    public:
        ComponentAddedEvent(Uma_ECS::Entity entityId, std::type_index componentType) : entityId(entityId), componentType(componentType) { priority = Priority::Normal; }

    public:
        Uma_ECS::Entity entityId;
        std::type_index componentType;
    };

    class ComponentRemovedEvent : public Event
    {
    public:
        ComponentRemovedEvent(Uma_ECS::Entity entityId, std::type_index componentType) : entityId(entityId), componentType(componentType) { priority = Priority::Normal; }

    public:
        Uma_ECS::Entity entityId;
        std::type_index componentType;
    };
}