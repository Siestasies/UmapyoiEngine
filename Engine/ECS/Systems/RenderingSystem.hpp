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

    private:
        Coordinator* pCoordinator = nullptr;
        Uma_Engine::Graphics* pGraphics = nullptr;
        Uma_Engine::ResourcesManager* pResourcesManager = nullptr;
    };
}