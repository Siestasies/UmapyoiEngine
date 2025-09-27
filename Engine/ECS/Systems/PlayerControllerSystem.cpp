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
        //if (aEntities.empty()) return;

        // by right shd only have 1 player

        auto& pArray = pCoordinator->GetComponentArray<Player>();

        if (pArray.Size() == 0)
        {
            return;
        }

        Entity player = pArray.GetEntity(0);

        auto& rb = pCoordinator->GetComponent<RigidBody>(player);
        auto& tf = pCoordinator->GetComponent<Transform>(player);
        auto& p = pCoordinator->GetComponent<Player>(player);

        Vec2& accel = rb.acceleration;
        accel = { 0, 0 };

        if (pInputSystem->KeyPressed(GLFW_KEY_W) ||
            pInputSystem->KeyDown(GLFW_KEY_W))
        {
            accel.y += rb.accel_strength;
        }
        if (pInputSystem->KeyPressed(GLFW_KEY_A) ||
            pInputSystem->KeyDown(GLFW_KEY_A))
        {
            accel.x -= rb.accel_strength;
        }
        if (pInputSystem->KeyPressed(GLFW_KEY_S) ||
            pInputSystem->KeyDown(GLFW_KEY_S))
        {
            accel.y -= rb.accel_strength;
        }
        if (pInputSystem->KeyPressed(GLFW_KEY_D) ||
            pInputSystem->KeyDown(GLFW_KEY_D))
        {
            accel.x += rb.accel_strength;
        }

#ifdef _DEBUG_LOG
        std::cout << "Player : ( " << accel.x << " , " << accel.y << " )\n";
#endif // _DEBUG_LOG

    }
}