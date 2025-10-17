/*!
\file   ECSEvents.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Defines ECS lifecycle event types related to entity and component changes.

This file contains event classes for entity creation and destruction, as well as component
addition and removal within the ECS framework. Each event carries essential data such as
entity identifiers and component type information, with priority levels assigned to guide
their processing order in the event system.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "EventType.h"
#include "../ECS/Core/Types.hpp"

namespace Uma_Engine
{
    class EntityCreatedEvent : public Event
    {
    public:
        EntityCreatedEvent(Uma_ECS::Entity entityId, int entityCnt) : entityId(entityId), entityCnt(entityCnt){ priority = Priority::Normal; /* Safe to queue */ }

    public:
        Uma_ECS::Entity entityId;
        int entityCnt;
    };

    class EntityDestroyedEvent : public Event
    {
    public:
        EntityDestroyedEvent(Uma_ECS::Entity entityId, int entityCnt) : entityId(entityId), entityCnt(entityCnt) { priority = Priority::High;}

    public:
        Uma_ECS::Entity entityId;
        int entityCnt;
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