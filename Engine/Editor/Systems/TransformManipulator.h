#pragma once

#include "../../ECS/Core/Coordinator.hpp"
#include "../../Systems/Graphics.hpp"
#include "../Core/EditorTypes.h"

namespace Uma_Engine
{
    /**
     * \class TransformManipulator
     * \brief Applies transformations to entities during gizmo manipulation
     */
    class TransformManipulator
    {
    public:
        TransformManipulator() = default;
        ~TransformManipulator() = default;

        // Dependencies
        void SetCoordinator(Uma_ECS::Coordinator* coord) { pCoordinator = coord; }
        void SetGraphics(Graphics* gfx) { pGraphics = gfx; }

        /**
         * \brief Start a drag operation
         * \param entity Entity being manipulated
         * \param axis Which axis/handle is being dragged
         * \param startMouse Initial mouse position
         * \param state Editor state to update with drag info
         */
        void StartDrag(Uma_ECS::Entity entity, const Vec2& startMouse, GizmoAxis axis, EditorState& state);

        /**
         * \brief Update drag with current mouse position
         * \param entity Entity being manipulated
         * \param currentMouse Current mouse position
         * \param state Editor state with drag info
         * \param config Editor configuration
         */
        void UpdateDrag(const Vec2& currentMouse, EditorState& state, const EditorConfig& config);

        /**
         * \brief End drag operation
         * \param state Editor state to clear
         */
        void EndDrag(EditorState& state);

    private:
        Uma_ECS::Coordinator* pCoordinator = nullptr;
        Graphics* pGraphics = nullptr;

        // Apply specific transform types
        void ApplyTranslation(Uma_ECS::Entity entity, const Vec2& start, const Vec2& delta, const EditorConfig& config);
        void ApplyRotation(Uma_ECS::Entity entity, float deltaAngle);
        void ApplyScale(Uma_ECS::Entity entity, const Vec2& scaleFactor);

        // Helpers
        bool IsGameEntity(Uma_ECS::Entity entity) const;
        bool IsUIEntity(Uma_ECS::Entity entity) const;
        Vec2 GetEntityScreenPosition(Uma_ECS::Entity entity);
    };
}
