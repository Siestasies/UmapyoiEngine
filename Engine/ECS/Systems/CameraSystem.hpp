#pragma once

#include "../Core/System.hpp"
#include "../Core/Coordinator.hpp"

namespace Uma_ECS
{
    class CameraSystem : public ECSSystem
    {
    public:

        inline void Init(Coordinator* c)
        {
            pCoordinator = c;
        }

        void Update(float dt);


    private:

        Coordinator* pCoordinator = nullptr;
    };
}