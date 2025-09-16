#include "Test_Ecs_System.h"

#include "ECS/Core/Coordinator.hpp"
#include "ECS/Systems/PhysicsSystem.hpp"
#include "ECS/Components/Transform.h"
#include "ECS/Components/RigidBody.h"

#include <vector>
#include <random>
#include <iostream>
#include <iomanip>

#define _DEBUG_LOG

using Coordinator = Uma_ECS::Coordinator;

Coordinator gCoordinator;
std::shared_ptr<Uma_ECS::PhysicsSystem> physicsSystem;

void Uma_Engine::Test_Ecs::Init()
{
    using namespace Uma_ECS;

    gCoordinator.Init();

    // register components
    gCoordinator.RegisterComponent<Transform>();
    gCoordinator.RegisterComponent<RigidBody>();

    physicsSystem = gCoordinator.RegisterSystem<PhysicsSystem>();
    {
        Signature sign;
        sign.set(gCoordinator.GetComponentType<RigidBody>());
        sign.set(gCoordinator.GetComponentType<Transform>());
        gCoordinator.SetSystemSignature<PhysicsSystem>(sign);
    }

    physicsSystem->Init(&gCoordinator);

    // entities
    std::vector<Entity> entities(2500);

    std::default_random_engine generator;
    std::uniform_real_distribution<float> randPosition(-100.0f, 100.0f);
    std::uniform_real_distribution<float> randRotation(0.0f, 3.0f);
    std::uniform_real_distribution<float> randScale(3.0f, 5.0f);

    float scale = randScale(generator);

    for (auto& entity : entities)
    {
        entity = gCoordinator.CreateEntity();

        gCoordinator.AddComponent(
            entity,
            RigidBody{
              .velocity = Vec2(0.0f, -1.0f),
              .acceleration = Vec2(0.0f, 0.0f)
            });

        gCoordinator.AddComponent(
            entity,
            Transform{
              .position = Vec2(randPosition(generator), randPosition(generator)),
              .rotation = Vec2(randRotation(generator), randRotation(generator)),
              .scale = Vec2(scale, scale)
            });
    }

#ifdef _DEBUG_LOG

    //physicsSystem->PrintLog();

#endif // _DEBUG_LOG

    // create entity

    Entity en = gCoordinator.CreateEntity();
    gCoordinator.AddComponent(
        en, 
        Transform
        {
            .position = Vec2(2,2),
            .rotation = Vec2(1,1),
            .scale = Vec2(3,3),
        });

  /*  gCoordinator.AddComponent(
        en,
        RigidBody{
          .velocity = Vec2(0.0f, 0.0f),
          .acceleration = Vec2(0.0f, 0.0f)
        });*/

#ifdef _DEBUG_LOG

    //physicsSystem->PrintLog();

#endif // _DEBUG_LOG
}

void Uma_Engine::Test_Ecs::Update(float dt)
{
    physicsSystem->Update(dt);

    //camera updatye 

    //..update
}

void Uma_Engine::Test_Ecs::Shutdown()
{
}
