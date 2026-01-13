#pragma once
/*!
\file   PathFindingSystem.hpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Koh Kai Yang (100%)
\par    E-mail: k.kaiyang@digipen.edu
\par    DigiPen login: k.kaiyang

\brief
Implementation of updating the pathfinding component

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "../Core/System.hpp"
#include "../Core/Coordinator.hpp"
#include "../Core/EventSystem.h"

#include "../Systems/ResourcesManager.hpp"

//#include "pathfinding/NavMesh.hpp"
#include "pathfinding/GridPathfinder.hpp"

namespace Uma_ECS
{
    /**
    * @brief System that manages pathfinding for entities.
    *
    * Uses a GridPathfinder (and optionally NavMesh) to build navigation data
    * around the player and update AI agents each frame.
    */
    class PathFindingSystem : public ECSSystem
    {
    public:
        /**
        * @brief Initialize the pathfinding system with engine dependencies.
        *
        * Stores pointers to the ECS coordinator, event system, and graphics
        * system, and sets up internal navigation structures.
        *
        * @param c         Coordinator used to access entities and components.
        * @param es        Event system for emitting/receiving path-related events.
        * @param graphics  Graphics interface used for debug drawing.
        */
        void Init(Coordinator* c, Uma_Engine::EventSystem* es, Uma_Engine::Graphics* graphics);

        /**
        * @brief Update pathfinding for all relevant entities.
        *
        * Called once per frame. May recompute navigation data when needed
        * and update agents’ paths/movement based on the current grid/navmesh.
        *
        * @param dt Delta time in seconds since last frame.
        */
        void Update(float dt);

        /**
        * @brief Clean up any resources owned by the pathfinding system.
        *
        * Releases navigation data and clears references to engine systems.
        */
        void Shutdown();

        /**
        * @brief Draw debug visualization for the pathfinding data.
        *
        * When showDebug is true, renders grid cells, obstacles, and/or paths
        * using the graphics interface.
        */
        void DebugDraw();

        // Toggle to enable/disable debug rendering of navigation data.
        bool showDebug = false;

    private:
        // ECS coordinator used to query entities and components.
        Coordinator* pCoordinator = nullptr;

        // Event system used for pathfinding-related events (if any).
        Uma_Engine::EventSystem* pEventSystem = nullptr;

        // Graphics interface used for debug drawing.
        Uma_Engine::Graphics* pGraphics = nullptr;

        // Underlying grid-based pathfinder used by this system.
        Uma_Navigation::GridPathfinder* gridPathfinder = nullptr;

        // Cached player entity ID (used as a reference for rebuilding grid).
        Entity playerID = 0;

        // World-space size of each grid cell used by the pathfinder.
        float cellSize = 2.0f;

        /// Radius around the player within which the grid is rebuilt.
        float rebuildRadius = 500.0f;

        /**
        * @brief Rebuild the grid pathfinder around a given center.
        *
        * Regenerates blocked cells and clearance information around the
        * specified position, taking into account the largest agent radius.
        *
        * @param center         World-space center of the rebuild area.
        * @param maxAgentRadius Maximum agent radius to support (in world units).
        */
        void RebuildPathfinder(const Vec2& center, float maxAgentRadius);

        // Flag indicating that navigation data needs to be rebuilt (e.g. new entities).
        bool isDirty = false;
        bool initGoal = false;
    };
}