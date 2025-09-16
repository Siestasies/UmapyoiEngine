#pragma once

#include "../Core/System.hpp"
#include "../Core/Coordinator.hpp"
#include "../../Systems/InputSystem.h"


namespace Uma_ECS
{
    class PlayerControllerSystem : public ECSSystem
    {
    public:
        inline void Init(Uma_Engine::InputSystem* is, Coordinator* c) 
        { 
            pInputSystem = is;
            pCoordinator = c;
        }
        
        void Update(float dt);

    private:
        Uma_Engine::InputSystem* pInputSystem = nullptr;
        Coordinator* pCoordinator = nullptr;
    };
}