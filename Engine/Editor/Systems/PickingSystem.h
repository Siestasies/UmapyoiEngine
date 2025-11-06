#pragma once

#include "../../ECS/Core/Coordinator.hpp"
#include "../../Systems/Graphics.hpp"
#include "../Core/EditorTypes.h"

#include <optional>

namespace Uma_Engine
{
    /**
     * \class PickingSystem
     * \brief Handles entity picking via raycasting
     */
    class PickingSystem
    {
    public:
        PickingSystem() = default;
        ~PickingSystem() = default;

        // Dependencies
        void SetCoordinator(Uma_ECS::Coordinator* coord) { pCoordinator = coord; }
        void SetGraphics(Graphics* gfx) { pGraphics = gfx; }

        /**
         * \brief Raycast to find entity at screen position
         * \param screenPos Mouse position in screen pixels
         * \param config Editor configuration (for pick radius)
         * \return Entity ID or -1 if no hit
         */
        Uma_ECS::Entity RaycastEntity(const Vec2& screenPos, const EditorConfig& config);

        /**
         * \brief Raycast game entities (with Transform)
         * \param worldPos Position in world space
         * \param pickRadius Search radius in world units
         * \return Closest entity or -1 if none found
         */
        Uma_ECS::Entity RaycastGameEntity(const Vec2& worldPos, float pickRadius);

        /**
         * \brief Raycast UI entities (with RectTransform)
         * \param ndcPos Position in NDC space [-1, 1]
         * \return Topmost UI entity or -1 if none found
         */
        Uma_ECS::Entity RaycastUIEntity(const Vec2& ndcPos);

        // Helpers
        bool IsGameEntity(Uma_ECS::Entity entity) const;
        bool IsUIEntity(Uma_ECS::Entity entity) const;

    private:
        Uma_ECS::Coordinator* pCoordinator = nullptr;
        Graphics* pGraphics = nullptr;
    };
}
