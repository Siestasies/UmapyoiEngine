/*!
\file   EditorSceneScript.h
\par    Project: GAM200

\brief
Editor behavior script that handles all editor-specific functionality.
This replaces the old EditorScene class inheritance approach.
*/
#pragma once
#include "SceneType.h"
#include "Core/GameSerializer.h"
#include "Core/FilePaths.h"
#include <random>

namespace Uma_Engine
{
    class EditorSceneScript : public SceneScript
    {
    public:
        EditorSceneScript() : SceneScript("EditorBehavior") {}

        void OnAttach(Scene* scene) override
        {
            SceneScript::OnAttach(scene);
            std::cout << "EditorSceneScript attached" << std::endl;
        }

        void OnLoad() override
        {
            std::cout << "EditorSceneScript: OnLoad" << std::endl;

            // Subscribe to editor events
            SubscribeToEvents();
        }

        void OnUnload() override
        {
            std::cout << "EditorSceneScript: OnUnload" << std::endl;
            // Events will be automatically unsubscribed when scene is destroyed
        }

        void OnUpdate(float dt) override
        {
            // Handle editor-specific keyboard shortcuts
            HandleEditorInput();
        }

    private:
        std::string m_CurrentSceneName = "test_collider.json";

        void SubscribeToEvents()
        {
            auto eventSystem = GetEvents();
            auto& coordinator = GetCoordinator();

            // Query active entities
            eventSystem->Subscribe<QueryActiveEntitiesEvent>(
                [&coordinator](const QueryActiveEntitiesEvent& e) {
                    e.mActiveEntityCnt = coordinator.GetEntityCount();
                }
            );

            // Save scene
            eventSystem->Subscribe<SaveSceneRequestEvent>(
                [this](const SaveSceneRequestEvent& e) {
                    (void)e;
                    SaveScene();
                }
            );

            // Load scene
            eventSystem->Subscribe<LoadSceneRequestEvent>(
                [this](const LoadSceneRequestEvent& e) {
                    (void)e;
                    LoadScene();
                }
            );

            // Clear scene
            eventSystem->Subscribe<ClearSceneRequestEvent>(
                [this](const ClearSceneRequestEvent& e) {
                    (void)e;
                    ResetScene();
                }
            );

            // Stress test
            eventSystem->Subscribe<StressTestRequestEvent>(
                [this](const StressTestRequestEvent& e) {
                    (void)e;
                    StressTest();
                }
            );

            // Show default entities in viewport
            eventSystem->Subscribe<ShowEntityInVPRequestEvent>(
                [this](const ShowEntityInVPRequestEvent& e) {
                    (void)e;
                    SpawnDefaultEntities();
                }
            );

            // Change enemy rotation
            eventSystem->Subscribe<ChangeEnemyRotRequestEvent>(
                [this](const ChangeEnemyRotRequestEvent& e) {
                    ChangeAllEnemyRot(e.rot);
                }
            );

            // Change enemy X position
            eventSystem->Subscribe<ChangeEnemyXposRequestEvent>(
                [this](const ChangeEnemyXposRequestEvent& e) {
                    ChangeAllEnemyXPos(e.xpos);
                }
            );

            // Change enemy scale
            eventSystem->Subscribe<ChangeEnemyScaleRequestEvent>(
                [this](const ChangeEnemyScaleRequestEvent& e) {
                    ChangeAllEnemyScale(e.scale);
                }
            );

            // Show bounding boxes
            eventSystem->Subscribe<ShowBBoxRequestEvent>(
                [this](const ShowBBoxRequestEvent& e) {
                    ShowBBox(e.show);
                }
            );

            // Clone entity
            eventSystem->Subscribe<CloneEntityRequestEvent>(
                [this](const CloneEntityRequestEvent& e) {
                    (void)e;
                    DuplicateOrCreateEntity();
                }
            );

            // Load prefab
            eventSystem->Subscribe<LoadPrefabRequestEvent>(
                [this](const LoadPrefabRequestEvent& e) {
                    (void)e;
                    LoadPrefab();
                }
            );

            // Destroy entity
            eventSystem->Subscribe<DestroyEntityRequestEvent>(
                [this](const DestroyEntityRequestEvent& e) {
                    (void)e;
                    DestroyRandomEntity();
                }
            );
        }

        void HandleEditorInput()
        {
            auto input = GetInput();

            // Save to file
            if (input->KeyPressed(GLFW_KEY_1))
            {
                SaveScene();
            }

            // Load from file
            if (input->KeyPressed(GLFW_KEY_2))
            {
                LoadScene();
            }

            // Reset scene
            if (input->KeyPressed(GLFW_KEY_3))
            {
                ResetScene();
            }

            // Spawn default entities
            if (input->KeyPressed(GLFW_KEY_4))
            {
                GetCoordinator().DestroyAllEntities();
                SpawnDefaultEntities();
            }

            // Play sound effects
            if (input->KeyPressed(GLFW_KEY_P))
            {
                GetSound()->playSound(GetResources()->GetSound("explosion"));
            }

            if (input->KeyPressed(GLFW_KEY_O))
            {
                GetSound()->playSound(GetResources()->GetSound("cave"));
            }
        }

        void SaveScene()
        {
            std::string filepath = Uma_FilePath::SCENES_DIR + m_CurrentSceneName;

            GameSerializer serializer;
            serializer.Register(GetResources());
            serializer.Register(&GetCoordinator());
            serializer.save(filepath);

            std::cout << "Scene saved to: " << filepath << std::endl;
        }

        void LoadScene()
        {
            GetCoordinator().DestroyAllEntities();

            std::string filepath = Uma_FilePath::SCENES_DIR + m_CurrentSceneName;

            GameSerializer serializer;
            serializer.Register(GetResources());
            serializer.Register(&GetCoordinator());
            serializer.load(filepath);

            std::cout << "Scene loaded from: " << filepath << std::endl;
        }

        void ResetScene()
        {
            using namespace Uma_ECS;

            GetCoordinator().DestroyAllEntities();

            // Create player
            Entity player = m_Scene->CreateEntity();
            {
                GetCoordinator().AddComponent(player, Transform{
                    .position = Vec2(0.f, 0.f),
                    .rotation = Vec2(0.f, 0.f),
                    .scale = Vec2(1, 1),
                    });

                GetCoordinator().AddComponent(player, RigidBody{
                    .velocity = Vec2(0.0f, 0.0f),
                    .acceleration = Vec2(0.0f, 0.0f),
                    .accel_strength = 500,
                    .fric_coeff = 5
                    });

                GetCoordinator().AddComponent(player, Player{ .mSpeed = 1.f });

                std::string texName = "player";
                GetCoordinator().AddComponent(player, Sprite{
                    .textureName = texName,
                    .flipX = false,
                    .flipY = false,
                    .UseNativeSize = true,
                    .texture = GetResources()->GetTexture(texName),
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
                GetCoordinator().AddComponent(player, playerCollider);
            }

            // Create camera
            Entity cam = m_Scene->CreateEntity();
            {
                GetCoordinator().AddComponent(cam, Transform{
                    .position = Vec2(400.0f, 300.0f),
                    .rotation = Vec2(0, 0),
                    .scale = Vec2(1, 1),
                    });

                GetCoordinator().AddComponent(player, Camera{
                    .mZoom = 1.f,
                    .followPlayer = true
                    });
            }
        }

        void SpawnDefaultEntities()
        {
            GetCoordinator().DestroyAllEntities();

            using namespace Uma_ECS;

            Entity kappa;
            {
                kappa = GetCoordinator().CreateEntity();

                GetCoordinator().AddComponent(
                    kappa,
                    RigidBody{
                      .velocity = Vec2(0.0f, 0.0f),
                      .acceleration = Vec2(0.0f, 0.0f),
                      .accel_strength = 200,
                      .fric_coeff = 100
                    });

                GetCoordinator().AddComponent(
                    kappa,
                    Transform{
                      .position = Vec2(30, 35),
                      .rotation = Vec2(0, 0),
                      .scale = Vec2(3.f, 3.f)
                    });

                std::string texName = "kappa_statue";
                GetCoordinator().AddComponent(
                    kappa,
                    Sprite{
                      .textureName = texName,
                      .renderLayer = RL_ENV,
                      .flipX = false,
                      .flipY = false,
                      .UseNativeSize = true,
                      .texture = GetResources()->GetTexture(texName),
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

                    GetCoordinator().AddComponent(kappa, kappaScriptComponent);
                }
            }

            Entity wall;
            {
                wall = GetCoordinator().CreateEntity();

                GetCoordinator().AddComponent(
                    wall,
                    RigidBody{
                      .velocity = Vec2(0.0f, 0.0f),
                      .acceleration = Vec2(0.0f, 0.0f),
                      .accel_strength = 200,
                      .fric_coeff = 100
                    });

                GetCoordinator().AddComponent(
                    wall,
                    Transform{
                      .position = Vec2(-20, 0),
                      .rotation = Vec2(0, 0),
                      .scale = Vec2(1.f, 1.f)
                    });

                std::string texName = "wall_top";
                GetCoordinator().AddComponent(
                    wall,
                    Sprite{
                      .textureName = texName,
                      .renderLayer = RL_WALL_TOP,
                      .flipX = false,
                      .flipY = false,
                      .UseNativeSize = true,
                      .texture = GetResources()->GetTexture(texName),
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
                GetCoordinator().AddComponent(wall, wallCollider);

                for (size_t i = 0; i < 5; i++)
                {
                    Entity tmp = GetCoordinator().DuplicateEntity(wall);
                
                    Transform& tf = GetCoordinator().GetComponent<Transform>(tmp);
                    tf.position = Vec2(20 + (i * 5), 0);
                
                    Collider& collider = GetCoordinator().GetComponent<Collider>(tmp);
                    collider.shapes[0].autoFitToSprite = false;
                    collider.shapes[0].size = Vec2(5, 1);
                    collider.shapes[0].offset = Vec2(0, -2.0);
                    collider.shapes[0].layer = CL_WALL;
                    collider.shapes[0].colliderMask = CL_PLAYER | CL_ENEMY;  // Blocks entities,
                
                    Sprite& sr = GetCoordinator().GetComponent<Sprite>(tmp);
                
                    // set texture randomly
                    sr.textureName = "wall_btm";
                    sr.renderLayer = RL_WALL_BTM;
                    sr.texture = GetResources()->GetTexture(sr.textureName);
                }

                for (size_t i = 0; i < 8; i++)
                {
                    Entity tmp = GetCoordinator().DuplicateEntity(wall);

                    Transform& tf = GetCoordinator().GetComponent<Transform>(tmp);

                    tf.position = Vec2(15 + (6 * 5), 0 + (i * 5));

                    Sprite& sr = GetCoordinator().GetComponent<Sprite>(tmp);

                    // set texture randomly
                    sr.textureName = "wall_right";
                    sr.texture = GetResources()->GetTexture(sr.textureName);
                }

                for (size_t i = 0; i < 5; i++)
                {
                    Entity tmp = GetCoordinator().DuplicateEntity(wall);

                    Transform& tf = GetCoordinator().GetComponent<Transform>(tmp);

                    tf.position = Vec2(20 + (i * 5), 15 + (4 * 5));

                    Sprite& sr = GetCoordinator().GetComponent<Sprite>(tmp);

                    // set texture randomly
                    sr.textureName = "wall_top";
                    sr.texture = GetResources()->GetTexture(sr.textureName);
                }
            }

            GetCoordinator().DestroyEntity(wall);

            Entity floor;
            {
                floor = GetCoordinator().CreateEntity();

                GetCoordinator().AddComponent(
                    floor,
                    RigidBody{
                      .velocity = Vec2(0.0f, 0.0f),
                      .acceleration = Vec2(0.0f, 0.0f),
                      .accel_strength = 200,
                      .fric_coeff = 100
                    });

                GetCoordinator().AddComponent(
                    floor,
                    Transform{
                      .position = Vec2(20, 7.5),
                      .rotation = Vec2(0, 0),
                      .scale = Vec2(2.f, 2.f)
                    });

                std::string texName = "floor_tatami";
                GetCoordinator().AddComponent(
                    floor,
                    Sprite{
                      .textureName = texName,
                      .renderLayer = RL_FLOOR,
                      .flipX = false,
                      .flipY = false,
                      .UseNativeSize = true,
                      .texture = GetResources()->GetTexture(texName),
                    });

                for (size_t i = 0; i < 5; i++)
                {
                    for (size_t j = 0; j < 3; j++)
                    {
                        Entity tmp = GetCoordinator().DuplicateEntity(floor);

                        Transform& tf = GetCoordinator().GetComponent<Transform>(tmp);

                        tf.position = Vec2(20 + (i * 5), 7.5 + (j * 10));
                    }
                }
            }

            // create entities
            {
                Entity enemy;
                {
                    enemy = GetCoordinator().CreateEntity();

                    GetCoordinator().AddComponent(
                        enemy,
                        Enemy{
                            .mSpeed = 1.f
                        });

                    GetCoordinator().AddComponent(
                        enemy,
                        RigidBody{
                          .velocity = Vec2(0.0f, 0.0f),
                          .acceleration = Vec2(0.0f, 0.0f),
                          .accel_strength = 500,
                          .fric_coeff = 5
                        });

                    GetCoordinator().AddComponent(
                        enemy,
                        Transform{
                          .position = Vec2(-10, 0),
                          .rotation = Vec2(0, 0),
                          .scale = Vec2(2.f, 2.f)
                        });

                    std::string texName = "pink_enemy";
                    GetCoordinator().AddComponent(
                        enemy,
                        Sprite{
                          .textureName = texName,
                          .renderLayer = RL_ENEMY,
                          .flipX = false,
                          .flipY = false,
                          .UseNativeSize = true,
                          .texture = GetResources()->GetTexture(texName),
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
                        .size = Vec2(2.f, 0.7f),
                        .offset = Vec2(0.f, -2.0f),  // Changed from -2.f to -1.0f
                        .purpose = ColliderPurpose::Physics,
                        .layer = CL_ENEMY,
                        .colliderMask = CL_WALL,
                        .isActive = true,
                        .autoFitToSprite = false
                                            });

                    enemyCollider.bounds.resize(enemyCollider.shapes.size());
                    GetCoordinator().AddComponent(enemy, enemyCollider);

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

                        GetCoordinator().AddComponent(enemy, enemyScriptComponent);
                    }
                }

                // using 1 enemy to duplicate 2500 times and rand its transform
                /*for (size_t i = 0; i < 2500 - 3; i++)
                {
                    Entity tmp = GetCoordinator().DuplicateEntity(enemy);

                    Transform& tf = GetCoordinator().GetComponent<Transform>(tmp);

                    tf.position = Vec2(randPositionX(generator), randPositionY(generator));
                    tf.rotation = Vec2(0, 0);
                    tf.scale = Vec2(randScale(generator), randScale(generator));

                    Sprite& sr = GetCoordinator().GetComponent<Sprite>(tmp);

                    sr.textureName = (i > 1250) ? "pink_enemy" : "enemy";
                    sr.texture = pResourcesManager->GetTexture(sr.textureName);
                }*/
            }

            // create player
            m_Scene->m_player = GetCoordinator().CreateEntity();
            {
                GetCoordinator().AddComponent(
                    m_Scene->m_player,
                    Transform
                    {
                        .position = Vec2(0.f, 0.f),
                        .rotation = Vec2(0.f, 0.f),
                        .scale = Vec2(1,1),
                    });

                GetCoordinator().AddComponent(
                    m_Scene->m_player,
                    RigidBody{
                      .velocity = Vec2(0.0f, 0.0f),
                      .acceleration = Vec2(0.0f, 0.0f),
                      .accel_strength = 500,
                      .fric_coeff = 5
                    });

                GetCoordinator().AddComponent(
                    m_Scene->m_player,
                    Player{
                        .mSpeed = 1.f
                    });

                std::string texName = "player";
                GetCoordinator().AddComponent(
                    m_Scene->m_player,
                    Sprite{
                      .textureName = texName,
                      .renderLayer = RL_PLAYER,
                      .flipX = false,
                      .flipY = false,
                      .UseNativeSize = true,
                      .texture = GetResources()->GetTexture(texName),
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
                GetCoordinator().AddComponent(m_Scene->m_player, playerCollider);
            }

            // create camera
            m_Scene->m_cam = GetCoordinator().CreateEntity();
            {
                GetCoordinator().AddComponent(
                    m_Scene->m_cam,
                    Transform
                    {
                        .position = Vec2(400.0f, 300.0f),
                        .rotation = Vec2(0,0),
                        .scale = Vec2(1,1),
                    });

                GetCoordinator().AddComponent(
                    m_Scene->m_player,
                    Camera
                    {
                        .mZoom = 1.f,
                        .followPlayer = true
                    });
            }

            m_Scene->m_LuaScriptingSystem->CallStart();
        }

        void StressTest()
        {
            using namespace Uma_ECS;

            GetCoordinator().DestroyAllEntities();

            std::default_random_engine generator;
            std::uniform_real_distribution<float> randPosX(-1920.f, 1920.f);
            std::uniform_real_distribution<float> randPosY(-1080.f, 1080.f);

            // Create base enemy
            Entity enemy = m_Scene->CreateEntity();
            {
                GetCoordinator().AddComponent(enemy, Enemy{ .mSpeed = 1.f });
                GetCoordinator().AddComponent(enemy, RigidBody{
                    .velocity = Vec2(0.0f, 0.0f),
                    .acceleration = Vec2(0.0f, 0.0f),
                    .accel_strength = 200,
                    .fric_coeff = 100
                    });

                GetCoordinator().AddComponent(enemy, Transform{
                    .position = Vec2(-10, 0),
                    .rotation = Vec2(0, 0),
                    .scale = Vec2(1.f, 1.f)
                    });

                std::string texName = "pink_enemy";
                GetCoordinator().AddComponent(enemy, Sprite{
                    .textureName = texName,
                    .flipX = false,
                    .flipY = false,
                    .UseNativeSize = true,
                    .texture = GetResources()->GetTexture(texName),
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
                GetCoordinator().AddComponent(enemy, enemyCollider);
            }

            // Duplicate 10000 times
            for (size_t i = 0; i < 10000; i++)
            {
                Entity tmp = GetCoordinator().DuplicateEntity(enemy);
                Transform& tf = GetCoordinator().GetComponent<Transform>(tmp);
                tf.position = Vec2(randPosX(generator), randPosY(generator));
            }

            std::cout << "Stress test: 10000 entities spawned" << std::endl;
        }

        void DuplicateOrCreateEntity()
        {
            using namespace Uma_ECS;

            auto& eArray = GetCoordinator().GetComponentArray<Enemy>();

            std::default_random_engine generator(std::random_device{}());
            std::uniform_real_distribution<float> randPos(-400, 400);

            if (eArray.Size() == 0)
            {
                // Create new enemy and save as prefab
                Entity enemy = m_Scene->CreateEntity();
                {
                    GetCoordinator().AddComponent(enemy, Enemy{ .mSpeed = 1.f });

                    GetCoordinator().AddComponent(enemy, RigidBody{
                        .velocity = Vec2(0.0f, 0.0f),
                        .acceleration = Vec2(0.0f, 0.0f),
                        .accel_strength = 200,
                        .fric_coeff = 100
                        });

                    GetCoordinator().AddComponent(enemy, Transform{
                        .position = Vec2(-10, 0),
                        .rotation = Vec2(0, 0),
                        .scale = Vec2(1.f, 1.f)
                        });

                    std::string texName = "pink_enemy";
                    GetCoordinator().AddComponent(enemy, Sprite{
                        .textureName = texName,
                        .flipX = false,
                        .flipY = false,
                        .UseNativeSize = true,
                        .texture = GetResources()->GetTexture(texName),
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
                    GetCoordinator().AddComponent(enemy, enemyCollider);
                }

                // Save as prefab
                GameSerializer serializer;
                serializer.Register(GetResources());
                serializer.Register(&GetCoordinator());
                serializer.savePrefab(enemy, Uma_FilePath::PREFAB_DIR + "enemy.json");

                Transform& tf = GetCoordinator().GetComponent<Transform>(enemy);
                tf.position = Vec2(randPos(generator), randPos(generator));
            }
            else
            {
                // Duplicate existing entity
                Entity tmp = GetCoordinator().DuplicateEntity(eArray.GetEntity(0));
                Transform& tf = GetCoordinator().GetComponent<Transform>(tmp);
                tf.position = Vec2(randPos(generator), randPos(generator));

                Sprite& sr = GetCoordinator().GetComponent<Sprite>(tmp);
                sr.textureName = (randPos(generator) > 0.f) ? "pink_enemy" : "enemy";
                sr.texture = GetResources()->GetTexture(sr.textureName);
            }
        }

        void DestroyRandomEntity()
        {
            using namespace Uma_ECS;

            auto& eArray = GetCoordinator().GetComponentArray<Enemy>();
            if (eArray.Size() != 0)
            {
                GetCoordinator().DestroyEntity(eArray.GetEntity(0));
            }
        }

        void LoadPrefab()
        {
            GameSerializer serializer;
            serializer.Register(GetResources());
            serializer.Register(&GetCoordinator());
            serializer.loadPrefab(Uma_FilePath::PREFAB_DIR + "enemy.json");
        }

        void ChangeAllEnemyRot(float rot)
        {
            using namespace Uma_ECS;

            auto& eArray = GetCoordinator().GetComponentArray<Enemy>();
            auto& tfArray = GetCoordinator().GetComponentArray<Transform>();

            for (size_t i = 0; i < eArray.Size(); i++)
            {
                auto& tf = tfArray.GetData(eArray.GetEntity(i));
                tf.rotation.y = rot;
            }
        }

        void ChangeAllEnemyXPos(float xPos)
        {
            using namespace Uma_ECS;

            auto& eArray = GetCoordinator().GetComponentArray<Enemy>();
            auto& rbArray = GetCoordinator().GetComponentArray<RigidBody>();

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

            auto& eArray = GetCoordinator().GetComponentArray<Enemy>();
            auto& tfArray = GetCoordinator().GetComponentArray<Transform>();

            for (size_t i = 0; i < eArray.Size(); i++)
            {
                auto& tf = tfArray.GetData(eArray.GetEntity(i));
                tf.scale = Vec2{ randScale(generator), randScale(generator) } *scale;
            }
        }

        void ShowBBox(bool isShow)
        {
            using namespace Uma_ECS;

            auto& tfArray = GetCoordinator().GetComponentArray<Transform>();
            auto& cArray = GetCoordinator().GetComponentArray<Collider>();

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