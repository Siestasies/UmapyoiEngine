#pragma once

#include "../Core/System.hpp"
#include "../Core/Coordinator.hpp"

#include "../Systems/Graphics.hpp"


namespace Uma_ECS
{
    class RenderingSystem : public ECSSystem
    {
    public:
        void Init(Uma_Engine::Graphics* g, Coordinator* c);

        void Update(float dt);

    private:
        Coordinator* pCoordinator = nullptr;
        Uma_Engine::Graphics* pGraphics = nullptr;
    };
}