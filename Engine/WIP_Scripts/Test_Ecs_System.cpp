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
#include "Systems/Sound.hpp"
#include "Systems/ResourcesManager.hpp"
#include "../Core/SystemManager.h"
#include "../Core/EventSystem.h"
#include "../Core/EventTypes.h"

#include <vector>
#include <random>
#include <iostream>
#include <iomanip>
#include <stdexcept>

#include <GLFW/glfw3.h>

#define _DEBUG_LOG

using Coordinator = Uma_ECS::Coordinator;

Coordinator gCoordinator;
std::shared_ptr<Uma_ECS::PhysicsSystem> physicsSystem;
std::shared_ptr<Uma_ECS::PlayerControllerSystem> playerController;
std::shared_ptr<Uma_ECS::RenderingSystem> renderingSystem;

Uma_Engine::InputSystem* pInputSystem;
Uma_Engine::Graphics* pGraphics;
Uma_Engine::Sound* pSound;
Uma_Engine::ResourcesManager* pResourcesManager;
Uma_Engine::EventSystem* pEventSystem;

Uma_ECS::Entity player;

//testing sound remove later
bool paused = true;
float volume = 0.f;

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
            std::cout << "Entity created: " << e.entityId << std::endl;
            // e.handled = true;
        });


    // Load textures using ResourcesManager
    pResourcesManager->LoadTexture("player", "Assets/hello.jpg");
    pResourcesManager->LoadTexture("enemy", "Assets/white.png");
    pResourcesManager->LoadTexture("background", "Assets/background.jpg");
    pResourcesManager->PrintLoadedTextureNames();

    pResourcesManager->LoadSound("explosion", "Assets/sounds/explosion.mp3", SoundType::SFX);
    pResourcesManager->LoadSound("cave", "Assets/sounds/cave.mp3", SoundType::BGM);

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
                  .velocity = Vec2(0.0f, 0.0f),
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
    player = gCoordinator.CreateEntity();
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
          .acceleration = Vec2(0.0f, 0.0f)
        });

    gCoordinator.AddComponent(
        player,
        Player{});

    gCoordinator.AddComponent(
        player,
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

    pSound->Update(dt);

    // Update camera

    Uma_ECS::Transform& tf = gCoordinator.GetComponent<Uma_ECS::Transform>(player);

    pGraphics->GetCamera().SetPosition(tf.position);
    pGraphics->GetCamera().SetZoom(1.f);

    //pSound->playSound(pResourcesManager->GetSound("explosion"));
    // play sound
    if (pInputSystem->KeyPressed(GLFW_KEY_P))
    {
        // moo moo
        try{
            //pSound->playSound(pResourcesManager->GetSound("explosion"), -1);
            pSound->playSound(pResourcesManager->GetSound("cave"));
        }
        catch (const std::invalid_argument& e) {
            std::cout << e.what() << "\n";
        }
    }
    if (pInputSystem->KeyPressed(GLFW_KEY_O)) {
        pSound->pauseAllSounds(paused);
        paused = !paused;
    }
    if (pInputSystem->KeyPressed(GLFW_KEY_9)) {
        volume += dt * 10;
        if (volume > 1) volume = 1;
        pSound->setChannelGroupVolume(volume ,SoundType::BGM);
    }
    if (pInputSystem->KeyPressed(GLFW_KEY_0)) {
        volume -= dt * 10;
        if (volume < 0) volume = 0;
        pSound->setChannelGroupVolume(volume, SoundType::BGM);
    }

    pGraphics->ClearBackground(0.2f, 0.3f, 0.3f);
    pGraphics->DrawBackground(pResourcesManager->GetTexture("background")->tex_id, pResourcesManager->GetTexture("background")->tex_size);
    renderingSystem->Update(dt);


    //camera updatye 

    //..update
}

void Uma_Engine::Test_Ecs::Shutdown()
{
}
