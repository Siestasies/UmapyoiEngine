#include "PlayerControllerSystem.hpp"

#include "../Components/RigidBody.h"
#include "../Components/Transform.h"
#include "../Components/Player.h"

#include "PlayerEvents.h"

#include <GLFW/glfw3.h>

#define _DEBUG_LOG

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
    void PlayerControllerSystem::RegisterEventListeners()
    {
        SubscribeToEvent<Uma_Engine::KeyPressEvent>([this](const Uma_Engine::KeyPressEvent& event) { OnKeyPress(event); });
        SubscribeToEvent<Uma_Engine::KeyReleaseEvent>([this](const Uma_Engine::KeyReleaseEvent& event) { OnKeyRelease(event); });
        SubscribeToEvent<Uma_Engine::KeyRepeatEvent>([this](const Uma_Engine::KeyRepeatEvent& event) { OnKeyRepeat(event); });
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
    }

    void PlayerControllerSystem::HandleMovementInput(float dt)
    {
        if (aEntities.empty()) return;

        auto& pArray = pCoordinator->GetComponentArray<Player>();
        auto& rbArray = pCoordinator->GetComponentArray<RigidBody>();
        auto& tfArray = pCoordinator->GetComponentArray<Transform>();

        Entity player = aEntities[0];
        auto& rb = pCoordinator->GetComponent<RigidBody>(player);

        Vec2 targetAccel = { 0, 0 };

        if (inputState.moveRight)   targetAccel.x += rb.accel_strength;
        if (inputState.moveLeft)    targetAccel.x -= rb.accel_strength;
        if (inputState.moveUp)      targetAccel.y += rb.accel_strength;
        if (inputState.moveDown)    targetAccel.y -= rb.accel_strength;

        if (magnitude(rb.velocity) > 0.1f)
        {
            if (eventSystem)
            {
                //eventSystem->Emit<Uma_Engine::PlayerMoveEvent>(player, rb.velocity.x, rb.velocity.y);
            }
        }

        // Smooth acceleration to prevent jerk
        const float accelSmoothFactor = 15.f; // tweak for responsiveness
        rb.acceleration += (targetAccel - rb.acceleration) * accelSmoothFactor * dt;

#ifdef _DEBUG_LOG
        //std::cout << "Player Movement: " << rb.velocity << std::endl;
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
            if (eventSystem)
            {
                eventSystem->Emit<Uma_Engine::PlayerActionEvent>(player, Uma_Engine::PlayerActionEvent::ActionType::Attack);
            }
#ifdef _DEBUG_LOG
            std::cout << "Player Attack Action" << std::endl;
#endif
        }

        if (inputState.interactPressed && !lastInteractState)
        {
            if (eventSystem)
            {
                eventSystem->Emit<Uma_Engine::PlayerActionEvent>(player, Uma_Engine::PlayerActionEvent::ActionType::Interact);
            }
#ifdef _DEBUG_LOG
            std::cout << "Player Interact Action" << std::endl;
#endif
        }

        if (inputState.dashPressed && !lastDashState)
        {
            if (eventSystem)
            {
                eventSystem->Emit<Uma_Engine::PlayerActionEvent>(player, Uma_Engine::PlayerActionEvent::ActionType::Dash);
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