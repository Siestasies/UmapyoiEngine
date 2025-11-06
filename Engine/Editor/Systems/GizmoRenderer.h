#pragma once

#include "../../Systems/Graphics.hpp"
#include "../../ECS/Core/Coordinator.hpp"
#include "../Core/EditorTypes.h"

#include <vector>

namespace Uma_Engine
{
    struct DebugLineInfo;

    /**
     * \class GizmoRenderer
     * \brief Renders interactive gizmos for entity manipulation using instanced rendering
     */
    class GizmoRenderer
    {
    public:
        GizmoRenderer() = default;
        ~GizmoRenderer() = default;

        // Dependencies
        void SetGraphics(Graphics* gfx) { pGraphics = gfx; }
        void SetCoordinator(Uma_ECS::Coordinator* coord) { pCoordinator = coord; }

        /**
         * \brief Render gizmo for current mode
         * \param entity Entity to render gizmo for
         * \param state Current editor state
         * \param config Editor configuration
         */
        void RenderGizmo(Uma_ECS::Entity entity, const EditorState& state, const EditorConfig& config);

        /**
         * \brief Render selection highlight around entity
         * \param entity Entity to highlight
         * \param config Editor configuration
         */
        void RenderSelectionHighlight(Uma_ECS::Entity entity, const EditorConfig& config);

        /**
         * \brief Test if mouse is over gizmo handle
         * \param mousePos Mouse position in screen pixels
         * \param entity Entity with gizmo
         * \param state Current editor state
         * \param config Editor configuration
         * \return Which axis was hit, or None
         */
        GizmoAxis HitTestGizmo(const Vec2& mousePos, Uma_ECS::Entity entity,
            const EditorState& state, const EditorConfig& config);

        /**
         * \brief Get screen position of entity (for gizmo placement)
         */
        Vec2 GetEntityScreenPosition(Uma_ECS::Entity entity);

    private:
        Graphics* pGraphics = nullptr;
        Uma_ECS::Coordinator* pCoordinator = nullptr;

        // Mode-specific rendering
        void RenderTranslateGizmo(const Vec2& screenPos, const EditorState& state, const EditorConfig& config);
        void RenderRotateGizmo(const Vec2& screenPos, const EditorState& state, const EditorConfig& config);
        void RenderScaleGizmo(const Vec2& screenPos, const EditorState& state, const EditorConfig& config);

        // Mode-specific hit testing
        GizmoAxis HitTestTranslateGizmo(const Vec2& mousePos, const Vec2& gizmoPos, const EditorConfig& config);
        GizmoAxis HitTestRotateGizmo(const Vec2& mousePos, const Vec2& gizmoPos, const EditorConfig& config);
        GizmoAxis HitTestScaleGizmo(const Vec2& mousePos, const Vec2& gizmoPos, const EditorConfig& config);

        // Helper: Check if entity is game or UI
        bool IsGameEntity(Uma_ECS::Entity entity) const;
        bool IsUIEntity(Uma_ECS::Entity entity) const;
    };
}