/*!
\file   PlayerControllerSystem.hpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Jedrek Lee Jing Wei (50%), Leong Wai Men (50%)
\par    E-mail: jedrekjingwei.lee@digipen.edu, waimen.leong@digipen.edu
\par    DigiPen login: jedrekjingwei.lee@digipen.edu, waimen.leong

\brief
Defines player controller system that bridges input events to player entity physics and actions.

Maintains InputState struct tracking movement directions and action button states (attack, interact, dash).
Subscribes to keyboard events during initialization and processes input each frame to modify player RigidBody.
Requires EventSystem for event subscription and emission, HybridInputSystem reference, and Coordinator for component access.
Separates movement input (continuous WASD) from action input (single-press detection) in distinct handler methods.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

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