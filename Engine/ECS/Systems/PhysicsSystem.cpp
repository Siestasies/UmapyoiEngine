#include "PhysicsSystem.hpp"

#include "../Core/Coordinator.hpp"
#include "../Components/RigidBody.h"
#include "../Components/Transform.h"

#include <iostream>
#include <iomanip>

void Uma_ECS::PhysicsSystem::Update(float dt)
{

    // THIS IS OLD METHOD WHICH IS EXPENSIVE
    // NOT IN USED
    {
    //for (auto const& entity : aEntities)
    //{
    //    auto& rb = gCoordinator->GetComponent<RigidBody>(entity);
    //    auto& tf = gCoordinator->GetComponent<Transform>(entity);

    //    tf.position += rb.velocity;

    //    //std::cout << "physics updating\n";
    //}
    }

    // Get dense component arrays once
    auto& rbArray = gCoordinator->GetComponentArray<RigidBody>();
    auto& tfArray = gCoordinator->GetComponentArray<Transform>();

    if (rbArray.Size() == 0 ||
        tfArray.Size() == 0)
    {
        return;
    }

    // Iterate over the smaller array for efficiency (here, RigidBody)
    for (size_t i = 0; i < rbArray.Size(); ++i)
    {
        Entity e = rbArray.GetEntity(i);

        if (tfArray.Has(e))  // check if Transform exists
        {
            auto& rb = rbArray.GetComponentAt(i);
            auto& tf = tfArray.GetData(e);

            // set prev pos
            tf.prevPos = tf.position;

            rb.velocity += rb.acceleration * dt;

            rb.velocity -= rb.velocity * rb.fric_coeff * dt;

            tf.position += rb.velocity * dt;

            rb.acceleration = { 0,0 };

            // tmp
            /*if (tf.position.y <= 0)
            {
                tf.position.y += 1080.f;
            }*/
        }
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