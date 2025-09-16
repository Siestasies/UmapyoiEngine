#include "PhysicsSystem.hpp"

#include "../Core/Coordinator.hpp"
#include "../Components/RigidBody.h"
#include "../Components/Transform.h"

#include <iostream>
#include <iomanip>

void Uma_ECS::PhysicsSystem::Update(float dt)
{
    for (auto const& entity : aEntities)
    {
        auto& rb = gCoordinator->GetComponent<RigidBody>(entity);
        auto& tf = gCoordinator->GetComponent<Transform>(entity);

        //tf.position += ...
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