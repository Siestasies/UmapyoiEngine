/*!
\file   PhysicsSystem.cpp
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Implements Verlet-based physics integration with semi-implicit Euler method for velocity updates.

Applies acceleration to velocity, exponential friction damping, and epsilon-based velocity clamping to prevent jitter.
Stores previous position in Transform before updating for collision system's swept tests.
Includes debug logging method (PrintLog) that outputs entity signatures and component data for Transform and RigidBody
to console with formatted output showing total entity counts and system membership.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "PhysicsSystem.hpp"

#include "../Core/Coordinator.hpp"
#include "../Components/RigidBody.h"
#include "../Components/Transform.h"

#include "CollisionSystem.hpp"

#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>

#include "HybridInputSystem.h"

void Uma_ECS::PhysicsSystem::Update(float dt)
{
    Uma_Engine::HybridInputSystem::GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_DPAD_UP, GLFW_PRESS, 0);
    Uma_Engine::HybridInputSystem::GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_DPAD_UP, GLFW_REPEAT, 0);
    Uma_Engine::HybridInputSystem::GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_DPAD_UP, GLFW_RELEASE, 0);

    Uma_Engine::HybridInputSystem::GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_DPAD_DOWN, GLFW_PRESS, 0);
    Uma_Engine::HybridInputSystem::GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_DPAD_DOWN, GLFW_REPEAT, 0);
    Uma_Engine::HybridInputSystem::GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_DPAD_DOWN, GLFW_RELEASE, 0);

    Uma_Engine::HybridInputSystem::GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_DPAD_LEFT, GLFW_PRESS, 0);
    Uma_Engine::HybridInputSystem::GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_DPAD_LEFT, GLFW_REPEAT, 0);
    Uma_Engine::HybridInputSystem::GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_DPAD_LEFT, GLFW_RELEASE, 0);

    Uma_Engine::HybridInputSystem::GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, GLFW_PRESS, 0);
    Uma_Engine::HybridInputSystem::GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, GLFW_REPEAT, 0);
    Uma_Engine::HybridInputSystem::GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_DPAD_RIGHT, GLFW_RELEASE, 0);

    Uma_Engine::HybridInputSystem::GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_A, GLFW_PRESS, 0);
    Uma_Engine::HybridInputSystem::GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_A, GLFW_REPEAT, 0);
    Uma_Engine::HybridInputSystem::GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_A, GLFW_RELEASE, 0);

    Uma_Engine::HybridInputSystem::GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_B, GLFW_PRESS, 0);
    Uma_Engine::HybridInputSystem::GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_B, GLFW_REPEAT, 0);
    Uma_Engine::HybridInputSystem::GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_B, GLFW_RELEASE, 0);

    Uma_Engine::HybridInputSystem::GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_X, GLFW_PRESS, 0);
    Uma_Engine::HybridInputSystem::GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_X, GLFW_REPEAT, 0);
    Uma_Engine::HybridInputSystem::GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_X, GLFW_RELEASE, 0);

    Uma_Engine::HybridInputSystem::GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_Y, GLFW_PRESS, 0);
    Uma_Engine::HybridInputSystem::GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_Y, GLFW_REPEAT, 0);
    Uma_Engine::HybridInputSystem::GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_Y, GLFW_RELEASE, 0);

    Uma_Engine::HybridInputSystem::GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, GLFW_PRESS, 0);
    Uma_Engine::HybridInputSystem::GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, GLFW_REPEAT, 0);
    Uma_Engine::HybridInputSystem::GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_LEFT_BUMPER, GLFW_RELEASE, 0);

    Uma_Engine::HybridInputSystem::GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, GLFW_PRESS, 0);
    Uma_Engine::HybridInputSystem::GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, GLFW_REPEAT, 0);
    Uma_Engine::HybridInputSystem::GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER, GLFW_RELEASE, 0);

    Uma_Engine::HybridInputSystem::GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_LEFT_THUMB, GLFW_PRESS, 0);
    Uma_Engine::HybridInputSystem::GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_LEFT_THUMB, GLFW_REPEAT, 0);
    Uma_Engine::HybridInputSystem::GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_LEFT_THUMB, GLFW_RELEASE, 0);

    Uma_Engine::HybridInputSystem::GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_RIGHT_THUMB, GLFW_PRESS, 0);
    Uma_Engine::HybridInputSystem::GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_RIGHT_THUMB, GLFW_REPEAT, 0);
    Uma_Engine::HybridInputSystem::GetControllerButtonInput(GLFW_GAMEPAD_BUTTON_RIGHT_THUMB, GLFW_RELEASE, 0);

    Uma_Engine::HybridInputSystem::GetControllerAxesInput(GLFW_GAMEPAD_AXIS_LEFT_X, 0);
    Uma_Engine::HybridInputSystem::GetControllerAxesInput(GLFW_GAMEPAD_AXIS_LEFT_Y, 0);
    Uma_Engine::HybridInputSystem::GetControllerAxesInput(GLFW_GAMEPAD_AXIS_RIGHT_X, 0);
    Uma_Engine::HybridInputSystem::GetControllerAxesInput(GLFW_GAMEPAD_AXIS_RIGHT_Y, 0);
    Uma_Engine::HybridInputSystem::GetControllerAxesInput(GLFW_GAMEPAD_AXIS_LEFT_TRIGGER, 0);
    Uma_Engine::HybridInputSystem::GetControllerAxesInput(GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER, 0);

    auto& rbArray = gCoordinator->GetComponentArray<RigidBody>();
    auto& tfArray = gCoordinator->GetComponentArray<Transform>();

    for (auto const& entity : aEntities)
    {
        if (!gCoordinator->IsActiveInHierarchy(entity))
            continue;

        auto& rb = rbArray.GetData(entity);
        auto& tf = tfArray.GetData(entity);

        // Store previous position
        //tf.prevPos = tf.position;

        // Rotation update
        tf.rotation.x += tf.rotation.y;

        // Apply acceleration to velocity
        rb.velocity += rb.acceleration * dt;

        // Apply friction
        rb.velocity *= std::exp(-rb.fric_coeff * dt);

        // Clamp small velocities
        const float epsilon = 0.1f;
        if (std::abs(rb.velocity.x) < epsilon) rb.velocity.x = 0.f;
        if (std::abs(rb.velocity.y) < epsilon) rb.velocity.y = 0.f;
    }
}

void Uma_ECS::PhysicsSystem::SavePrevPos()
{
    auto& tfArray = gCoordinator->GetComponentArray<Transform>();
    for (size_t i = 0; i < tfArray.Size(); ++i)
    {
        auto& tf = tfArray.GetComponentAt(i);
        tf.prevPos = tf.position;  // Save current as previous
        tf.prevWorldPos = tf.worldPosition;
    }
}

//Apply position after collision resolution
void Uma_ECS::PhysicsSystem::ApplyVelocity(float dt)
{
    auto& rbArray = gCoordinator->GetComponentArray<RigidBody>();
    auto& tfArray = gCoordinator->GetComponentArray<Transform>();

    for (auto const& entity : aEntities)
    {
        auto& rb = rbArray.GetData(entity);
        auto& tf = tfArray.GetData(entity);

        //tf.prevPos = tf.position;

        // Now apply the (collision-corrected) velocity to position
        tf.position += rb.velocity * dt;
    }
}

void Uma_ECS::PhysicsSystem::PrintLog()
{

    std::cout << std::setw(100) << std::setfill('-') << "\n";
    std::cout << std::setw(100) << std::setfill('-') << "\n";

    int en_cnt = 0;
    for (auto const& entity : aEntities)
    {
        Signature sig = gCoordinator->GetEntitySignature(entity);

        std::cout << "Entity [" << entity << "] has components:\n";

        bool hasAny = false;

        // Transform component
        if (sig.test(gCoordinator->GetComponentType<Transform>()))
        {
            hasAny = true;
            auto& tf = gCoordinator->GetComponent<Transform>(entity);
            std::cout << "  Transform { "
                << "position: (" << tf.position.x << ", " << tf.position.y << "), "
                << "rotation: (" << tf.rotation.x << ", " << tf.rotation.y << "), "
                << "scale: (" << tf.scale.x << ", " << tf.scale.y << ") }\n";
        }

        // RigidBody component
        if (sig.test(gCoordinator->GetComponentType<RigidBody>()))
        {
            hasAny = true;
            auto& rb = gCoordinator->GetComponent<RigidBody>(entity);
            std::cout << "  RigidBody { "
                << "velocity: (" << rb.velocity.x << ", " << rb.velocity.y << "), "
                << "acceleration: (" << rb.acceleration.x << ", " << rb.acceleration.y << ") }\n";
        }

        if (!hasAny)
            std::cout << "  None\n";

        en_cnt++;
    }
    std::cout << "\n";
    std::cout << "Total entities that are having the same signature as the system : " << en_cnt << "\n";
    std::cout << "Total entities in the scene : " << gCoordinator->GetEntityCount() << "\n";

    std::cout << std::setw(100) << std::setfill('-') << "\n";
    std::cout << std::setw(100) << std::setfill('-') << "\n";
}

void Uma_ECS::PhysicsSystem::AddForce(Entity entity, Vec2 pos, Vec2 dir, float force, float rotation)
{
    if (!gCoordinator->IsActiveInHierarchy(entity)) return;

    auto& tf = gCoordinator->GetComponent<Transform>(entity);
    auto& rb = gCoordinator->GetComponent<RigidBody>(entity);

    
    tf.position = pos;
    tf.rotation = rotation;

    rb.velocity = dir * force;
}

std::unordered_set<Uma_ECS::Entity> Uma_ECS::PhysicsSystem::OverlapCircle(Vec2 center, float radius)
{
    CollisionSystem* collisionSystem = gCoordinator->GetSystem<CollisionSystem>().get();

    Vec2 boundMin = { center.x - radius, center.y - radius };
    Vec2 boundMax = { center.x + radius, center.y + radius };

    return collisionSystem->GetEntitiesInArea(boundMin, boundMax);
}

bool Uma_ECS::PhysicsSystem::LineOfSight(Entity lhs, Entity rhs)
{
    auto& lhs_tf = gCoordinator->GetComponent<Transform>(lhs);
    auto& rhs_tf = gCoordinator->GetComponent<Transform>(rhs);

    Vec2 origin = lhs_tf.worldPosition;
    Vec2 toTarget = rhs_tf.worldPosition - origin;
    float maxDist = Uma_Math::magnitude(toTarget);

    if (maxDist < 0.001f)
        return true;

    toTarget.normalize();

    Vec2 dir = toTarget;

    Uma_ECS::RaycastHit hit = RayCast(origin, dir, maxDist);

    if (!hit.hit)   // nothing blocking the ray
        return true;

    if (hit.entity == lhs || hit.entity == rhs) // hit lhs or rhs
        return true;

    if (hit.colliderLayer & CL_WALL)    // hit a wall return false
        return false;

    return true;
}

Uma_ECS::RaycastHit Uma_ECS::PhysicsSystem::RayCast(Vec2 origin, Vec2 dir, float maxDist)
{
    RaycastHit result{};
    result.hit = false;
    result.distance = maxDist;

    // Compute the ray's bounding box for spatial grid query
    Vec2 endPoint = { origin.x + dir.x * maxDist, origin.y + dir.y * maxDist };
    Vec2 areaMin = { std::min(origin.x, endPoint.x), std::min(origin.y, endPoint.y) };
    Vec2 areaMax = { std::max(origin.x, endPoint.x), std::max(origin.y, endPoint.y) };

    // Get candidate entities from spatial grid
    CollisionSystem* collisionSystem = gCoordinator->GetSystem<CollisionSystem>().get();
    auto candidates = collisionSystem->GetEntitiesInArea(areaMin, areaMax);

    auto& cArray = gCoordinator->GetComponentArray<Collider>();

    for (auto entity : candidates)
    {
        if (!gCoordinator->IsActiveInHierarchy(entity))
            continue;

        auto& collider = cArray.GetData(entity);

        for (size_t i = 0; i < collider.shapes.size(); ++i)
        {
            if (!collider.shapes[i].isActive)
                continue;

            const BoundingBox& box = collider.bounds[i];

            // Slab method: ray vs AABB
            float tMin = 0.0f;
            float tMax = maxDist;

            // X axis slab
            if (std::abs(dir.x) > 0.0001f)
            {
                float invDirX = 1.0f / dir.x;                   // optimise if not have to div by dir
                float t1 = (box.min.x - origin.x) * invDirX;    // time needed for origin to reach box.min.x
                float t2 = (box.max.x - origin.x) * invDirX;    //  time needed for origin to reach box.max.x

                if (t1 > t2) std::swap(t1, t2);                 // t1 = earliest, t2 = later

                tMin = std::max(tMin, t1);
                tMax = std::min(tMax, t2);

                if (tMin > tMax)                                // skip cus t is more then max dist
                    continue;
            }
            else
            {
                // Ray parallel to Y axis — check if origin.x is inside the box
                if (origin.x < box.min.x || origin.x > box.max.x)
                    continue;
            }

            // Y axis slab
            if (std::abs(dir.y) > 0.0001f)
            {
                float invDirY = 1.0f / dir.y;
                float t1 = (box.min.y - origin.y) * invDirY;
                float t2 = (box.max.y - origin.y) * invDirY;

                if (t1 > t2) std::swap(t1, t2);

                tMin = std::max(tMin, t1);
                tMax = std::min(tMax, t2);

                if (tMin > tMax)
                    continue;
            }
            else
            {
                if (origin.y < box.min.y || origin.y > box.max.y)
                    continue;
            }

            // Hit confirmed — check if it's the closest
            if (tMin < result.distance)
            {
                result.hit = true;
                result.distance = tMin;
                result.entity = entity;
                result.point = { origin.x + dir.x * tMin, origin.y + dir.y * tMin };
                result.colliderLayer = collider.GetEffectiveLayer(i);

                // Normal: determined by which slab the ray entered
                float tX = (std::abs(dir.x) > 0.0001f)
                    ? (std::max(0.0f, ((dir.x > 0 ? box.min.x : box.max.x) - origin.x) / dir.x))
                    : -1.0f;
                float tY = (std::abs(dir.y) > 0.0001f)
                    ? (std::max(0.0f, ((dir.y > 0 ? box.min.y : box.max.y) - origin.y) / dir.y))
                    : -1.0f;

                if (tX > tY)
                    result.normal = { (dir.x > 0) ? -1.0f : 1.0f, 0.0f };
                else
                    result.normal = { 0.0f, (dir.y > 0) ? -1.0f : 1.0f };
            }
        }
    }

    return result;
}
