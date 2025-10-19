/*!
\file   RenderingSystem.hpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines rendering system that orchestrates sprite drawing through graphics API with texture batching optimization.

Operates on entities with SpriteRenderer and Transform components to extract sprite data and world positions.
Requires initialization with Graphics renderer, ResourcesManager for texture loading, and Coordinator for component queries.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "../Core/System.hpp"
#include "../Core/Coordinator.hpp"

#include "../Systems/Graphics.hpp"
#include "../Systems/ResourcesManager.hpp"


namespace Uma_ECS
{
    class RenderingSystem : public ECSSystem
    {
    public:
        void Init(Uma_Engine::Graphics* g, Uma_Engine::ResourcesManager* rm, Coordinator* c);

        void Update(float dt);

        void SortEntitiesByLayer(std::vector<Entity>& sorted);

    private:

        Coordinator* pCoordinator = nullptr;
        Uma_Engine::Graphics* pGraphics = nullptr;
        Uma_Engine::ResourcesManager* pResourcesManager = nullptr;
    };
}