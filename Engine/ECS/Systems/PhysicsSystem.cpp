#include "PhysicsSystem.hpp"

#include "../Core/Coordinator.hpp"
#include "../Components/RigidBody.h"
#include "../Components/Transform.h"


#define _DEBUG_LOG

#ifdef _DEBUG_LOG
#include <iostream>
#endif // _DEBUG_LOG

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
//#ifdef _DEBUG_LOG
//    for (auto const& entity : aEntities)
//    {
//        Signature sig = gCoordinator->GetEntitySignature(entity);
//
//        std::cout << "Entity [" << entity << "] has components: ";
//        bool first = true;
//
//        // Loop through each bit in the signature
//        for (size_t i = 0; i < sig.size(); i++)
//        {
//            if (sig.test(i)) // entity has this component
//            {
//                if (!first) std::cout << ", ";
//                std::cout << "ComponentType(" << i << ")";
//                first = false;
//            }
//        }
//
//        if (first)
//            std::cout << "None";
//
//        std::cout << "\n";
//    }
//#endif

#ifdef _DEBUG_LOG
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
    }
#endif
}