/*!
\file   PlayerControllerSystem.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Jedrek Lee Jing Wei (50%), Leong Wai Men (50%)
\par    E-mail: jedrekjingwei.lee@digipen.edu, waimen.leong@digipen.edu
\par    DigiPen login: jedrekjingwei.lee@digipen.edu, waimen.leong

\brief
Implements player input handling system that translates keyboard events into character movement and actions.

Subscribes to KeyPress/Release/Repeat events and maintains input state for WASD movement, dash, attack, and interact actions.
Applies smooth acceleration to RigidBody based on input state to prevent jerky movement transitions.
Uses static boolean flags to detect single-press actions (attack, interact, dash) and emits PlayerActionEvents
through EventSystem. Includes conditional debug logging for action triggers. Movement applies directional acceleration
with configurable smoothing factor for responsive feel.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "PlayerControllerSystem.hpp"

#include "../Components/RigidBody.h"
#include "../Components/Transform.h"
#include "../Components/Player.h"

#include "PlayerEvents.h"
#include "Debugging/Debugger.hpp"

#include <GLFW/glfw3.h>

//#define _DEBUG_LOG

#ifdef _DEBUG_LOG
#include <iostream>
#endif // _DEBUG_LOG

namespace Uma_ECS
{
    void PlayerControllerSystem::Update(float dt)
    {
        if (aEntities.empty()) return;

        // by right shd only have 1 player
        HandleMovementInput(dt);
        HandleActionInput();


    }

    void PlayerControllerSystem::OnKeyPress(const Uma_Engine::KeyPressEvent& event)
    {
        switch (event.key)
        {
        case GLFW_KEY_W:
            inputState.moveUp = true;
            break;
        case GLFW_KEY_A:
            inputState.moveLeft = true;
            break;
        case GLFW_KEY_S:
            inputState.moveDown = true;
            break;
        case GLFW_KEY_D:
            inputState.moveRight = true;
            break;
            //case GLFW_KEY_SPACE:
            //	inputState.jumpPressed = true;
            break;
        case GLFW_KEY_LEFT_CONTROL:
            inputState.attackPressed = true;
            break;
        case GLFW_KEY_E:
            inputState.interactPressed = true;
            break;
        case GLFW_KEY_LEFT_SHIFT:
            inputState.dashPressed = true;
            break;
        }
    }
    void PlayerControllerSystem::OnKeyRelease(const Uma_Engine::KeyReleaseEvent& event)
    {
        switch (event.key)
        {
        case GLFW_KEY_W:
            inputState.moveUp = false;
            break;
        case GLFW_KEY_A:
            inputState.moveLeft = false;
            break;
        case GLFW_KEY_S:
            inputState.moveDown = false;
            break;
        case GLFW_KEY_D:
            inputState.moveRight = false;
            break;
            //case GLFW_KEY_SPACE:
            //	inputState.jumpPressed = false;
            break;
        case GLFW_KEY_LEFT_CONTROL:
            inputState.attackPressed = false;
            break;
        case GLFW_KEY_E:
            inputState.interactPressed = false;
            break;
        case GLFW_KEY_LEFT_SHIFT:
            inputState.dashPressed = false;
            break;
        }
    }
    void PlayerControllerSystem::OnKeyRepeat(const Uma_Engine::KeyRepeatEvent& event)
    {
        (void)event;
    }

    void PlayerControllerSystem::HandleMovementInput(float dt)
    {
        if (aEntities.empty()) return;

        Entity player = aEntities[0];
        auto& rb = pCoordinator->GetComponent<RigidBody>(player);

        Vec2 targetAccel = { 0, 0 };

        if (inputState.moveRight)   targetAccel.x += rb.accel_strength;
        if (inputState.moveLeft )   targetAccel.x -= rb.accel_strength;
        if (inputState.moveUp   )   targetAccel.y += rb.accel_strength;
        if (inputState.moveDown )   targetAccel.y -= rb.accel_strength;

        if (magnitude(rb.velocity) > 0.1f)
        {
            if (pEventSystem)
            {
                //eventSystem->Emit<Uma_Engine::PlayerMoveEvent>(player, rb.velocity.x, rb.velocity.y);
            }
        }

        // Smooth acceleration to prevent jerk
        const float accelSmoothFactor = 15.f; // tweak for responsiveness
        rb.acceleration += (targetAccel - rb.acceleration) * accelSmoothFactor * dt;
     
#ifdef _DEBUG_LOG

        std::stringstream ss{ "" };
        ss << "Player Movement: " << rb.velocity;
        Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo, ss.str());
#endif

    }

    void PlayerControllerSystem::HandleActionInput()
    {
        if (aEntities.empty()) return;

        Entity player = *(aEntities.begin());

        // Handle action inputs (only trigger once per press)
        //static bool lastJumpState = false;
        static bool lastAttackState = false;
        static bool lastInteractState = false;
        static bool lastDashState = false;

        //		if (inputState.jumpPressed && !lastJumpState)
        //		{
        //			if (eventSystem)
        //			{
        //				eventSystem->Emit<Uma_Engine::PlayerActionEvent>(player, Uma_Engine::PlayerActionEvent::ActionType::Jump);
        //			}
        //#ifdef _DEBUG_LOG
        //			std::cout << "Player Jump Action" << std::endl;
        //#endif
        //		}

        if (inputState.attackPressed && !lastAttackState)
        {
            if (pEventSystem)
            {
                pEventSystem->Emit<Uma_Engine::PlayerActionEvent>(player, Uma_Engine::PlayerActionEvent::ActionType::Attack);
            }
#ifdef _DEBUG_LOG
            std::cout << "Player Attack Action" << std::endl;
#endif
        }

        if (inputState.interactPressed && !lastInteractState)
        {
            if (pEventSystem)
            {
                pEventSystem->Emit<Uma_Engine::PlayerActionEvent>(player, Uma_Engine::PlayerActionEvent::ActionType::Interact);
            }
#ifdef _DEBUG_LOG
            std::cout << "Player Interact Action" << std::endl;
#endif
        }

        if (inputState.dashPressed && !lastDashState)
        {
            if (pEventSystem)
            {
                pEventSystem->Emit<Uma_Engine::PlayerActionEvent>(player, Uma_Engine::PlayerActionEvent::ActionType::Dash);
            }
#ifdef _DEBUG_LOG
            std::cout << "Player Dash Action" << std::endl;
#endif
        }

        // Update previous states
        //lastJumpState = inputState.jumpPressed;
        lastAttackState = inputState.attackPressed;
        lastInteractState = inputState.interactPressed;
        lastDashState = inputState.dashPressed;
    }
}