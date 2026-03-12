/*!
\file   ECSEvents.h
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

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
        /*!
        \brief Constructs an EntityCreatedEvent.
        \param entityId The ID of the newly created entity.
        \param entityCnt The total entity count after creation.
        */
        EntityCreatedEvent(Uma_ECS::Entity entityId, int entityCnt) : entityId(entityId), entityCnt(entityCnt){ priority = Priority::Normal; /* Safe to queue */ }

    public:
        Uma_ECS::Entity entityId;
        int entityCnt;
    };

    class EntityDestroyedEvent : public Event
    {
    public:
        /*!
        \brief Constructs an EntityDestroyedEvent.
        \param entityId The ID of the destroyed entity.
        \param entityCnt The total entity count after destruction.
        */
        EntityDestroyedEvent(Uma_ECS::Entity entityId, int entityCnt) : entityId(entityId), entityCnt(entityCnt) { priority = Priority::High;}

    public:
        Uma_ECS::Entity entityId;
        int entityCnt;
    };

    class ComponentAddedEvent : public Event
    {
    public:
        /*!
        \brief Constructs a ComponentAddedEvent.
        \param entityId The entity that received the new component.
        \param componentType The type_index of the added component.
        */
        ComponentAddedEvent(Uma_ECS::Entity entityId, std::type_index componentType) : entityId(entityId), componentType(componentType) { priority = Priority::Normal; }

    public:
        Uma_ECS::Entity entityId;
        std::type_index componentType;
    };

    class ComponentRemovedEvent : public Event
    {
    public:
        /*!
        \brief Constructs a ComponentRemovedEvent.
        \param entityId The entity that had the component removed.
        \param componentType The type_index of the removed component.
        */
        ComponentRemovedEvent(Uma_ECS::Entity entityId, std::type_index componentType) : entityId(entityId), componentType(componentType) { priority = Priority::Normal; }

    public:
        Uma_ECS::Entity entityId;
        std::type_index componentType;
    };

    class EntityActiveStateChangedEvent : public Event
    {
    public:
        /*!
        \brief Constructs an EntityActiveStateChangedEvent.
        \param entityId The entity whose active state changed.
        \param isActive The new active state of the entity.
        */
        EntityActiveStateChangedEvent(Uma_ECS::Entity entityId, bool isActive) : entityId(entityId), isActive(isActive) { priority = Priority::Normal; }

    public:
        Uma_ECS::Entity entityId;
        bool isActive;
    };

    // Lua scripting system

    class EntityScriptActiveStateChangedEvent : public Event
    {
    public:
        /*!
        \brief Constructs an EntityScriptActiveStateChangedEvent.
        \param entityId The entity whose script active state changed.
        \param scriptIndex Index of the script within the entity's script list.
        \param state The new active state of the script.
        */
        EntityScriptActiveStateChangedEvent(Uma_ECS::Entity entityId, int scriptIndex, bool state)
            : entityId(entityId)
            , scriptIndex(scriptIndex)
            , isActive(state)
        {
            priority = Priority::High;
        }
    public:
        Uma_ECS::Entity entityId;
        int scriptIndex;
        bool isActive;
    };
}