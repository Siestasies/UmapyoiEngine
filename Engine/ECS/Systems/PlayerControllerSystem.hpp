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
        /*!
        \brief Initializes the player controller with required engine dependencies.
        \param es Pointer to the EventSystem for subscribing to input events.
        \param is Pointer to the HybridInputSystem for polling input state.
        \param c Pointer to the ECS Coordinator for component access.
        \param g Pointer to the Graphics system.
        */
        void Init(Uma_Engine::EventSystem* es, Uma_Engine::HybridInputSystem* is, Coordinator* c, Uma_Engine::Graphics* g);

        /*!
        \brief Updates player movement and actions each frame.
        \param dt Delta time in seconds since last frame.
        */
        void Update(float dt);

        /*!
        \brief Shuts down the player controller and unsubscribes from events.
        */
        void Shutdown();

    private:
        /*!
        \brief Handles a key press event and updates the input state.
        \param event Key press event data.
        */
        void OnKeyPress(const Uma_Engine::KeyPressEvent& event);

        /*!
        \brief Handles a key release event and updates the input state.
        \param event Key release event data.
        */
        void OnKeyRelease(const Uma_Engine::KeyReleaseEvent& event);

        /*!
        \brief Handles a key repeat event for continuous key input.
        \param event Key repeat event data.
        */
        void OnKeyRepeat(const Uma_Engine::KeyRepeatEvent& event);

        /*!
        \brief Processes continuous movement input (WASD) and applies velocity to the player entity.
        \param dt Delta time in seconds since last frame.
        */
        void HandleMovementInput(float dt);

        /*!
        \brief Processes single-press action inputs such as attack, interact, and dash.
        \param dt Delta time in seconds since last frame.
        */
        void HandleActionInput(float dt);

        /*!
        \brief Subscribes to keyboard and mouse input events via the EventSystem.
        */
        void SubscribeToEvents();

        /*!
        \brief Updates the player's animation state based on current movement and actions.
        */
        void HandlePlayerAnimation();

        /*!
        \brief Handles collision response between a defender and an attacker entity.
        \param deffender Entity receiving the collision.
        \param attacker Entity initiating the collision.
        */
        void HandleCollision(Entity deffender, Entity attacker);

        /*!
        \brief Applies damage to an entity and triggers hurt response logic.
        \param entity Entity taking damage.
        \param damage Amount of damage to apply.
        */
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

        bool isPlayerDead = false;

        Uma_Engine::EventSystem* pEventSystem = nullptr;
        Uma_Engine::Graphics* pGraphicsSystem = nullptr;
        Uma_Engine::HybridInputSystem* pHybridInputSystem = nullptr;
        Coordinator* pCoordinator = nullptr;
    };
}