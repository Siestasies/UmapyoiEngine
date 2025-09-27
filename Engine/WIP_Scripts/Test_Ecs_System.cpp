#include "Test_Ecs_System.h"

// ECS Core
#include "ECS/Core/Coordinator.hpp"

// ECS Systems
#include "ECS/Systems/PhysicsSystem.hpp"
#include "ECS/Systems/PlayerControllerSystem.hpp"
#include "ECS/Systems/RenderingSystem.hpp"
#include "ECS/Systems/CollisionSystem.hpp"

// ECS Components
#include "ECS/Components/Transform.h"
#include "ECS/Components/RigidBody.h"
#include "ECS/Components/Player.h"
#include "ECS/Components/SpriteRenderer.h"
#include "ECS/Components/Collider.h"
#include "ECS/Components/Camera.h"

// Engine Systems
#include "Systems/InputSystem.h"
#include "Systems/Graphics.hpp"
#include "Systems/Sound.hpp"
#include "Systems/ResourcesManager.hpp"
#include "Systems/CameraSystem.hpp"
#include "../Core/SystemManager.h"
#include "../Core/EventSystem.h"
#include "../Core/EventTypes.h"

#include <vector>
#include <random>
#include <iostream>
#include <iomanip>

#include <GLFW/glfw3.h>

#define _DEBUG_LOG

using Coordinator = Uma_ECS::Coordinator;

Coordinator gCoordinator;
std::shared_ptr<Uma_ECS::PhysicsSystem> physicsSystem;
std::shared_ptr<Uma_ECS::CollisionSystem> collisionSystem;
std::shared_ptr<Uma_ECS::PlayerControllerSystem> playerController;
std::shared_ptr<Uma_ECS::RenderingSystem> renderingSystem;
std::shared_ptr<Uma_ECS::CameraSystem> cameraSystem;

Uma_Engine::InputSystem* pInputSystem;
Uma_Engine::Graphics* pGraphics;
Uma_Engine::Sound* pSound;
Uma_Engine::ResourcesManager* pResourcesManager;
Uma_Engine::EventSystem* pEventSystem;

Uma_ECS::Entity player;
Uma_ECS::Entity cam;


void Uma_Engine::Test_Ecs::Init()
{
    pInputSystem = pSystemManager->GetSystem<InputSystem>();
    pGraphics = pSystemManager->GetSystem<Graphics>();
    pResourcesManager = pSystemManager->GetSystem<ResourcesManager>();
    pEventSystem = pSystemManager->GetSystem<EventSystem>();
    pSound = pSystemManager->GetSystem<Sound>();

    // event system stuffs
    //subscribe to events
    pEventSystem->Subscribe<Uma_Engine::Events::EntityCreatedEvent>(
        [](const Uma_Engine::Events::EntityCreatedEvent& e) {
            //std::cout << "Entity created: " << e.entityId << std::endl;
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
    gCoordinator.RegisterComponent<Collider>();
    gCoordinator.RegisterComponent<SpriteRenderer>();
    gCoordinator.RegisterComponent<Camera>();

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

    // Physics System
    physicsSystem = gCoordinator.RegisterSystem<PhysicsSystem>();
    {
        Signature sign;
        sign.set(gCoordinator.GetComponentType<RigidBody>());
        sign.set(gCoordinator.GetComponentType<Transform>());
        gCoordinator.SetSystemSignature<PhysicsSystem>(sign);
    }
    physicsSystem->Init(&gCoordinator);

    // Collision System
    collisionSystem = gCoordinator.RegisterSystem<CollisionSystem>();
    {
        Signature sign;
        sign.set(gCoordinator.GetComponentType<RigidBody>());
        sign.set(gCoordinator.GetComponentType<Transform>());
        sign.set(gCoordinator.GetComponentType<Collider>());
        gCoordinator.SetSystemSignature<CollisionSystem>(sign);
    }
    collisionSystem->Init(&gCoordinator);


    // Rendering System
    renderingSystem = gCoordinator.RegisterSystem<RenderingSystem>();
    {
        Signature sign;
        sign.set(gCoordinator.GetComponentType<SpriteRenderer>());
        sign.set(gCoordinator.GetComponentType<Transform>());
        gCoordinator.SetSystemSignature<RenderingSystem>(sign);
    }
    renderingSystem->Init(pGraphics, &gCoordinator);

    cameraSystem = gCoordinator.RegisterSystem<CameraSystem>();
    {
        Signature sign;
        sign.set(gCoordinator.GetComponentType<Camera>());
        sign.set(gCoordinator.GetComponentType<Transform>());
        gCoordinator.SetSystemSignature<CameraSystem>(sign);
    }
    cameraSystem->Init(&gCoordinator);

    // create entities
    {
        //std::vector<Entity> entities(2500);

        std::default_random_engine generator;
        std::uniform_real_distribution<float> randPositionX(0.f, 1920.f);
        std::uniform_real_distribution<float> randPositionY(0.f, 1080.f);
        std::uniform_real_distribution<float> randRotation(0.0f, 0.0f);
        std::uniform_real_distribution<float> randScale(10.0f, 15.0f);

        Entity enemy;

        {
            enemy = gCoordinator.CreateEntity();

            gCoordinator.AddComponent(
                enemy,
                RigidBody{
                  .velocity = Vec2(0.0f, 0.0f),
                  .acceleration = Vec2(0.0f, 0.0f),
                  .accel_strength = 200,
                  .fric_coeff = 100
                });

            gCoordinator.AddComponent(
                enemy,
                Transform{
                  .position = Vec2(randPositionX(generator), randPositionY(generator)),
                  .rotation = Vec2(randRotation(generator), randRotation(generator)),
                  .scale = Vec2(randScale(generator), randScale(generator))
                });

            std::string texName = "enemy";
            gCoordinator.AddComponent(
                enemy,
                SpriteRenderer{
                  .textureName = texName,
                  .flipX = false,
                  .flipY = false,
                  .texture = pResourcesManager->GetTexture(texName),
                });


            gCoordinator.AddComponent(
                enemy,
                Collider{
                  .layer = CollisionLayer::CL_ENEMY,
                  .colliderMask = CollisionLayer::CL_ENEMY | CollisionLayer::CL_PLAYER | CollisionLayer::CL_WALL | CollisionLayer::CL_PROJECTILE | CollisionLayer::CL_WALL
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
    }

    // create player
    player = gCoordinator.CreateEntity();

    {
        gCoordinator.AddComponent(
            player,
            Transform
            {
                .position = Vec2(400.0f, 300.0f),
                .rotation = Vec2(1,1),
                .scale = Vec2(50,50),
            });

        gCoordinator.AddComponent(
            player,
            RigidBody{
              .velocity = Vec2(0.0f, 0.0f),
              .acceleration = Vec2(0.0f, 0.0f),
              .accel_strength = 2500,
              .fric_coeff = 5
            });

        gCoordinator.AddComponent(
            player,
            Player{});

        std::string texName = "player";
        gCoordinator.AddComponent(
            player,
            SpriteRenderer{
              .textureName = texName,
              .flipX = false,
              .flipY = false,
              .texture = pResourcesManager->GetTexture(texName),
            });

        gCoordinator.AddComponent(
            player,
            Collider{
              .layer = CollisionLayer::CL_PLAYER,
              .colliderMask = CollisionLayer::CL_ENEMY | CollisionLayer::CL_WALL | CollisionLayer::CL_PROJECTILE | CollisionLayer::CL_WALL
            });
    }
    
    // create camera
    cam = gCoordinator.CreateEntity();
    {
        gCoordinator.AddComponent(
            cam,
            Transform
            {
                .position = Vec2(400.0f, 300.0f),
                .rotation = Vec2(0,0),
                .scale = Vec2(1,1),
            });

        gCoordinator.AddComponent(
            player,
            Camera
            {
                .mZoom = 1.f,
                .followPlayer = true
            });
    }

    // serialize to file
    gCoordinator.SerializeAllEntities("Assets/Scenes/data.json");
}

void Uma_Engine::Test_Ecs::Update(float dt)
{
    playerController->Update(dt);

    physicsSystem->Update(dt);

    collisionSystem->Update(dt);

    cameraSystem->Update(dt);

    // play sound
    if (pInputSystem->KeyPressed(GLFW_KEY_9))
    {
        // moo moo
    }

    pGraphics->ClearBackground(0.2f, 0.3f, 0.3f);
    //pGraphics->DrawBackground(pResourcesManager->GetTexture("background")->tex_id, pResourcesManager->GetTexture("background")->tex_size);
    renderingSystem->Update(dt);


    //camera updatye 

    //..update
}

void Uma_Engine::Test_Ecs::Shutdown()
{
}
