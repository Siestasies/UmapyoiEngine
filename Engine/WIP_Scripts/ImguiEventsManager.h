#pragma once

// ECS Core
#include "ECS/Core/Coordinator.hpp"

// ECS Systems
#include "ECS/Systems/PhysicsSystem.hpp"
#include "ECS/Systems/PlayerControllerSystem.hpp"
#include "ECS/Systems/RenderingSystem.hpp"
#include "ECS/Systems/CollisionSystem.hpp"
#include "ECS/Systems/LuaScriptingSystem.hpp"

// ECS Components
#include "ECS/Components/Transform.h"
#include "ECS/Components/RigidBody.h"
#include "ECS/Components/Player.h"
#include "ECS/Components/Sprite.h"
#include "ECS/Components/Collider.h"
#include "ECS/Components/Camera.h"
#include "ECS/Components/Enemy.h"
#include "ECS/Components/LuaScript.h"

// Engine Systems
#include "Systems/InputSystem.h"
#include "WIP_Scripts/Test_Input_Events.h"
#include "Systems/Graphics.hpp"
#include "Systems/Sound.hpp"
#include "Systems/ResourcesManager.hpp"
#include "Systems/CameraSystem.hpp"
#include "../Core/SystemManager.h"
#include "../Core/EventSystem.h"
#include "../Events/ECSEvents.h"
#include "../Events/IMGUIEvents.h"

#include "Core/GameSerializer.h"
// Engine Settings
#include "Core/FilePaths.h"

#include <random>

namespace Uma_Engine
{
    class ImguiEventsManager
    {
    private:
        Uma_Engine::SystemManager* m_SystemManager;
        Uma_Engine::HybridInputSystem* m_HybridInputSystem;
        Uma_Engine::ResourcesManager* m_ResourcesManager;
        Uma_Engine::EventSystem* m_eventSystem;

        Uma_ECS::Coordinator m_Coordinator;
        std::shared_ptr<Uma_ECS::PhysicsSystem> m_PhysicsSystem;
        std::shared_ptr<Uma_ECS::CollisionSystem> m_CollisionSystem;
        std::shared_ptr<Uma_ECS::PlayerControllerSystem> m_PlayerController;
        std::shared_ptr<Uma_ECS::RenderingSystem> m_RenderingSystem;
        std::shared_ptr<Uma_ECS::CameraSystem> m_CameraSystem;
        std::shared_ptr<Uma_ECS::LuaScriptingSystem> m_LuaScriptingSystem;

    public:
        ImguiEventsManager()
        {
            m_SystemManager = nullptr;
            m_HybridInputSystem = nullptr;
            m_ResourcesManager = nullptr;
            m_eventSystem = nullptr;
        }
        void Init (SystemManager* sm)
        {
            m_SystemManager = sm;
            m_HybridInputSystem = sm->GetSystem<HybridInputSystem>();
            m_ResourcesManager = sm->GetSystem<ResourcesManager>();
            m_eventSystem = sm->GetSystem<EventSystem>();

            SubscribeToEvents();
        }

        void SubscribeToEvents()
        {
                auto eventSystem = m_eventSystem;
                auto& coordinator = m_Coordinator;
 
            // Query active entities
            m_eventSystem->Subscribe<QueryActiveEntitiesEvent>(
                [&coordinator](const QueryActiveEntitiesEvent& e) {
                    e.mActiveEntityCnt = coordinator.GetEntityCount();
                }
            );

            // Save scene
            //m_eventSystem->Subscribe<SaveSceneRequestEvent>(
            //    [this](const SaveSceneRequestEvent& e) {
            //        (void)e;
            //        SaveScene();
            //    }
            //);

            //// Load scene
            //m_eventSystem->Subscribe<LoadSceneRequestEvent>(
            //    [this](const LoadSceneRequestEvent& e) {
            //        (void)e;
            //        LoadScene();
            //    }
            //);

            // Clear scene
            m_eventSystem->Subscribe<ClearSceneRequestEvent>(
                [this](const ClearSceneRequestEvent& e) {
                    (void)e;
                    ResetScene();
                }
            );

            // Stress test
            m_eventSystem->Subscribe<StressTestRequestEvent>(
                [this](const StressTestRequestEvent& e) {
                    (void)e;
                    StressTest();
                }
            );

            // Show default entities in viewport
            m_eventSystem->Subscribe<ShowEntityInVPRequestEvent>(
                [this](const ShowEntityInVPRequestEvent& e) {
                    (void)e;
                    SpawnDefaultEntities();
                }
            );

            // Change enemy rotation
            m_eventSystem->Subscribe<ChangeEnemyRotRequestEvent>(
                [this](const ChangeEnemyRotRequestEvent& e) {
                    ChangeAllEnemyRot(e.rot);
                }
            );

            // Change enemy X position
            m_eventSystem->Subscribe<ChangeEnemyXposRequestEvent>(
                [this](const ChangeEnemyXposRequestEvent& e) {
                    ChangeAllEnemyXPos(e.xpos);
                }
            );

            // Change enemy scale
            m_eventSystem->Subscribe<ChangeEnemyScaleRequestEvent>(
                [this](const ChangeEnemyScaleRequestEvent& e) {
                    ChangeAllEnemyScale(e.scale);
                }
            );

            // Show bounding boxes
            m_eventSystem->Subscribe<ShowBBoxRequestEvent>(
                [this](const ShowBBoxRequestEvent& e) {
                    ShowBBox(e.show);
                }
            );

            // Clone entity
            m_eventSystem->Subscribe<CloneEntityRequestEvent>(
                [this](const CloneEntityRequestEvent& e) {
                    (void)e;
                    DuplicateOrCreateEntity();
                }
            );

            // Load prefab
            m_eventSystem->Subscribe<LoadPrefabRequestEvent>(
                [this](const LoadPrefabRequestEvent& e) {
                    (void)e;
                    LoadPrefab();
                }
            );

            // Destroy entity
            m_eventSystem->Subscribe<DestroyEntityRequestEvent>(
                [this](const DestroyEntityRequestEvent& e) {
                    (void)e;
                    DestroyRandomEntity();
                }
            );
        }

    private:

        //void SaveScene()
        //{
        //    std::string filepath = Uma_FilePath::SCENES_DIR + m_CurrentSceneName;

        //    GameSerializer serializer;
        //    serializer.Register(m_ResourcesManager);
        //    serializer.Register(&m_Coordinator);
        //    serializer.save(filepath);

        //    std::cout << "Scene saved to: " << filepath << std::endl;
        //}

        //void LoadScene()
        //{
        //    m_Coordinator.DestroyAllEntities();

        //    std::string filepath = Uma_FilePath::SCENES_DIR + m_CurrentSceneName;

        //    GameSerializer serializer;
        //    serializer.Register(m_ResourcesManager);
        //    serializer.Register(&m_Coordinator);
        //    serializer.load(filepath);

        //    std::cout << "Scene loaded from: " << filepath << std::endl;
        //}

        void ResetScene()
        {
            using namespace Uma_ECS;

            m_Coordinator.DestroyAllEntities();

            // Create player
            Entity player = m_Coordinator.CreateEntity();
            {
                m_Coordinator.AddComponent(player, Transform{
                    .position = Vec2(0.f, 0.f),
                    .rotation = Vec2(0.f, 0.f),
                    .scale = Vec2(1, 1),
                    });

                m_Coordinator.AddComponent(player, RigidBody{
                    .velocity = Vec2(0.0f, 0.0f),
                    .acceleration = Vec2(0.0f, 0.0f),
                    .accel_strength = 500,
                    .fric_coeff = 5
                    });

                m_Coordinator.AddComponent(player, Player{ .mSpeed = 1.f });

                std::string texName = "player";
                m_Coordinator.AddComponent(player, Sprite{
                    .textureName = texName,
                    .flipX = false,
                    .flipY = false,
                    .UseNativeSize = true,
                    .texture = m_ResourcesManager->GetTexture(texName),
                    });

                Collider playerCollider;
                playerCollider.shapes[0] = ColliderShape{
                    .purpose = ColliderPurpose::Physics,
                    .layer = CL_PLAYER,
                    .colliderMask = CL_ENEMY | CL_PROJECTILE,
                    .isActive = true,
                    .autoFitToSprite = true
                };

                playerCollider.shapes.push_back(ColliderShape{
                    .size = Vec2(7.0f, 0.5f),
                    .offset = Vec2(0, -3.f),
                    .purpose = ColliderPurpose::Physics,
                    .layer = CL_PLAYER,
                    .colliderMask = CL_WALL,
                    .isActive = true,
                    .autoFitToSprite = false
                    });

                playerCollider.bounds.resize(playerCollider.shapes.size());
                m_Coordinator.AddComponent(player, playerCollider);
            }

            // Create camera
            Entity cam = m_Coordinator.CreateEntity();
            {
                m_Coordinator.AddComponent(cam, Transform{
                    .position = Vec2(400.0f, 300.0f),
                    .rotation = Vec2(0, 0),
                    .scale = Vec2(1, 1),
                    });

                m_Coordinator.AddComponent(player, Camera{
                    .mZoom = 1.f,
                    .followPlayer = true
                    });
            }
        }

        void SpawnDefaultEntities()
        {
            m_Coordinator.DestroyAllEntities();

            using namespace Uma_ECS;

            Entity kappa;
            {
                kappa = m_Coordinator.CreateEntity();

                m_Coordinator.AddComponent(
                    kappa,
                    RigidBody{
                      .velocity = Vec2(0.0f, 0.0f),
                      .acceleration = Vec2(0.0f, 0.0f),
                      .accel_strength = 200,
                      .fric_coeff = 100
                    });

                m_Coordinator.AddComponent(
                    kappa,
                    Transform{
                      .position = Vec2(30, 35),
                      .rotation = Vec2(0, 0),
                      .scale = Vec2(3.f, 3.f)
                    });

                std::string texName = "kappa_statue";
                m_Coordinator.AddComponent(
                    kappa,
                    Sprite{
                      .textureName = texName,
                      .renderLayer = RL_ENV,
                      .flipX = false,
                      .flipY = false,
                      .UseNativeSize = true,
                      .texture = m_ResourcesManager->GetTexture(texName),
                    });

                LuaScript kappaScriptComponent;
                {
                    kappaScriptComponent.AddScript(Uma_FilePath::SCRIPT_DIR + "kappa.lua");

                    kappaScriptComponent.GetScript(0)->exposedVariables.push_back(Uma_ECS::LuaVariable{
                        .name = "speed",
                        .value = 100.0f,
                        .type = Uma_ECS::LuaVarType::T_FLOAT,
                        .min = 0.0f,
                        .max = 500.0f,
                        .isSlider = true
                        });

                    // this works just that i didnt want to add this now
                    /*kappaScriptComponent.AddScript(Uma_FilePath::SCRIPT_DIR + "kappaScale.lua");

                    kappaScriptComponent.GetScript(1)->exposedVariables.push_back(Uma_ECS::LuaVariable{
                       .name = "speed",
                       .value = 100.0f,
                       .type = Uma_ECS::LuaVarType::T_FLOAT,
                       .min = 0.0f,
                       .max = 500.0f,
                       .isSlider = true
                        });*/

                    m_Coordinator.AddComponent(kappa, kappaScriptComponent);
                }

                //LuaScript kappaScaleScript;
                //{
                //    kappaScaleScript.scriptPath = Uma_FilePath::SCRIPT_DIR + "kappaScale.lua";

                //    // Optional: Pre-define exposed variables (or let Lua auto-discover)
                //    kappaScaleScript.exposedVariables.push_back(Uma_ECS::LuaVariable{
                //        .name = "speed",
                //        .value = 100.0f,
                //        .type = Uma_ECS::LuaVarType::T_FLOAT,
                //        .min = 0.0f,
                //        .max = 500.0f,
                //        .isSlider = true
                //        });

                //    m_Coordinator.AddComponent(kappa, kappaScaleScript);
                //}
            }

            Entity wall;
            {
                wall = m_Coordinator.CreateEntity();

                m_Coordinator.AddComponent(
                    wall,
                    RigidBody{
                      .velocity = Vec2(0.0f, 0.0f),
                      .acceleration = Vec2(0.0f, 0.0f),
                      .accel_strength = 200,
                      .fric_coeff = 100
                    });

                m_Coordinator.AddComponent(
                    wall,
                    Transform{
                      .position = Vec2(20, 0),
                      .rotation = Vec2(0, 0),
                      .scale = Vec2(1.f, 1.f)
                    });

                std::string texName = "wall_top";
                m_Coordinator.AddComponent(
                    wall,
                    Sprite{
                      .textureName = texName,
                      .renderLayer = RL_WALL,
                      .flipX = false,
                      .flipY = false,
                      .UseNativeSize = true,
                      .texture = m_ResourcesManager->GetTexture(texName),
                    });

                // Create collider with two shapes
                Collider wallCollider;

                // Primary shape: Body hitbox (for taking damage)
                wallCollider.shapes[0] = ColliderShape{
                    .purpose = ColliderPurpose::Environment,
                    .layer = CL_WALL,
                    .colliderMask = CL_PLAYER | CL_ENEMY,  // Blocks entities,
                    .isActive = true,
                    .autoFitToSprite = true  // Will be 128x128 (64*2 scale)
                };

                wallCollider.bounds.resize(wallCollider.shapes.size());
                m_Coordinator.AddComponent(wall, wallCollider);

                for (size_t i = 0; i < 5; i++)
                {
                    Entity tmp = m_Coordinator.DuplicateEntity(wall);

                    Transform& tf = m_Coordinator.GetComponent<Transform>(tmp);

                    tf.position = Vec2(20 + (i * 5), 0);

                    Sprite& sr = m_Coordinator.GetComponent<Sprite>(tmp);

                    // set texture randomly
                    sr.textureName = "wall_btm";
                    sr.texture = m_ResourcesManager->GetTexture(sr.textureName);
                }

                for (size_t i = 0; i < 6; i++)
                {
                    Entity tmp = m_Coordinator.DuplicateEntity(wall);

                    Transform& tf = m_Coordinator.GetComponent<Transform>(tmp);

                    tf.position = Vec2(15 + (6 * 5), 5 + (i * 5));

                    Sprite& sr = m_Coordinator.GetComponent<Sprite>(tmp);

                    // set texture randomly
                    sr.textureName = "wall_right";
                    sr.texture = m_ResourcesManager->GetTexture(sr.textureName);
                }

                for (size_t i = 0; i < 5; i++)
                {
                    Entity tmp = m_Coordinator.DuplicateEntity(wall);

                    Transform& tf = m_Coordinator.GetComponent<Transform>(tmp);

                    tf.position = Vec2(20 + (i * 5), 15 + (4 * 5));

                    Sprite& sr = m_Coordinator.GetComponent<Sprite>(tmp);

                    // set texture randomly
                    sr.textureName = "wall_top";
                    sr.texture = m_ResourcesManager->GetTexture(sr.textureName);
                }
            }

            Entity floor;
            {
                floor = m_Coordinator.CreateEntity();

                m_Coordinator.AddComponent(
                    floor,
                    RigidBody{
                      .velocity = Vec2(0.0f, 0.0f),
                      .acceleration = Vec2(0.0f, 0.0f),
                      .accel_strength = 200,
                      .fric_coeff = 100
                    });

                m_Coordinator.AddComponent(
                    floor,
                    Transform{
                      .position = Vec2(20, 7.5),
                      .rotation = Vec2(0, 0),
                      .scale = Vec2(2.f, 2.f)
                    });

                std::string texName = "floor_tatami";
                m_Coordinator.AddComponent(
                    floor,
                    Sprite{
                      .textureName = texName,
                      .renderLayer = RL_WALL,
                      .flipX = false,
                      .flipY = false,
                      .UseNativeSize = true,
                      .texture = m_ResourcesManager->GetTexture(texName),
                    });

                for (size_t i = 0; i < 5; i++)
                {
                    for (size_t j = 0; j < 3; j++)
                    {
                        Entity tmp = m_Coordinator.DuplicateEntity(floor);

                        Transform& tf = m_Coordinator.GetComponent<Transform>(tmp);

                        tf.position = Vec2(20 + (i * 5), 7.5 + (j * 10));
                    }
                }
            }

            // create entities
            {
                //std::default_random_engine generator;
                //std::uniform_real_distribution<float> randPositionX(-1920.f * 0.1f, 1920.f * 0.1f);
                //std::uniform_real_distribution<float> randPositionY(-1080.f * 0.1f, 1080.f * 0.1f);
                ////std::uniform_real_distribution<float> randRotation(10.0f, 15.0f);
                //std::uniform_real_distribution<float> randScale(1.f, 1.f);

                Entity enemy;
                {
                    enemy = m_Coordinator.CreateEntity();

                    m_Coordinator.AddComponent(
                        enemy,
                        Enemy{
                            .mSpeed = 1.f
                        });

                    m_Coordinator.AddComponent(
                        enemy,
                        RigidBody{
                          .velocity = Vec2(0.0f, 0.0f),
                          .acceleration = Vec2(0.0f, 0.0f),
                          .accel_strength = 200,
                          .fric_coeff = 100
                        });

                    m_Coordinator.AddComponent(
                        enemy,
                        Transform{
                          .position = Vec2(-10, 0),
                          .rotation = Vec2(0, 0),
                          .scale = Vec2(2.f, 2.f)
                        });

                    std::string texName = "pink_enemy";
                    m_Coordinator.AddComponent(
                        enemy,
                        Sprite{
                          .textureName = texName,
                          .renderLayer = RL_ENEMY,
                          .flipX = false,
                          .flipY = false,
                          .UseNativeSize = true,
                          .texture = m_ResourcesManager->GetTexture(texName),
                        });

                    // Create collider with two shapes
                    Collider enemyCollider;

                    enemyCollider.shapes[0] = ColliderShape{
                        .size = Vec2(3.f, 3.f),
                        .offset = Vec2(0.f, 1.f),
                        .purpose = ColliderPurpose::Physics,
                        .layer = CL_ENEMY,
                        .colliderMask = CL_PLAYER | CL_PROJECTILE,
                        .isActive = true,
                        .autoFitToSprite = false
                    };

                    enemyCollider.shapes.push_back(ColliderShape{
                        .size = Vec2(2.f, 0.5f),
                        .offset = Vec2(0.f, -2.f),
                        .purpose = ColliderPurpose::Physics,
                        .layer = CL_ENEMY,
                        .colliderMask = CL_WALL,
                        .isActive = true,
                        .autoFitToSprite = false
                        });

                    enemyCollider.bounds.resize(enemyCollider.shapes.size());
                    m_Coordinator.AddComponent(enemy, enemyCollider);

                    LuaScript enemyScriptComponent;
                    {
                        enemyScriptComponent.AddScript(Uma_FilePath::SCRIPT_DIR + "BirdEnemy.lua");

                        enemyScriptComponent.GetScript(0)->exposedVariables.push_back(Uma_ECS::LuaVariable{
                            .name = "speed",
                            .value = 100.0f,
                            .type = Uma_ECS::LuaVarType::T_FLOAT,
                            .min = 0.0f,
                            .max = 500.0f,
                            .isSlider = true
                            });

                        enemyScriptComponent.GetScript(0)->exposedVariables.push_back(Uma_ECS::LuaVariable{
                            .name = "name",
                            .value = "bird",
                            .type = Uma_ECS::LuaVarType::T_STRING,
                            .isSlider = false
                            });

                        // this works just that i didnt want to add this now
                        /*kappaScriptComponent.AddScript(Uma_FilePath::SCRIPT_DIR + "kappaScale.lua");

                        kappaScriptComponent.GetScript(1)->exposedVariables.push_back(Uma_ECS::LuaVariable{
                           .name = "speed",
                           .value = 100.0f,
                           .type = Uma_ECS::LuaVarType::T_FLOAT,
                           .min = 0.0f,
                           .max = 500.0f,
                           .isSlider = true
                            });*/

                        m_Coordinator.AddComponent(enemy, enemyScriptComponent);
                    }
                }

                // using 1 enemy to duplicate 2500 times and rand its transform
                /*for (size_t i = 0; i < 2500 - 3; i++)
                {
                    Entity tmp = m_Coordinator.DuplicateEntity(enemy);

                    Transform& tf = m_Coordinator.GetComponent<Transform>(tmp);

                    tf.position = Vec2(randPositionX(generator), randPositionY(generator));
                    tf.rotation = Vec2(0, 0);
                    tf.scale = Vec2(randScale(generator), randScale(generator));

                    Sprite& sr = m_Coordinator.GetComponent<Sprite>(tmp);

                    sr.textureName = (i > 1250) ? "pink_enemy" : "enemy";
                    sr.texture = pResourcesManager->GetTexture(sr.textureName);
                }*/
            }

            // create player
            Entity m_player = m_Coordinator.CreateEntity();
            {
                m_Coordinator.AddComponent(
                    m_player,
                    Transform
                    {
                        .position = Vec2(0.f, 0.f),
                        .rotation = Vec2(0.f, 0.f),
                        .scale = Vec2(1,1),
                    });

                m_Coordinator.AddComponent(
                    m_player,
                    RigidBody{
                      .velocity = Vec2(0.0f, 0.0f),
                      .acceleration = Vec2(0.0f, 0.0f),
                      .accel_strength = 500,
                      .fric_coeff = 5
                    });

                m_Coordinator.AddComponent(
                    m_player,
                    Player{
                        .mSpeed = 1.f
                    });

                std::string texName = "player";
                m_Coordinator.AddComponent(
                    m_player,
                    Sprite{
                      .textureName = texName,
                      .renderLayer = RL_PLAYER,
                      .flipX = false,
                      .flipY = false,
                      .UseNativeSize = true,
                      .texture = m_ResourcesManager->GetTexture(texName),
                    });

                // Create collider with two shapes
                Collider playerCollider;

                playerCollider.shapes[0] = ColliderShape{
                        .purpose = ColliderPurpose::Physics,
                        .layer = CL_PLAYER,
                        .colliderMask = CL_ENEMY | CL_PROJECTILE,
                        .isActive = true,
                        .autoFitToSprite = true
                };

                playerCollider.shapes.push_back(ColliderShape{
                    .size = Vec2(7.0f, 0.5f),
                    .offset = Vec2(0, -2.75f),
                    .purpose = ColliderPurpose::Physics,
                    .layer = CL_PLAYER,
                    .colliderMask = CL_WALL,
                    .isActive = true,
                    .autoFitToSprite = false
                    });

                playerCollider.bounds.resize(playerCollider.shapes.size());
                m_Coordinator.AddComponent(m_player, playerCollider);
            }

            // create camera
            Entity m_cam = m_Coordinator.CreateEntity();
            {
                m_Coordinator.AddComponent(
                    m_cam,
                    Transform
                    {
                        .position = Vec2(400.0f, 300.0f),
                        .rotation = Vec2(0,0),
                        .scale = Vec2(1,1),
                    });

                m_Coordinator.AddComponent(
                    m_player,
                    Camera
                    {
                        .mZoom = 1.f,
                        .followPlayer = true
                    });
            }

            m_LuaScriptingSystem->CallStart();
        }

        void StressTest()
        {
            using namespace Uma_ECS;

            m_Coordinator.DestroyAllEntities();

            std::default_random_engine generator;
            std::uniform_real_distribution<float> randPosX(-1920.f, 1920.f);
            std::uniform_real_distribution<float> randPosY(-1080.f, 1080.f);

            // Create base enemy
            Entity enemy = m_Coordinator.CreateEntity();
            {
                m_Coordinator.AddComponent(enemy, Enemy{ .mSpeed = 1.f });
                m_Coordinator.AddComponent(enemy, RigidBody{
                    .velocity = Vec2(0.0f, 0.0f),
                    .acceleration = Vec2(0.0f, 0.0f),
                    .accel_strength = 200,
                    .fric_coeff = 100
                    });

                m_Coordinator.AddComponent(enemy, Transform{
                    .position = Vec2(-10, 0),
                    .rotation = Vec2(0, 0),
                    .scale = Vec2(1.f, 1.f)
                    });

                std::string texName = "pink_enemy";
                m_Coordinator.AddComponent(enemy, Sprite{
                    .textureName = texName,
                    .flipX = false,
                    .flipY = false,
                    .UseNativeSize = true,
                    .texture = m_ResourcesManager->GetTexture(texName),
                    });

                Collider enemyCollider;
                enemyCollider.shapes[0] = ColliderShape{
                    .size = Vec2(3.f, 3.f),
                    .offset = Vec2(0.f, 1.f),
                    .purpose = ColliderPurpose::Physics,
                    .layer = CL_ENEMY,
                    .colliderMask = CL_PLAYER | CL_PROJECTILE,
                    .isActive = true,
                    .autoFitToSprite = false
                };

                enemyCollider.shapes.push_back(ColliderShape{
                    .size = Vec2(2.f, 0.5f),
                    .offset = Vec2(0.f, -2.f),
                    .purpose = ColliderPurpose::Environment,
                    .layer = CL_WALL,
                    .colliderMask = CL_WALL,
                    .isActive = true,
                    .autoFitToSprite = false
                    });

                enemyCollider.bounds.resize(enemyCollider.shapes.size());
                m_Coordinator.AddComponent(enemy, enemyCollider);
            }

            // Duplicate 10000 times
            for (size_t i = 0; i < 10000; i++)
            {
                Entity tmp = m_Coordinator.DuplicateEntity(enemy);
                Transform& tf = m_Coordinator.GetComponent<Transform>(tmp);
                tf.position = Vec2(randPosX(generator), randPosY(generator));
            }

            std::cout << "Stress test: 10000 entities spawned" << std::endl;
        }

        void DuplicateOrCreateEntity()
        {
            using namespace Uma_ECS;

            auto& eArray = m_Coordinator.GetComponentArray<Enemy>();

            std::default_random_engine generator(std::random_device{}());
            std::uniform_real_distribution<float> randPos(-400, 400);

            if (eArray.Size() == 0)
            {
                // Create new enemy and save as prefab
                Entity enemy = m_Coordinator.CreateEntity();
                {
                    m_Coordinator.AddComponent(enemy, Enemy{ .mSpeed = 1.f });

                    m_Coordinator.AddComponent(enemy, RigidBody{
                        .velocity = Vec2(0.0f, 0.0f),
                        .acceleration = Vec2(0.0f, 0.0f),
                        .accel_strength = 200,
                        .fric_coeff = 100
                        });

                    m_Coordinator.AddComponent(enemy, Transform{
                        .position = Vec2(-10, 0),
                        .rotation = Vec2(0, 0),
                        .scale = Vec2(1.f, 1.f)
                        });

                    std::string texName = "pink_enemy";
                    m_Coordinator.AddComponent(enemy, Sprite{
                        .textureName = texName,
                        .flipX = false,
                        .flipY = false,
                        .UseNativeSize = true,
                        .texture = m_ResourcesManager->GetTexture(texName),
                        });

                    Collider enemyCollider;
                    enemyCollider.shapes[0] = ColliderShape{
                        .size = Vec2(3.f, 3.f),
                        .offset = Vec2(0.f, 1.f),
                        .purpose = ColliderPurpose::Physics,
                        .layer = CL_ENEMY,
                        .colliderMask = CL_PLAYER | CL_PROJECTILE,
                        .isActive = true,
                        .autoFitToSprite = false
                    };

                    enemyCollider.shapes.push_back(ColliderShape{
                        .size = Vec2(2.f, 0.5f),
                        .offset = Vec2(0.f, -2.f),
                        .purpose = ColliderPurpose::Environment,
                        .layer = CL_WALL,
                        .colliderMask = CL_WALL,
                        .isActive = true,
                        .autoFitToSprite = false
                        });

                    enemyCollider.bounds.resize(enemyCollider.shapes.size());
                    m_Coordinator.AddComponent(enemy, enemyCollider);
                }

                // Save as prefab
                GameSerializer serializer;
                serializer.Register(m_ResourcesManager);
                serializer.Register(&m_Coordinator);
                serializer.savePrefab(enemy, Uma_FilePath::PREFAB_DIR + "enemy.json");

                Transform& tf = m_Coordinator.GetComponent<Transform>(enemy);
                tf.position = Vec2(randPos(generator), randPos(generator));
            }
            else
            {
                // Duplicate existing entity
                Entity tmp = m_Coordinator.DuplicateEntity(eArray.GetEntity(0));
                Transform& tf = m_Coordinator.GetComponent<Transform>(tmp);
                tf.position = Vec2(randPos(generator), randPos(generator));

                Sprite& sr = m_Coordinator.GetComponent<Sprite>(tmp);
                sr.textureName = (randPos(generator) > 0.f) ? "pink_enemy" : "enemy";
                sr.texture = m_ResourcesManager->GetTexture(sr.textureName);
            }
        }

        void DestroyRandomEntity()
        {
            using namespace Uma_ECS;

            auto& eArray = m_Coordinator.GetComponentArray<Enemy>();
            if (eArray.Size() != 0)
            {
                m_Coordinator.DestroyEntity(eArray.GetEntity(0));
            }
        }

        void LoadPrefab()
        {
            GameSerializer serializer;
            serializer.Register(m_ResourcesManager);
            serializer.Register(&m_Coordinator);
            serializer.loadPrefab(Uma_FilePath::PREFAB_DIR + "enemy.json");
        }

        void ChangeAllEnemyRot(float rot)
        {
            using namespace Uma_ECS;

            auto& eArray = m_Coordinator.GetComponentArray<Enemy>();
            auto& tfArray = m_Coordinator.GetComponentArray<Transform>();

            for (size_t i = 0; i < eArray.Size(); i++)
            {
                auto& tf = tfArray.GetData(eArray.GetEntity(i));
                tf.rotation.y = rot;
            }
        }

        void ChangeAllEnemyXPos(float xPos)
        {
            using namespace Uma_ECS;

            auto& eArray = m_Coordinator.GetComponentArray<Enemy>();
            auto& rbArray = m_Coordinator.GetComponentArray<RigidBody>();

            for (size_t i = 0; i < eArray.Size(); i++)
            {
                auto& rb = rbArray.GetData(eArray.GetEntity(i));
                rb.acceleration = Vec2{ 5000.f * xPos, 0 };
            }
        }

        void ChangeAllEnemyScale(float scale)
        {
            using namespace Uma_ECS;

            std::default_random_engine generator(std::random_device{}());
            std::uniform_real_distribution<float> randScale(10.0f, 15.0f);

            auto& eArray = m_Coordinator.GetComponentArray<Enemy>();
            auto& tfArray = m_Coordinator.GetComponentArray<Transform>();

            for (size_t i = 0; i < eArray.Size(); i++)
            {
                auto& tf = tfArray.GetData(eArray.GetEntity(i));
                tf.scale = Vec2{ randScale(generator), randScale(generator) } *scale;
            }
        }

        void ShowBBox(bool isShow)
        {
            using namespace Uma_ECS;

            auto& tfArray = m_Coordinator.GetComponentArray<Transform>();
            auto& cArray = m_Coordinator.GetComponentArray<Collider>();

            for (size_t i = 0; i < tfArray.Size(); i++)
            {
                if (!cArray.Has(tfArray.GetEntity(i)))
                    continue;

                auto& c = cArray.GetData(tfArray.GetEntity(i));
                c.showBBox = isShow;
            }
        }

	};
}