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

#include "pathfinding/NavMesh.hpp"
#include "pathfinding/GridPathfinder.hpp"

namespace Uma_ECS
{
    class PathFindingSystem : public ECSSystem
    {
    public:
        /*!
        * \brief passes reference of the sound manager and coordinator to this component update
        * \param sound manager and coordinator pointer
        * \return nothing
        */
        void Init(Coordinator* c, Uma_Engine::EventSystem* es, Uma_Engine::Graphics* graphics);

        /*!
        * \brief updates the listner position in sound manager
        * \return nothing
        */
        void Update(float dt);

        void Shutdown();

        void DebugDraw();
        bool showDebug = false;
    private:

        Coordinator* pCoordinator = nullptr;
        Uma_Engine::EventSystem* pEventSystem = nullptr;
        Uma_Engine::Graphics* pGraphics = nullptr;

        Uma_Navigation::GridPathfinder* gridPathfinder = nullptr;
        Entity playerID = 0;
        float cellSize = 2.0f;
        float rebuildRadius = 500.0f;

        void RebuildPathfinder(const Vec2& center, float maxAgentRadius);
        bool isDirty = false; //to check for any new entites
    };
}