/*!
\file   TilemapSystem.cpp
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Implementation of the tilemap rendering system


All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "TilemapSystem.hpp"

#include "ECS/Components/Transform.h"
#include "ECS/Components/Sprite.h"

namespace Uma_ECS
{
    void TilemapSystem::Init(Coordinator* coord, Uma_Engine::Graphics* graphics, Uma_Engine::ResourcesManager* resourcesManager)
    {
        pCoordinator = coord;
        pGraphics = graphics;
        pResourcesManager = resourcesManager;
    }

    void TilemapSystem::Update(float dt)
    {
        (void)dt;
    }

    void TilemapSystem::Shutdown()
    {

    }
}