#include "PlayerControllerSystem.hpp"

#include "../Components/RigidBody.h"
#include "../Components/Transform.h"
#include "../Components/Player.h"

#include "PlayerEvents.h"

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

		Entity player = *(aEntities.begin());

		auto& rb = pCoordinator->GetComponent<RigidBody>(player);
		auto& tf = pCoordinator->GetComponent<Transform>(player);
		auto& p = pCoordinator->GetComponent<Player>(player);

		Vec2& velocity = rb.velocity;
		velocity = {0, 0};

		if (inputState.moveRight) velocity.x += 1.0f;
		if (inputState.moveLeft) velocity.x -= 1.0f;
		if (inputState.moveUp) velocity.y += 1.0f;
		if (inputState.moveDown) velocity.y -= 1.0f;

		// Normalize diagonal movement
		if (velocity.x != 0.0f && velocity.y != 0.0f) velocity.normalize();

		// Only emit movement event if there's actual movement or change
		static float lastVelocityX = 0.0f;
		static float lastVelocityY = 0.0f;

		if (velocity.x != lastVelocityX || velocity.y != lastVelocityY)
		{
			if (eventSystem)
			{
				eventSystem->Emit<Uma_Engine::PlayerMoveEvent>(player, velocity.x, velocity.y);
			}

			// Also directly update the player for immediate response
			UpdatePlayerMovement(player, velocity.x, velocity.y);

			lastVelocityX = velocity.x;
			lastVelocityY = velocity.y;

#ifdef _DEBUG_LOG
			std::cout << "Player Movement: " << velocity << std::endl;
#endif
		}
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

	void PlayerControllerSystem::UpdatePlayerMovement(Entity player, float velocityX, float velocityY)
	{
		auto& rb = pCoordinator->GetComponent<RigidBody>(player);
		auto& p = pCoordinator->GetComponent<Player>(player);

		rb.velocity.x = velocityX * p.mSpeed;
		rb.velocity.y = velocityY * p.mSpeed;
	}
}