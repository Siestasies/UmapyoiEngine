    #pragma once

#include "../Core/System.hpp"
#include "../Core/Coordinator.hpp"

#include "../../Core/EventSystem.h"
#include "../../Core/InputEvents.h"

#include "../../Systems/InputSystem.h"

#include "Test_Input_Events.h"

namespace Uma_ECS
{
    class PlayerControllerSystem : public ECSSystem
    {
    public:
        inline void Init(Uma_Engine::EventSystem* es, Uma_Engine::HybridInputSystem* is, Coordinator* c)
        {
            pEventSystem = es;
            pHybridInputSystem = is;
            pCoordinator = c;

            pEventSystem->Subscribe<Uma_Engine::KeyPressEvent>([this](const Uma_Engine::KeyPressEvent& e) { OnKeyPress(e); });
            pEventSystem->Subscribe<Uma_Engine::KeyReleaseEvent>([this](const Uma_Engine::KeyReleaseEvent& e) { OnKeyRelease(e); });
            pEventSystem->Subscribe<Uma_Engine::KeyRepeatEvent>([this](const Uma_Engine::KeyRepeatEvent& e) { OnKeyRepeat(e); });
        }
        
        void Update(float dt);
    private:
        void OnKeyPress(const Uma_Engine::KeyPressEvent& event);
        void OnKeyRelease(const Uma_Engine::KeyReleaseEvent& event);
        void OnKeyRepeat(const Uma_Engine::KeyRepeatEvent& event);

        void HandleMovementInput(float dt);
        void HandleActionInput();

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

        Uma_Engine::EventSystem* pEventSystem = nullptr;
        Uma_Engine::HybridInputSystem* pHybridInputSystem = nullptr;
        Coordinator* pCoordinator = nullptr;
    };
}