#pragma once
#include "../Core/System.hpp"
#include "../Core/Coordinator.hpp"

namespace Uma_ECS
{
    class AnimatorSystem : public ECSSystem
    {
    public:
        void Init(Coordinator* c);
        void Update(float dt);

    private:
        Coordinator* pCoordinator = nullptr;
    };
}