#pragma once
#include "SceneType.h"

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
#include "ECS/Components/Enemy.h"

// Engine Systems
#include "Systems/InputSystem.h"
#include "WIP_Scripts/Test_Input_Events.h"
#include "Systems/Graphics.hpp"
#include "Systems/Sound.hpp"
#include "Systems/ResourcesManager.hpp"
#include "Systems/CameraSystem.hpp"
#include "../Core/SystemManager.h"
#include "../Core/EventSystem.h"
#include "../Core/ECSEvents.h"
#include "../Core/IMGUIEvents.h"

// Serializer
#include "Core/GameSerializer.h"

// Engine Settings
#include "Core/FilePaths.h"

// debug
#include "Debugging/Debugger.hpp"

#include <vector>
#include <random>
#include <iostream>
#include <iomanip>

#include <GLFW/glfw3.h>


// Engine Systems
Uma_Engine::HybridInputSystem* pHybridInputSystem;
Uma_Engine::Graphics* pGraphics;
Uma_Engine::Sound* pSound;
Uma_Engine::ResourcesManager* pResourcesManager;
Uma_Engine::EventSystem* pEventSystem;

// ECS related
using Coordinator = Uma_ECS::Coordinator;
Coordinator gCoordinator;
std::shared_ptr<Uma_ECS::PhysicsSystem> physicsSystem;
std::shared_ptr<Uma_ECS::CollisionSystem> collisionSystem;
std::shared_ptr<Uma_ECS::PlayerControllerSystem> playerController;
std::shared_ptr<Uma_ECS::RenderingSystem> renderingSystem;
std::shared_ptr<Uma_ECS::CameraSystem> cameraSystem;
Uma_ECS::Entity player;
Uma_ECS::Entity cam;

// Scene Specific
Uma_Engine::GameSerializer gGameSerializer;
std::string currSceneName;

namespace Uma_Engine
{
    class EditorScene : public Scene
	  {
    private:
        SystemManager* pSystemManager;

    public:
        EditorScene(SystemManager* sm) : pSystemManager(sm) {}

		    void OnLoad() override
		    {
            currSceneName = "data.json";

            pHybridInputSystem = pSystemManager->GetSystem<HybridInputSystem>();
            pGraphics = pSystemManager->GetSystem<Graphics>();
            pResourcesManager = pSystemManager->GetSystem<ResourcesManager>();
            pEventSystem = pSystemManager->GetSystem<EventSystem>();
            pSound = pSystemManager->GetSystem<Sound>();

            // event system stuffs
            // subscribe to events
            pEventSystem->Subscribe<Uma_Engine::QueryActiveEntitiesEvent>([this](const Uma_Engine::QueryActiveEntitiesEvent& e) { e.mActiveEntityCnt = gCoordinator.GetEntityCount(); });
           
            pEventSystem->Subscribe<Uma_Engine::SaveSceneRequestEvent>([this](const Uma_Engine::SaveSceneRequestEvent& e) { (void)e; gGameSerializer.save(Uma_FilePath::SCENES_DIR + currSceneName); });
            pEventSystem->Subscribe<Uma_Engine::LoadSceneRequestEvent>([this](const Uma_Engine::LoadSceneRequestEvent& e) { (void)e; gCoordinator.DestroyAllEntities(); gGameSerializer.load(Uma_FilePath::SCENES_DIR + currSceneName); });
            pEventSystem->Subscribe<Uma_Engine::ClearSceneRequestEvent>([this](const Uma_Engine::ClearSceneRequestEvent& e) { (void)e; ResetAll(); });
            pEventSystem->Subscribe<Uma_Engine::StressTestRequestEvent>([this](const Uma_Engine::StressTestRequestEvent& e) { (void)e; StressTest(); });
            pEventSystem->Subscribe<Uma_Engine::ShowEntityInVPRequestEvent>([this](const Uma_Engine::ShowEntityInVPRequestEvent& e) { (void)e; SpawnDefaultEntities(); });


            pEventSystem->Subscribe<Uma_Engine::ChangeEnemyRotRequestEvent>([this](const Uma_Engine::ChangeEnemyRotRequestEvent& e) { ChangeAllEnemyRot(e.rot); });
            pEventSystem->Subscribe<Uma_Engine::ChangeEnemyScaleRequestEvent>([this](const Uma_Engine::ChangeEnemyScaleRequestEvent& e) { ChangeAllEnemyScale(e.scale); });
            pEventSystem->Subscribe<Uma_Engine::ShowBBoxRequestEvent>([this](const Uma_Engine::ShowBBoxRequestEvent& e) {  ShowBBox(e.show); });


            pEventSystem->Subscribe<Uma_Engine::CloneEntityRequestEvent>([this](const Uma_Engine::CloneEntityRequestEvent& e) 
                { 
                    (void)e;
                    DuplicateOrCreateEntity();
                });
            pEventSystem->Subscribe<Uma_Engine::DestroyEntityRequestEvent>([this](const Uma_Engine::DestroyEntityRequestEvent& e) 
                { 
                    (void)e;
                    DestroyRandomEntity();
                });


            // Ecs stuff
            using namespace Uma_ECS;

            gCoordinator.Init(pEventSystem);

            // register components
            gCoordinator.RegisterComponent<Transform>();
            gCoordinator.RegisterComponent<RigidBody>();
            gCoordinator.RegisterComponent<Collider>();
            gCoordinator.RegisterComponent<SpriteRenderer>();
            gCoordinator.RegisterComponent<Camera>();
            gCoordinator.RegisterComponent<Player>();
            gCoordinator.RegisterComponent<Enemy>();

            // Player controller
            playerController = gCoordinator.RegisterSystem<PlayerControllerSystem>();
            {
                Signature sign;
                sign.set(gCoordinator.GetComponentType<RigidBody>());
                sign.set(gCoordinator.GetComponentType<Transform>());
                sign.set(gCoordinator.GetComponentType<Player>());
                gCoordinator.SetSystemSignature<PlayerControllerSystem>(sign);
            }
            playerController->Init(pEventSystem, pHybridInputSystem, &gCoordinator);

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
            renderingSystem->Init(pGraphics, pResourcesManager, &gCoordinator);

            cameraSystem = gCoordinator.RegisterSystem<CameraSystem>();
            {
                Signature sign;
                sign.set(gCoordinator.GetComponentType<Camera>());
                sign.set(gCoordinator.GetComponentType<Transform>());
                gCoordinator.SetSystemSignature<CameraSystem>(sign);
            }
            cameraSystem->Init(&gCoordinator);

            // Init the game serializer
            gGameSerializer.Register(pResourcesManager);
            gGameSerializer.Register(&gCoordinator);


            //deserialize and spawn all the entities
            //gCoordinator.DeserializeAllEntities("Assets/Scenes/data.json");
            gGameSerializer.load(Uma_FilePath::SCENES_DIR + currSceneName);

		    }
		    void OnUnload() override
		    {
			      std::cout << "Test Scene 1: UNLOADED" << std::endl;

            // resources unload
            pResourcesManager->UnloadAllTextures();
            pResourcesManager->UnloadAllSound();
		    }
		    void Update(float dt) override
		    {
            static bool firstFrame = true;
            static float smoothedDt = 0.0f;
            if (firstFrame) {
                smoothedDt = dt; // seed filter with a realistic value
                firstFrame = false;
            }
            else {
                smoothedDt = 0.9f * smoothedDt + 0.1f * dt;
            }

            playerController->Update(dt);

            physicsSystem->Update(smoothedDt);

            collisionSystem->Update(dt);

            cameraSystem->Update(dt);

            // save to file
            if (pHybridInputSystem->KeyPressed(GLFW_KEY_1))
            {
                std::string filepath = Uma_FilePath::SCENES_DIR + currSceneName;

                gGameSerializer.save(filepath);
            }

            // load from file
            if (pHybridInputSystem->KeyPressed(GLFW_KEY_2))
            {
                gCoordinator.DestroyAllEntities();

                std::string filepath = Uma_FilePath::SCENES_DIR + currSceneName;
                
                gGameSerializer.load(filepath);
            }

            // reset
            if (pHybridInputSystem->KeyPressed(GLFW_KEY_3))
            {
                ResetAll();
            }

            // Spawn Default
            if (HybridInputSystem::KeyPressed(GLFW_KEY_4))
            {
                gCoordinator.DestroyAllEntities();
                SpawnDefaultEntities();
            }

            if (HybridInputSystem::KeyPressed(GLFW_KEY_P))
            {
                pSound->playSound(pResourcesManager->GetSound("explosion"));
            }

            if (HybridInputSystem::KeyPressed(GLFW_KEY_O))
            {
                pSound->playSound(pResourcesManager->GetSound("cave"));
            }

            pGraphics->ClearBackground(0.2f, 0.3f, 0.3f);
            pGraphics->DrawBackground(pResourcesManager->GetTexture("background")->tex_id);
            renderingSystem->Update(dt);
		    }
		    void Render() override
		    {

		    }

        void ResetAll()
        {
            gCoordinator.DestroyAllEntities();

            using namespace Uma_ECS;
            
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

            //std::string log;
            //std::stringstream ss(log);
            //ss << "Created Entity : " << 10000;
            //Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo, ss.str());
        }

        void SpawnDefaultEntities()
        {
            gCoordinator.DestroyAllEntities();

            using namespace Uma_ECS;

            // create entities
            {
                std::default_random_engine generator;
                std::uniform_real_distribution<float> randPositionX(-1920.f * 0.45f, 1920.f * 0.45f);
                std::uniform_real_distribution<float> randPositionY(-1080.f * 0.45f, 1080.f * 0.45f);
                std::uniform_real_distribution<float> randRotation(0.0f, 0.0f);
                std::uniform_real_distribution<float> randScale(10.0f, 15.0f);

                Entity enemy;
                {
                    enemy = gCoordinator.CreateEntity();

                    gCoordinator.AddComponent(
                        enemy,
                        Enemy{
                            .mSpeed = 1.f
                        });

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
                for (size_t i = 0; i < 2500 - 3; i++)
                {
                    Entity tmp = gCoordinator.DuplicateEntity(enemy);

                    Transform& tf = gCoordinator.GetComponent<Transform>(tmp);

                    tf.position = Vec2(randPositionX(generator), randPositionY(generator));
                    tf.rotation = Vec2(randRotation(generator), randRotation(generator));
                    tf.scale = Vec2(randScale(generator), randScale(generator));

                    SpriteRenderer& sr = gCoordinator.GetComponent<SpriteRenderer>(tmp);

                    sr.textureName = (i > 1250) ? "pink_enemy" : "enemy";
                    sr.texture = pResourcesManager->GetTexture(sr.textureName);
                }
            }

            // create player
            player = gCoordinator.CreateEntity();
            {
                gCoordinator.AddComponent(
                    player,
                    Transform
                    {
                        .position = Vec2(0.f, 0.f),
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
                    Player{
                        .mSpeed = 1.f
                    });

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
        }

        void DuplicateOrCreateEntity()
        {
            using namespace Uma_ECS;

            // find an active entity
            auto& eArray = gCoordinator.GetComponentArray<Uma_ECS::Enemy>();

            std::default_random_engine generator(std::random_device{}());
            std::uniform_real_distribution<float> randPositionX(-400, 400);
            std::uniform_real_distribution<float> randPositionY(-400, 400);
            std::uniform_real_distribution<float> randScale(10.0f, 15.0f);

            if (eArray.Size() == 0)
            {
                // Create Entity

                Entity enemy;
                {
                    enemy = gCoordinator.CreateEntity();

                    gCoordinator.AddComponent(
                        enemy,
                        Enemy{
                            .mSpeed = 1.f
                        });

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
                          .rotation = Vec2(0, 0),
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
            }
            else
            {
                // duplicate existing entity

                Entity tmp = gCoordinator.DuplicateEntity(eArray.GetEntity(0));

                Transform& tf = gCoordinator.GetComponent<Transform>(tmp);

                tf.position = Vec2(randPositionX(generator), randPositionY(generator));
                tf.rotation = Vec2(0, 0);
                tf.scale = Vec2(randScale(generator), randScale(generator));

                SpriteRenderer& sr = gCoordinator.GetComponent<SpriteRenderer>(tmp);

                // set texture randomly
                sr.textureName = (randPositionX(generator) > 0.f) ? "pink_enemy" : "enemy";
                sr.texture = pResourcesManager->GetTexture(sr.textureName);
            }
        }

        void DestroyRandomEntity()
        {
            auto& eArray = gCoordinator.GetComponentArray<Uma_ECS::Enemy>();

            std::default_random_engine generator(std::random_device{}());
            std::uniform_real_distribution<float> randPositionX(-400, 400);
            std::uniform_real_distribution<float> randPositionY(-400, 400);
            std::uniform_real_distribution<float> randScale(10.0f, 15.0f);

            if (eArray.Size() != 0)
            {
                gCoordinator.DestroyEntity(eArray.GetEntity(0));
            }
        }

        void StressTest()
        {
            gCoordinator.DestroyAllEntities();

            using namespace Uma_ECS;

            // create entities
            {
                std::default_random_engine generator;
                std::uniform_real_distribution<float> randPositionX(-1920.f, 1920.f);
                std::uniform_real_distribution<float> randPositionY(-1080.f, 1080.f);
                std::uniform_real_distribution<float> randRotation(0.0f, 0.0f);
                std::uniform_real_distribution<float> randScale(10.0f, 15.0f);

                Entity enemy;
                {
                    enemy = gCoordinator.CreateEntity();

                    gCoordinator.AddComponent(
                        enemy,
                        Enemy{
                            .mSpeed = 1.f
                        });

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
                for (size_t i = 0; i < 10000 - 3; i++)
                {
                    Entity tmp = gCoordinator.DuplicateEntity(enemy);

                    Transform& tf = gCoordinator.GetComponent<Transform>(tmp);

                    tf.position = Vec2(randPositionX(generator), randPositionY(generator));
                    tf.rotation = Vec2(randRotation(generator), randRotation(generator));
                    tf.scale = Vec2(randScale(generator), randScale(generator));

                    SpriteRenderer& sr = gCoordinator.GetComponent<SpriteRenderer>(tmp);

                    sr.textureName = (i > 1250) ? "pink_enemy" : "enemy";
                    sr.texture = pResourcesManager->GetTexture(sr.textureName);
                }
            }

            // create player
            player = gCoordinator.CreateEntity();
            {
                gCoordinator.AddComponent(
                    player,
                    Transform
                    {
                        .position = Vec2(0.f, 0.f),
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
                    Player{
                        .mSpeed = 1.f
                    });

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
        }

        void ChangeAllEnemyRot(float rot)
        {
            using namespace Uma_ECS;

            //std::default_random_engine generator(std::random_device{}());
            //std::uniform_real_distribution<float> randScale(10.0f, 15.0f);

            auto& eArray = gCoordinator.GetComponentArray<Uma_ECS::Enemy>();
            auto& tfArray = gCoordinator.GetComponentArray<Uma_ECS::Transform>();

            for (size_t i = 0; i < eArray.Size(); i++)
            {
                auto& tf = tfArray.GetData(eArray.GetEntity(i));

                //tf.scale = //Vec2{randScale(generator), randScale(generator)} * scale;

                tf.rotation.x += rot;
                tf.rotation.y = 200.f;
            }
        }

        void ChangeAllEnemyScale(float scale)
        {
            using namespace Uma_ECS;

            std::default_random_engine generator(std::random_device{}());
            std::uniform_real_distribution<float> randScale(10.0f, 15.0f);

            auto& eArray = gCoordinator.GetComponentArray<Uma_ECS::Enemy>();
            auto& tfArray = gCoordinator.GetComponentArray<Uma_ECS::Transform>();

            for (size_t i = 0; i < eArray.Size(); i++)
            {
                auto& tf = tfArray.GetData(eArray.GetEntity(i));

                tf.scale = Vec2{randScale(generator), randScale(generator)} * scale;
            }
        }

        void ShowBBox(bool isShow)
        {
            using namespace Uma_ECS;

            auto& tfArray = gCoordinator.GetComponentArray<Uma_ECS::Enemy>();
            auto& cArray = gCoordinator.GetComponentArray<Uma_ECS::Collider>();

            for (size_t i = 0; i < tfArray.Size(); i++)
            {
                auto& c = cArray.GetData(tfArray.GetEntity(i));

                c.showBBox = isShow;
            }
        }
	  };
}
