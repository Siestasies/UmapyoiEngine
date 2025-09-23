#pragma once

#include "../Core/System.hpp"
#include "../Core/Coordinator.hpp"

#include "../Systems/Graphics.hpp"


namespace Uma_ECS
{
    class RenderingSystem : public ECSSystem
    {
    public:
        inline void Init(Uma_Engine::Graphics* g, Coordinator* c)
        {
            pCoordinator = c;
            pGraphics = g;
        }

        void Update(float dt);

    private:
        Coordinator* pCoordinator = nullptr;
        Uma_Engine::Graphics* pGraphics = nullptr;
    };
}