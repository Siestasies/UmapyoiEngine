#pragma once

#include "../Core/System.hpp"
#include "../Core/Coordinator.hpp"

namespace Uma_ECS
{
    class StateMachineSystem : public ECSSystem
    {
    public:

        inline void Init(Coordinator* c) { gCoordinator = c; }

        void Update(float dt);

    private:

        Coordinator* gCoordinator = nullptr;
    };
}
