#include "PlayerControllerSystem.hpp"

#include "../Components/RigidBody.h"
#include "../Components/Transform.h"
#include "../Components/Player.h"

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

        Entity player = *(aEntities.begin());

        auto& rb = pCoordinator->GetComponent<RigidBody>(player);
        auto& tf = pCoordinator->GetComponent<Transform>(player);
        auto& p = pCoordinator->GetComponent<Player>(player);

        Vec2& velocity = rb.velocity;
        velocity = { 0, 0 };

        if (pInputSystem->KeyPressed(GLFW_KEY_W) ||
            pInputSystem->KeyDown(GLFW_KEY_W))
        {
            velocity.y -= 1;
        }
        if (pInputSystem->KeyPressed(GLFW_KEY_A) ||
            pInputSystem->KeyDown(GLFW_KEY_A))
        {
            velocity.x -= 1;
        }
        if (pInputSystem->KeyPressed(GLFW_KEY_S) ||
            pInputSystem->KeyDown(GLFW_KEY_S))
        {
            velocity.y += 1;
        }
        if (pInputSystem->KeyPressed(GLFW_KEY_D) ||
            pInputSystem->KeyDown(GLFW_KEY_D))
        {
            velocity.x += 1;
        }

#ifdef _DEBUG_LOG
        std::cout << "Player : ( " << velocity.x << " , " << velocity.y << " )\n";
#endif // _DEBUG_LOG

    }
}