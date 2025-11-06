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
