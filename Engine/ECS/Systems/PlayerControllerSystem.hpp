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

#include "Core/System.hpp"
#include "Core/Coordinator.hpp"

#include "Core/EventSystem.h"
#include "Events/InputEvents.h"

#include "Systems/InputSystem.h"
#include "Systems/HybridInputSystem.h"
#include "Systems/Graphics.hpp"

namespace Uma_ECS
{
    class PlayerControllerSystem : public ECSSystem
    {
    public:
        void Init(Uma_Engine::EventSystem* es, Uma_Engine::HybridInputSystem* is, Coordinator* c, Uma_Engine::Graphics* g);
        
        void Update(float dt);

        void Shutdown();

    private:
        void OnKeyPress(const Uma_Engine::KeyPressEvent& event);
        void OnKeyRelease(const Uma_Engine::KeyReleaseEvent& event);
        void OnKeyRepeat(const Uma_Engine::KeyRepeatEvent& event);

        void HandleMovementInput(float dt);
        void HandleActionInput(float dt);
        void SubscribeToEvents();

        void HandlePlayerAnimation();

        void HandleCollision(Entity deffender, Entity attacker);

        void OnHurt(Entity entity, int damage);

    private:
        struct InputState
        {
            bool attack_1 = false;
            bool attack_2 = false;

            bool interactPressed = false;
            bool dashPressed = false;
            bool movePressed = false;

            // Mouse input buffering for fixed timestep
            bool rightMousePressed = false;
            bool rightMouseConsumed = false;  // Track if we've already processed this press
        } inputState;

        Uma_Engine::EventSystem* pEventSystem = nullptr;
        Uma_Engine::Graphics* pGraphicsSystem = nullptr;
        Uma_Engine::HybridInputSystem* pHybridInputSystem = nullptr;
        Coordinator* pCoordinator = nullptr;
    };
}