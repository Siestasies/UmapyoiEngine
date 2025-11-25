/*!
\file   TransformManipulator.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Defines the TransformManipulator class for applying entity transformations.

This header declares the TransformManipulator class which handles applying
translation, rotation, and scale transformations to entities during gizmo interactions.

CORRECTED: Added helper methods for canvas scale and marking children dirty.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "../../ECS/Core/Coordinator.hpp"
#include "../../Systems/Graphics.hpp"
#include "../Core/EditorTypes.h"

namespace Uma_Engine
{
    /*!
     * \class TransformManipulator
     * \brief Applies transformations to entities during gizmo manipulation.
     */
    class TransformManipulator
    {
    public:
        /*!
         * \brief Constructs the transform manipulator.
         */
        TransformManipulator() = default;

        /*!
         * \brief Destroys the transform manipulator.
         */
        ~TransformManipulator() = default;

        /*!
         * \brief Sets the ECS coordinator dependency.
         * \param coord Pointer to the coordinator.
         */
        void SetCoordinator(Uma_ECS::Coordinator* coord) { pCoordinator = coord; }

        /*!
         * \brief Sets the graphics system dependency.
         * \param gfx Pointer to the graphics system.
         */
        void SetGraphics(Graphics* gfx) { pGraphics = gfx; }

        /*!
         * \brief Initializes a drag operation on an entity.
         * \param entity Entity being manipulated.
         * \param startMouse Initial mouse position in screen pixels.
         * \param axis Which axis/handle is being dragged.
         * \param state Editor state to update with drag information.
         */
        void StartDrag(Uma_ECS::Entity entity, const Vec2& startMouse, GizmoAxis axis, EditorState& state);

        /*!
         * \brief Updates the drag operation with the current mouse position.
         * \param currentMouse Current mouse position in screen pixels.
         * \param state Editor state with drag information.
         * \param config Editor configuration.
         */
        void UpdateDrag(const Vec2& currentMouse, EditorState& state, const EditorConfig& config);

        /*!
         * \brief Ends the current drag operation.
         * \param state Editor state to clear.
         */
        void EndDrag(EditorState& state);

    private:
        Uma_ECS::Coordinator* pCoordinator = nullptr;
        Graphics* pGraphics = nullptr;

        /*!
         * \brief Applies translation to an entity.
         * \param entity Entity to translate.
         * \param start Starting position.
         * \param delta Translation delta.
         * \param config Editor configuration.
         */
        void ApplyTranslation(Uma_ECS::Entity entity, const Vec2& start, const Vec2& delta, const EditorConfig& config);

        /*!
         * \brief Applies rotation to an entity.
         * \param entity Entity to rotate.
         * \param deltaAngle Rotation delta in radians.
         */
        void ApplyRotation(Uma_ECS::Entity entity, float deltaAngle);

        /*!
         * \brief Applies scaling to an entity.
         * \param entity Entity to scale.
         * \param scaleFactor Scale factor to apply.
         */
        void ApplyScale(Uma_ECS::Entity entity, const Vec2& scaleFactor);

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

        /*!
         * \brief Gets the screen position of an entity.
         * \param entity Entity to get screen position for.
         * \return Screen position as Vec2.
         */
        Vec2 GetEntityScreenPosition(Uma_ECS::Entity entity);

        /*!
         * \brief Gets the canvas scale factor by walking up Transform hierarchy.
         * \param entity Entity to find canvas scale for.
         * \return Canvas scale factor, or 1.0f if no canvas found.
         */
        float GetCanvasScale(Uma_ECS::Entity entity);

        /*!
         * \brief Marks all children as dirty recursively using Transform hierarchy.
         * \param entity Parent entity whose children to mark dirty.
         */
        void MarkChildrenDirty(Uma_ECS::Entity entity);
    };
}