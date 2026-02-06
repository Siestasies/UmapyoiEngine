/*!
\file   TilemapSystem.hpp
\par    Project: GAM250
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 4

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
ECS system responsible for rendering tilemaps each frame.
Processes all Tilemap components, sorts layers by render order,
and submits tile geometry to the graphics system for rendering.


All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "../Core/Types.hpp"
#include "../Core/System.hpp"
#include "../Core/Coordinator.hpp"

#include "../Components/Tilemap.h"
#include "Systems/Graphics.hpp"
#include "Systems/ResourcesManager.hpp"

namespace Uma_ECS
{
    struct tilemapLayerInfo
    {
        Entity tilemapId;
        int index;
        int sortingOrder;
    };

    class TilemapSystem : public ECSSystem
    {
    public:

        /**
         * @brief Initialize the tilemap rendering system
         *
         * Stores references to required systems (Coordinator for entity access,
         * Graphics for rendering, ResourcesManager for texture loading).
         *
         * @param coord ECS coordinator for accessing tilemap components
         * @param graphics Graphics system for submitting draw calls
         * @param resourcesManager Resource manager for loading tileset textures
         */
        void Init(Coordinator* coord, Uma_Engine::Graphics* graphics, Uma_Engine::ResourcesManager* resourcesManager);

        /**
         * @brief Update and render all tilemaps each frame
         *
         * Collects all visible layers from all active tilemaps, sorts them by
         * render order, and submits tile geometry to the graphics system for
         * batched rendering.
         *
         * @param dt Delta time in seconds (currently unused for rendering)
         */
        void Update(float dt);

        /**
         * @brief Clean up tilemap system resources on shutdown
         *
         * Releases any allocated resources and resets system state.
         */
        void Shutdown();

    private:

        Coordinator* pCoordinator = nullptr;
        Uma_Engine::Graphics* pGraphics = nullptr;
        Uma_Engine::ResourcesManager* pResourcesManager = nullptr;
    };
}