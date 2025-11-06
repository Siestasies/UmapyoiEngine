/*!
\file   GizmoRenderer.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Defines the GizmoRenderer class for interactive manipulation gizmos.

This header declares the GizmoRenderer class which handles rendering and hit-testing
of translation, rotation, and scale gizmos for both game and UI entities.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "../../Systems/Graphics.hpp"
#include "../../ECS/Core/Coordinator.hpp"
#include "../Core/EditorTypes.h"
#include <vector>

namespace Uma_Engine
{
    struct DebugLineInfo;

    /*!
     * \class GizmoRenderer
     * \brief Renders interactive gizmos for entity manipulation using instanced rendering.
     */
    class GizmoRenderer
    {
    public:
        /*!
         * \brief Constructs the gizmo renderer.
         */
        GizmoRenderer() = default;
        
        /*!
         * \brief Destroys the gizmo renderer.
         */
        ~GizmoRenderer() = default;

        /*!
         * \brief Sets the graphics system dependency.
         * \param gfx Pointer to the graphics system.
         */
        void SetGraphics(Graphics* gfx) { pGraphics = gfx; }

        /*!
         * \brief Sets the ECS coordinator dependency.
         * \param coord Pointer to the coordinator.
         */
        void SetCoordinator(Uma_ECS::Coordinator* coord) { pCoordinator = coord; }

        /*!
         * \brief Renders the appropriate gizmo for the current editor mode.
         * \param entity Entity to render gizmo for.
         * \param state Current editor state.
         * \param config Editor configuration.
         */
        void RenderGizmo(Uma_ECS::Entity entity, const EditorState& state, const EditorConfig& config);

        /*!
         * \brief Renders selection highlight around an entity.
         * \param entity Entity to highlight.
         * \param config Editor configuration.
         */
        void RenderSelectionHighlight(Uma_ECS::Entity entity, const EditorConfig& config);

        /*!
         * \brief Performs hit-testing on gizmo handles.
         * \param mousePos Mouse position in screen pixels.
         * \param entity Entity with the gizmo.
         * \param state Current editor state.
         * \param config Editor configuration.
         * \return The axis that was hit, or GizmoAxis::None.
         */
        GizmoAxis HitTestGizmo(const Vec2& mousePos, Uma_ECS::Entity entity,
            const EditorState& state, const EditorConfig& config);

        /*!
         * \brief Gets the screen position of an entity for gizmo placement.
         * \param entity Entity to get screen position for.
         * \return Screen position as Vec2.
         */
        Vec2 GetEntityScreenPosition(Uma_ECS::Entity entity);

    private:
        Graphics* pGraphics = nullptr;
        Uma_ECS::Coordinator* pCoordinator = nullptr;

        /*!
         * \brief Renders the translation gizmo.
         * \param screenPos Gizmo position in screen pixels.
         * \param state Current editor state.
         * \param config Editor configuration.
         */
        void RenderTranslateGizmo(const Vec2& screenPos, const EditorState& state, const EditorConfig& config);

        /*!
         * \brief Renders the rotation gizmo.
         * \param screenPos Gizmo position in screen pixels.
         * \param state Current editor state.
         * \param config Editor configuration.
         */
        void RenderRotateGizmo(const Vec2& screenPos, const EditorState& state, const EditorConfig& config);

        /*!
         * \brief Renders the scale gizmo.
         * \param screenPos Gizmo position in screen pixels.
         * \param state Current editor state.
         * \param config Editor configuration.
         */
        void RenderScaleGizmo(const Vec2& screenPos, const EditorState& state, const EditorConfig& config);

        /*!
         * \brief Hit-tests the translation gizmo handles.
         * \param mousePos Mouse position in screen pixels.
         * \param gizmoPos Gizmo position in screen pixels.
         * \param config Editor configuration.
         * \return The axis that was hit, or GizmoAxis::None.
         */
        GizmoAxis HitTestTranslateGizmo(const Vec2& mousePos, const Vec2& gizmoPos, const EditorConfig& config);

        /*!
         * \brief Hit-tests the rotation gizmo.
         * \param mousePos Mouse position in screen pixels.
         * \param gizmoPos Gizmo position in screen pixels.
         * \param config Editor configuration.
         * \return The axis that was hit, or GizmoAxis::None.
         */
        GizmoAxis HitTestRotateGizmo(const Vec2& mousePos, const Vec2& gizmoPos, const EditorConfig& config);

        /*!
         * \brief Hit-tests the scale gizmo handles.
         * \param mousePos Mouse position in screen pixels.
         * \param gizmoPos Gizmo position in screen pixels.
         * \param config Editor configuration.
         * \return The axis that was hit, or GizmoAxis::None.
         */
        GizmoAxis HitTestScaleGizmo(const Vec2& mousePos, const Vec2& gizmoPos, const EditorConfig& config);

        /*!
         * \brief Checks if an entity is a game entity.
         * \param entity Entity to check.
         * \return True if it's a game entity.
         */
        bool IsGameEntity(Uma_ECS::Entity entity) const;

        /*!
         * \brief Checks if an entity is a UI entity.
         * \param entity Entity to check.
         * \return True if it's a UI entity.
         */
        bool IsUIEntity(Uma_ECS::Entity entity) const;
    };
}