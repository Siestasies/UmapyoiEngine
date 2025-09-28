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

        auto& pArray = pCoordinator->GetComponentArray<Player>();
        auto& rbArray = pCoordinator->GetComponentArray<RigidBody>();
        auto& tfArray = pCoordinator->GetComponentArray<Transform>();

        Entity player = aEntities[0];
        auto& rb = pCoordinator->GetComponent<RigidBody>(player);

        Vec2 targetAccel = { 0, 0 };

        if (pInputSystem->KeyDown(GLFW_KEY_W)) targetAccel.y += rb.accel_strength;
        if (pInputSystem->KeyDown(GLFW_KEY_S)) targetAccel.y -= rb.accel_strength;
        if (pInputSystem->KeyDown(GLFW_KEY_D)) targetAccel.x += rb.accel_strength;
        if (pInputSystem->KeyDown(GLFW_KEY_A)) targetAccel.x -= rb.accel_strength;

        // Smooth acceleration to prevent jerk
        const float accelSmoothFactor = 15.f; // tweak for responsiveness
        rb.acceleration += (targetAccel - rb.acceleration) * accelSmoothFactor * dt;

        std::cout << "Player : ( " << magnitude(rb.velocity) << " )\n";

#ifdef _DEBUG_LOG
        std::cout << "Player : ( " << accel.x << " , " << accel.y << " )\n";
#endif // _DEBUG_LOG

    }
}