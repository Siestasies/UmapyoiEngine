#pragma once

#include "../Core/System.hpp"
#include "../Core/Coordinator.hpp"

namespace Uma_ECS
{
    class PhysicsSystem : public ECSSystem
    {
    public:

        void Init(Coordinator* c) { gCoordinator = c; }

        void Update(float dt);

        void PrintLog();

    private:

        Coordinator* gCoordinator = nullptr;
    };
}