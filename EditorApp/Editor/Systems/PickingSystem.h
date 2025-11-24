/*!
\file   PickingSystem.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Defines the PickingSystem class for entity raycasting.

This header declares the PickingSystem class which provides raycasting functionality
to determine which game or UI entity is under the mouse cursor.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "ECS/Core/Coordinator.hpp"
#include "Systems/Graphics.hpp"
#include "../EditorApp/Editor/Core/EditorTypes.h"
#include <optional>

namespace Uma_Engine
{
    /*!
     * \class PickingSystem
     * \brief Handles entity picking via raycasting.
     */
    class PickingSystem
    {
    public:
        /*!
         * \brief Constructs the picking system.
         */
        PickingSystem() = default;
        
        /*!
         * \brief Destroys the picking system.
         */
        ~PickingSystem() = default;

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
         * \brief Performs a raycast to find an entity at the specified screen position.
         * \param screenPos Mouse position in screen pixels.
         * \param config Editor configuration.
         * \return Entity ID or -1 if no hit.
         */
        Uma_ECS::Entity RaycastEntity(const Vec2& screenPos, const EditorConfig& config);

        /*!
         * \brief Raycasts against game entities (with Transform components).
         * \param worldPos Position in world space.
         * \param pickRadius Search radius in world units.
         * \return Closest entity or -1 if none found.
         */
        Uma_ECS::Entity RaycastGameEntity(const Vec2& worldPos, float pickRadius);

        /*!
         * \brief Raycasts against UI entities (with RectTransform components).
         * \param ndcPos Position in NDC space [-1, 1].
         * \return Topmost UI entity or -1 if none found.
         */
        Uma_ECS::Entity RaycastUIEntity(const Vec2& ndcPos);

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

    private:
        Uma_ECS::Coordinator* pCoordinator = nullptr;
        Graphics* pGraphics = nullptr;
    };
}