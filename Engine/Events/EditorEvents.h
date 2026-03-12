/*!
\file   EditorEvents.h
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Defines editor-related event types for entity picking, dropping, mode changes,
transform manipulation via gizmos, and mouse-over-UI state updates.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "../../ECS/Core/Types.hpp"
#include "../../Core/EventSystem.h"

namespace Uma_Engine
{
    /**
     * \class EntityPickedEvent
     * \brief Fired when an entity is picked by the editor
     */
    class EntityPickedEvent : public Event
    {
    public:
        EntityPickedEvent(Uma_ECS::Entity e) : entity(e) {}

    public:
        Uma_ECS::Entity entity;
    };

    /**
     * \class EntityDroppedEvent
     * \brief Fired when the currently picked entity is dropped
     */
    class EntityDroppedEvent : public Event
    {
    public:
        EntityDroppedEvent(Uma_ECS::Entity e) : entity(e) {}

    public:
        Uma_ECS::Entity entity;
    };

    /**
     * \class EditorModeChangedEvent
     * \brief Fired when the editor mode changes (translate/rotate/scale)
     */
    class EditorModeChangedEvent : public Event
    {
    public:
        EditorModeChangedEvent(int prev, int curr) : previousMode(prev), newMode(curr) {}

    public:
        int previousMode;
        int newMode;
    };

    /**
     * \class EntityTransformedEvent
     * \brief Fired when entity transform is modified via gizmo
     */
    class EntityTransformedEvent : public Event
    {
    public:
        EntityTransformedEvent(Uma_ECS::Entity e, int type) : entity(e), transformType(type) {}

    public:
        Uma_ECS::Entity entity;
        int transformType; // 0=translate, 1=rotate, 2=scale

    };


    class UpdateMouseOverUIEvent : public Event
    {
    public:
        UpdateMouseOverUIEvent(bool focus) : isFocus(focus) {}

    public:
        bool isFocus;
    };
}
