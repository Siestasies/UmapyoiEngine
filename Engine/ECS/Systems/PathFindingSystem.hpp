#pragma once
/*!
\file   AudioSystem.hpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Koh Kai Yang (100%)
\par    E-mail: k.kaiyang@digipen.edu
\par    DigiPen login: k.kaiyang

\brief
Implementation of updating the audio listener position/audio component

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "../Core/System.hpp"
#include "../Core/Coordinator.hpp"
#include "../Core/EventSystem.h"

#include "../Systems/ResourcesManager.hpp"

#include "../pathfinding/NavMesh.hpp"

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
        void Init(Coordinator* c, Uma_Engine::EventSystem* es);

        /*!
        * \brief updates the listner position in sound manager
        * \return nothing
        */
        void Update(float dt);

        void Shutdown();

        void LoadTiles(const std::vector<Uma_Navigation::Tile>& tiles);

    private:

        Coordinator* pCoordinator = nullptr;
        Uma_Engine::EventSystem* pEventSystem = nullptr;

        Uma_Navigation::DynamicNavMesh* navmesh;
        float navmeshUpdateTimer = 0.0f;
        Vec2 navmeshCenter;
    };
}