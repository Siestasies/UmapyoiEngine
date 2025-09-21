#include "Test_Ecs_System.h"

// ECS Core
#include "ECS/Core/Coordinator.hpp"

// ECS Systems
#include "ECS/Systems/PhysicsSystem.hpp"
#include "ECS/Systems/PlayerControllerSystem.hpp"
#include "ECS/Systems/RenderingSystem.hpp"

// ECS Components
#include "ECS/Components/Transform.h"
#include "ECS/Components/RigidBody.h"
#include "ECS/Components/Player.h"
#include "ECS/Components/SpriteRenderer.h"

// Engine Systems
#include "Systems/InputSystem.h"
#include "Systems/Graphics.hpp"
#include "Systems/ResourcesManager.hpp"
#include "../Core/SystemManager.h"
#include "../Core/EventSystem.h"
#include "../Core/EventTypes.h"

#include <vector>
#include <random>
#include <iostream>
#include <iomanip>

#define _DEBUG_LOG

using Coordinator = Uma_ECS::Coordinator;

Coordinator gCoordinator;
std::shared_ptr<Uma_ECS::PhysicsSystem> physicsSystem;
std::shared_ptr<Uma_ECS::PlayerControllerSystem> playerController;
std::shared_ptr<Uma_ECS::RenderingSystem> renderingSystem;

Uma_Engine::InputSystem* pInputSystem;
Uma_Engine::Graphics* pGraphics;
Uma_Engine::ResourcesManager* pResourcesManager;
Uma_Engine::EventSystem* pEventSystem;


void Uma_Engine::Test_Ecs::Init()
{
    pInputSystem = pSystemManager->GetSystem<InputSystem>();
    pGraphics = pSystemManager->GetSystem<Graphics>();
    pResourcesManager = pSystemManager->GetSystem<ResourcesManager>();
    pEventSystem = pSystemManager->GetSystem<EventSystem>();

    // event system stuff

    //subscribe to events
    pEventSystem->Subscribe<Uma_Engine::Events::EntityCreatedEvent>(
        [](const Uma_Engine::Events::EntityCreatedEvent& e) {
            std::cout << "Entity created: " << e.entityId << std::endl;
            // e.handled = true;
        });


    // Load textures using ResourcesManager
    pResourcesManager->LoadTexture("player", "Assets/hello.jpg");
    pResourcesManager->LoadTexture("enemy", "Assets/white.png");
    pResourcesManager->LoadTexture("background", "Assets/background.jpg");
    pResourcesManager->PrintLoadedTextureNames();


    // Ecs stuff
    using namespace Uma_ECS;

    gCoordinator.Init(pEventSystem);

    // register components
    gCoordinator.RegisterComponent<Transform>();
    gCoordinator.RegisterComponent<RigidBody>();
    gCoordinator.RegisterComponent<Player>();
    gCoordinator.RegisterComponent<SpriteRenderer>();

    // Physics System
    physicsSystem = gCoordinator.RegisterSystem<PhysicsSystem>();
    {
        Signature sign;
        sign.set(gCoordinator.GetComponentType<RigidBody>());
        sign.set(gCoordinator.GetComponentType<Transform>());
        gCoordinator.SetSystemSignature<PhysicsSystem>(sign);
    }
    physicsSystem->Init(&gCoordinator);

    // Player controller
    playerController = gCoordinator.RegisterSystem<PlayerControllerSystem>();
    {
        Signature sign;
        sign.set(gCoordinator.GetComponentType<RigidBody>());
        sign.set(gCoordinator.GetComponentType<Transform>());
        sign.set(gCoordinator.GetComponentType<Player>());
        gCoordinator.SetSystemSignature<PlayerControllerSystem>(sign);
    }
    playerController->Init(pInputSystem, &gCoordinator);

    // Rendering System
    renderingSystem = gCoordinator.RegisterSystem<RenderingSystem>();
    {
        Signature sign;
        sign.set(gCoordinator.GetComponentType<SpriteRenderer>());
        sign.set(gCoordinator.GetComponentType<Transform>());
        gCoordinator.SetSystemSignature<RenderingSystem>(sign);
    }
    renderingSystem->Init(pGraphics, &gCoordinator);

    // create entities
    {
        //std::vector<Entity> entities(2500);

        std::default_random_engine generator;
        std::uniform_real_distribution<float> randPositionX(0.f, 1920.f);
        std::uniform_real_distribution<float> randPositionY(0.f, 1080.f);
        std::uniform_real_distribution<float> randRotation(0.0f, 0.0f);
        std::uniform_real_distribution<float> randScale(5.0f, 15.0f);

        Entity enemy;

        {
            enemy = gCoordinator.CreateEntity();

            gCoordinator.AddComponent(
                enemy,
                RigidBody{
                  .velocity = Vec2(0.0f, -1.0f),
                  .acceleration = Vec2(0.0f, 0.0f)
                });

            gCoordinator.AddComponent(
                enemy,
                Transform{
                  .position = Vec2(randPositionX(generator), randPositionY(generator)),
                  .rotation = Vec2(randRotation(generator), randRotation(generator)),
                  .scale = Vec2(randScale(generator), randScale(generator))
                });

            gCoordinator.AddComponent(
                enemy,
                SpriteRenderer{
                  .texture = pResourcesManager->GetTexture("player"),
                  .flipX = false,
                  .flipY = false
                });
        }
        
        // using 1 enemy to duplicate 2500 times and rand its transform
        for (size_t i = 0; i < 2500; i++)
        {
            Entity tmp = gCoordinator.DuplicateEntity(enemy);

            Transform& tf = gCoordinator.GetComponent<Transform>(tmp);

            tf.position = Vec2(randPositionX(generator), randPositionY(generator));
            tf.rotation = Vec2(randRotation(generator), randRotation(generator));
            tf.scale = Vec2(randScale(generator), randScale(generator));
        }

        /*for (auto& entity : entities)
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
                  .position = Vec2(randPositionX(generator), randPositionY(generator)),
                  .rotation = Vec2(randRotation(generator), randRotation(generator)),
                  .scale = Vec2(scale, scale)
                });

            gCoordinator.AddComponent(
                entity,
                SpriteRenderer{
                  .texture = pResourcesManager->GetTexture("player"),
                  .flipX = false,
                  .flipY = false
                });
        }*/
    }

    // create player
    Entity en = gCoordinator.CreateEntity();
    gCoordinator.AddComponent(
        en, 
        Transform
        {
            .position = Vec2(400.0f, 300.0f),
            .rotation = Vec2(1,1),
            .scale = Vec2(50,50),
        });

    gCoordinator.AddComponent(
        en,
        RigidBody{
          .velocity = Vec2(0.0f, 0.0f),
          .acceleration = Vec2(0.0f, 0.0f)
        });

    gCoordinator.AddComponent(
        en,
        Player{});

    gCoordinator.AddComponent(
        en,
        SpriteRenderer{
          .texture = pResourcesManager->GetTexture("player"),
          .flipX = false,
          .flipY = false
        });
}

void Uma_Engine::Test_Ecs::Update(float dt)
{
    physicsSystem->Update(dt);

    playerController->Update(dt);

    pGraphics->ClearBackground(0.2f, 0.3f, 0.3f);
    pGraphics->DrawBackground(pResourcesManager->GetTexture("background")->tex_id, pResourcesManager->GetTexture("background")->tex_size);
    renderingSystem->Update(dt);


    //camera updatye 

    //..update
}

void Uma_Engine::Test_Ecs::Shutdown()
{
}
