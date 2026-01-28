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
#include "Systems/FSMSystem.hpp"
#include "Components/FSM.h"

namespace Uma_ECS
{
    class PlayerControllerSystem : public ECSSystem
    {
    public:
        void Init(Uma_Engine::EventSystem* es, Uma_Engine::HybridInputSystem* is, Coordinator* c, Uma_Engine::Graphics* g);
        void Update(float dt);
        void Shutdown();

        // ============================================================
        // Input Query API - Called by Lua scripts
        // ============================================================

        bool IsAttack1Pressed() const { return inputState.attack_1 && !inputState.attack_1_consumed; }
        bool IsAttack2Pressed() const { return inputState.attack_2 && !inputState.attack_2_consumed; }
        bool IsInteractPressed() const { return inputState.interactPressed && !inputState.interactConsumed; }
        bool IsDashPressed() const { return inputState.dashPressed && !inputState.dashConsumed; }
        bool IsMovePressed() const { return inputState.rightMousePressed && !inputState.rightMouseConsumed; }

        // Input consumption - prevents multiple state transitions from same input
        void ConsumeAttack1() { inputState.attack_1_consumed = true; }
        void ConsumeAttack2() { inputState.attack_2_consumed = true; }
        void ConsumeInteract() { inputState.interactConsumed = true; }
        void ConsumeDash() { inputState.dashConsumed = true; }
        void ConsumeMove() { inputState.rightMouseConsumed = true; }

        // Get world position for movement target
        Uma_Math::Vec2 GetMoveTargetPosition() const;

    private:
        void SubscribeToEvents();

        // Input event handlers
        void OnKeyPress(const Uma_Engine::KeyPressEvent& event);
        void OnKeyRelease(const Uma_Engine::KeyReleaseEvent& event);

        // Collision handling
        void HandleTriggerEnter(Entity player, Entity other);
        void OnPlayerHurt(Entity entity, int damage);

    private:
        struct InputState
        {
            // Attack inputs
            bool attack_1 = false;
            bool attack_2 = false;
            bool attack_1_consumed = false;
            bool attack_2_consumed = false;

            // Action inputs
            bool interactPressed = false;
            bool interactConsumed = false;
            bool dashPressed = false;
            bool dashConsumed = false;

            // Movement input
            bool rightMousePressed = false;
            bool rightMouseConsumed = false;

            void ResetConsumedFlags()
            {
                attack_1_consumed = false;
                attack_2_consumed = false;
                interactConsumed = false;
                dashConsumed = false;
            }
        } inputState;

        Uma_Engine::EventSystem* pEventSystem = nullptr;
        Uma_Engine::Graphics* pGraphicsSystem = nullptr;
        Uma_Engine::HybridInputSystem* pHybridInputSystem = nullptr;
        Coordinator* pCoordinator = nullptr;
    };

    //class PlayerControllerSystem : public ECSSystem
    //{
    //public:
    //    void Init(Uma_Engine::EventSystem* es, Uma_Engine::HybridInputSystem* is, Coordinator* c, Uma_Engine::Graphics* g);
    //    
    //    void Update(float dt);

    //    void Shutdown();

    //private:
    //    void OnKeyPress(const Uma_Engine::KeyPressEvent& event);
    //    void OnKeyRelease(const Uma_Engine::KeyReleaseEvent& event);
    //    void OnKeyRepeat(const Uma_Engine::KeyRepeatEvent& event);

    //    void HandleMovementInput(float dt);
    //    void HandleActionInput(float dt);
    //    void SubscribeToEvents();

    //    void HandlePlayerAnimation();

    //    void HandleCollision(Entity deffender, Entity attacker);

    //    void OnHurt(Entity entity, int damage);

    //private:
    //    struct InputState
    //    {
    //        bool attack_1 = false;
    //        bool attack_2 = false;

    //        bool interactPressed = false;
    //        bool dashPressed = false;
    //        bool movePressed = false;

    //        // Mouse input buffering for fixed timestep
    //        bool rightMousePressed = false;
    //        bool rightMouseConsumed = false;  // Track if we've already processed this press
    //    } inputState;

    //    bool isPlayerDead = false;

    //    Uma_Engine::EventSystem* pEventSystem = nullptr;
    //    Uma_Engine::Graphics* pGraphicsSystem = nullptr;
    //    Uma_Engine::HybridInputSystem* pHybridInputSystem = nullptr;
    //    Coordinator* pCoordinator = nullptr;
    //};

}