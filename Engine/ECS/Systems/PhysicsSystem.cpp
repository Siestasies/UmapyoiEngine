/*!
\file   PhysicsSystem.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

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

#include <iostream>
#include <iomanip>

void Uma_ECS::PhysicsSystem::Update(float dt)
{
    auto& rbArray = gCoordinator->GetComponentArray<RigidBody>();
    auto& tfArray = gCoordinator->GetComponentArray<Transform>();

    for (auto const& entity : aEntities)
    {
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