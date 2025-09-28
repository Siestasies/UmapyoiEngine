    #pragma once

#include "../Core/System.hpp"
#include "../Core/Coordinator.hpp"

#include "../../Core/EventSystem.h"
#include "../../Core/InputEvents.h"

#include "../../Systems/InputSystem.h"

#include "Test_Input_Events.h"

namespace Uma_ECS
{
    class PlayerControllerSystem : public ECSSystem, Uma_Engine::EventListenerSystem
    {
    public:
        inline void Init(Uma_Engine::SystemManager* sm, Uma_Engine::HybridInputSystem* is, Coordinator* c)
        { 
            pSystemManager = sm;
            pInputSystem = is;
            pCoordinator = c;

            EventListenerSystem::Init();
        }
        
        void Update(float dt);
    protected:
        void RegisterEventListeners() override;

    private:
        void OnKeyPress(const Uma_Engine::KeyPressEvent& event);
        void OnKeyRelease(const Uma_Engine::KeyReleaseEvent& event);
        void OnKeyRepeat(const Uma_Engine::KeyRepeatEvent& event);

        void HandleMovementInput(float dt);
        void HandleActionInput();
        //void UpdatePlayerMovement(Entity player, float velocityX, float velocityY);

    private:
        struct InputState
        {
            bool moveUp = false;
            bool moveDown = false;
            bool moveLeft = false;
            bool moveRight = false;
            // bool jumpPressed = false;
            bool attackPressed = false;
            bool interactPressed = false;
            bool dashPressed = false;
        } inputState;

        Uma_Engine::HybridInputSystem* pInputSystem = nullptr;
        Coordinator* pCoordinator = nullptr;
    };
}