/*!
\file   EditorScript.h
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

// events
#include <Events/AudioEvents.h>

namespace Uma_Engine
{
    class EditorScript : public SceneScript
    {
    public:
        EditorScript() : SceneScript("Editor") {}

        void OnAttach(Scene* scene) override
        {
            SceneScript::OnAttach(scene);
            m_CurrentSceneName = scene->GetFilePath();
            std::cout << "EditorScript attached" << std::endl;
        }

        void OnLoad() override
        {
            std::cout << "EditorScript: OnLoad" << std::endl;

            //m_Canvas = m_Scene->CreateEntity();
            //GetCoordinator().AddComponent<Uma_UI::RectTransform>(m_Canvas, 
            //    {
            //    .anchorMin = Vec2(0.0f, 0.0f),      // Bottom-left
            //    .anchorMax = Vec2(1.0f, 1.0f),      // Top-right (stretch)
            //    .pivot = Vec2(0.5f, 0.5f),          // Center pivot
            //    .anchoredPosition = Vec2(0, 0),     // No offset
            //    .sizeDelta = Vec2(0, 0),            // Stretch to fill
            //    .parent = static_cast<Uma_ECS::Entity>(-1)  // Root
            //    });


            //GetCoordinator().AddComponent<Uma_UI::Canvas>(m_Canvas,
            //    {
            //    .sortingOrder = 0,
            //    .referenceResolution = Vec2(1280.f, 720.f),
            //    .scaleMode = Uma_UI::CanvasScaleMode::ScaleWithScreenSize,
            //    .matchWidthOrHeight = 0.5f
            //    });

            //if (!GetResources()->GetTexture("whitePixel")) GetResources()->LoadTexture("whitePixel", "Assets/whitePixel.png");

            //// Ensure font exists (size 48)
            ////GetGraphics()->LoadFont("default", "Assets/Fonts/Neucha.ttf", 48);
            //GetResources()->LoadFont("default", Uma_FilePath::FONTS_DIR + "Neucha.ttf", 48);


            //// Subscribe to editor events
            SubscribeToEvents();

            //CreateButtonWithText("Hello", Vec2(0.f, 0.f), Vec2(200.f, 50.f), m_Canvas,
            //    [](Uma_ECS::Entity btn){std::cout << "[UI] Button clicked! entity=" << btn << std::endl;});
        }

        void OnUnload() override
        {
            std::cout << "EditorScript: OnUnload" << std::endl;
        
            // unsub the events
            UnsubscribeEvents();
        }

        void OnUpdate(float dt) override
        {
            UNREFERENCED_PARAMETER(dt);
            HandleEditorInput();
        }

    private:
        std::string m_CurrentSceneName;

        Entity m_Canvas{};

        std::vector<std::shared_ptr<IEventListener>> m_EventListeners;

        void SubscribeToEvents()
        {
            auto eventSystem = GetEventSystem();
            auto& coordinator = GetCoordinator();

            // Query active entities
            m_EventListeners.push_back(
                eventSystem->Subscribe<QueryActiveEntitiesEvent>(
                    [&coordinator](const QueryActiveEntitiesEvent& e) {
                        e.mActiveEntityCnt = coordinator.GetEntityCount();
                    }
                ));

            // Save scene
            m_EventListeners.push_back(
            eventSystem->Subscribe<SaveSceneRequestEvent>(
                [this](const SaveSceneRequestEvent& e) {
                    (void)e;
                    SaveScene();
                }
            ));

            // Load scene from path (for new scenes)
            //eventSystem->Subscribe<LoadSceneRequestEvent>(
            //    [this](const LoadSceneRequestEvent& e) {
            //        LoadScene(e.filepath);
            //    }
            //);

            // reload the current scene
            m_EventListeners.push_back(
            eventSystem->Subscribe<ReLoadSceneRequestEvent>(
                [this](const ReLoadSceneRequestEvent& e) {
                    (void)e;
                    ReLoadScene();
                }
            ));

            // Clear scene
            m_EventListeners.push_back(
            eventSystem->Subscribe<ClearSceneRequestEvent>(
                [this](const ClearSceneRequestEvent& e) {
                    (void)e;
                    ResetScene();
                }
            ));

            // Stress test
            m_EventListeners.push_back(
            eventSystem->Subscribe<StressTestRequestEvent>(
                [this](const StressTestRequestEvent& e) {
                    (void)e;
                    StressTest();
                }
            ));

            // Show default entities in viewport
            m_EventListeners.push_back(
            eventSystem->Subscribe<ShowEntityInVPRequestEvent>(
                [this](const ShowEntityInVPRequestEvent& e) {
                    (void)e;
                    SpawnDefaultEntities();
                }
            ));

            // Change enemy rotation
            m_EventListeners.push_back(
            eventSystem->Subscribe<ChangeEnemyRotRequestEvent>(
                [this](const ChangeEnemyRotRequestEvent& e) {
                    ChangeAllEnemyRot(e.rot);
                }
            ));

            // Change enemy X position
            m_EventListeners.push_back(
            eventSystem->Subscribe<ChangeEnemyXposRequestEvent>(
                [this](const ChangeEnemyXposRequestEvent& e) {
                    ChangeAllEnemyXPos(e.xpos);
                }
            ));

            // Change enemy scale
            m_EventListeners.push_back(
            eventSystem->Subscribe<ChangeEnemyScaleRequestEvent>(
                [this](const ChangeEnemyScaleRequestEvent& e) {
                    ChangeAllEnemyScale(e.scale);
                }
            ));

            // Show bounding boxes
            m_EventListeners.push_back(
            eventSystem->Subscribe<ShowBBoxRequestEvent>(
                [this](const ShowBBoxRequestEvent& e) {
                    ShowBBox(e.show);
                }
            ));

            // Clone entity
            m_EventListeners.push_back(
            eventSystem->Subscribe<CloneEntityRequestEvent>(
                [this](const CloneEntityRequestEvent& e) {
                    (void)e;
                    DuplicateOrCreateEntity();
                }
            ));

            // Load prefab
            m_EventListeners.push_back(
            eventSystem->Subscribe<LoadPrefabRequestEvent>(
                [this](const LoadPrefabRequestEvent& e) {
                    (void)e;
                    LoadPrefab("bird");
                }
            ));

            // Destroy entity
            m_EventListeners.push_back(
            eventSystem->Subscribe<DestroyEntityRequestEvent>(
                [this](const DestroyEntityRequestEvent& e) {
                    (void)e;
                    DestroyRandomEntity();
                }
            ));
        }

        void UnsubscribeEvents()
        {
            auto eventSystem = GetEventSystem();

            for (auto listener : m_EventListeners)
            {
                eventSystem->UnsubscribeListener(listener);
            }

            m_EventListeners.clear();

            std::cout << "EditorScript: Unsubscribed from all events" << std::endl;
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
                ReLoadScene();
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
                //GetSound()->playSound(GetResources()->GetSound("explosion"));
                //m_Scene->m_EventSystem->Emit<Uma_Engine::PlaySoundEvent>("explosion", 1, 0);

                //used for entities with position
                m_Scene->m_EventSystem->Emit<Uma_Engine::PlaySound3DEvent>("explosion", 0, 0, 1, 0);
            }

            if (input->KeyPressed(GLFW_KEY_O))
            {
                //GetSound()->playSound(GetResources()->GetSound("cave"));
                m_Scene->m_EventSystem->Emit<Uma_Engine::PlaySoundEvent>("cave", 1, 0);
            }
        }

        void SaveScene()
        {
            // SAVE to scene path that this script is attached to
            std::string filepath = m_CurrentSceneName;

            m_Scene->gGameSerializer.save(filepath);

            std::cout << "Scene saved to: " << filepath << std::endl;
        }

        //void LoadScene(const std::string& filepath)
        //{
        //    GetCoordinator().DestroyAllEntities();

        //    m_Scene->gGameSerializer.load(filepath);

        //    std::cout << "Scene loaded from: " << filepath << std::endl;
        //}

        void ReLoadScene()
        {
            GetCoordinator().DestroyAllEntities();

            std::string filepath = m_CurrentSceneName;

            m_Scene->gGameSerializer.load(filepath);

            std::cout << "Scene loaded from: " << filepath << std::endl;
        }

        void ResetScene()
        {
            using namespace Uma_ECS;

            GetLuascriptingSystem().Shutdown();
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

                GetCoordinator().AddComponent(cam, Camera{
                    .mZoom = 1.f,
                    .followPlayer = true
                    });
            }

            GetLuascriptingSystem().Restart();
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
                      .tintColor = Vec3(1.0f, 0.5f, 1.0f),
                      .alpha = 0.5f
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

                for (int i = 0; i < 5; i++)
                {
                    Entity tmp = GetCoordinator().DuplicateEntity(wall);
                
                    Transform& tf = GetCoordinator().GetComponent<Transform>(tmp);
                    tf.position = Vec2(static_cast<float>(20 + (i * 5)), 0.f);
                
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

                for (int i = 0; i < 8; i++)
                {
                    Entity tmp = GetCoordinator().DuplicateEntity(wall);

                    Transform& tf = GetCoordinator().GetComponent<Transform>(tmp);

                    tf.position = Vec2(static_cast<float>(15 + (6 * 5)), static_cast<float>(0 + (i * 5)) );

                    Sprite& sr = GetCoordinator().GetComponent<Sprite>(tmp);

                    // set texture randomly
                    sr.textureName = "wall_right";
                    sr.texture = GetResources()->GetTexture(sr.textureName);
                }

                for (int i = 0; i < 5; i++)
                {
                    Entity tmp = GetCoordinator().DuplicateEntity(wall);

                    Transform& tf = GetCoordinator().GetComponent<Transform>(tmp);

                    tf.position = Vec2(static_cast<float>(20 + (i * 5)), static_cast<float>(15 + (4 * 5)) );

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

                for (int i = 0; i < 5; i++)
                {
                    for (int j = 0; j < 3; j++)
                    {
                        Entity tmp = GetCoordinator().DuplicateEntity(floor);

                        Transform& tf = GetCoordinator().GetComponent<Transform>(tmp);

                        tf.position = Vec2(static_cast<float>(20 + (i * 5)), static_cast<float>(7.5 + (j * 10)) );
                    }
                }
            }

            // create entities
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
                    enemyScriptComponent.AddScript(Uma_FilePath::SCRIPT_DIR + "testEnemy.lua");

                    enemyScriptComponent.GetScript(1)->exposedVariables.push_back(Uma_ECS::LuaVariable{
                       .name = "speed",
                       .value = 100.0f,
                       .type = Uma_ECS::LuaVarType::T_FLOAT,
                       .min = 0.0f,
                       .max = 500.0f,
                       .isSlider = true
                        });

                    enemyScriptComponent.GetScript(1)->exposedVariables.push_back(Uma_ECS::LuaVariable{
                       .name = "name",
                       .value = "bird",
                       .type = Uma_ECS::LuaVarType::T_STRING,
                       .isSlider = false
                        });

                    GetCoordinator().AddComponent(enemy, enemyScriptComponent);
                }
            }
            {
                Entity en = GetCoordinator().CreateEntity();

                GetCoordinator().AddComponent(
                    en,
                    RigidBody{
                      .velocity = Vec2(0.0f, 0.0f),
                      .acceleration = Vec2(0.0f, 0.0f),
                      .accel_strength = 500,
                      .fric_coeff = 5
                    });

                GetCoordinator().AddComponent(
                    en,
                    Transform{
                      .position = Vec2(-2, 0),
                      .rotation = Vec2(0, 0),
                      .scale = Vec2(0.5f, 0.5f)
                    });

                std::string texName = "kappa_statue";
                GetCoordinator().AddComponent(
                    en,
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
                    .size = Vec2(2.f, 2.f),
                    .offset = Vec2(0.f, 0.f),  // Changed from -2.f to -1.0f
                    .purpose = ColliderPurpose::Physics,
                    .layer = CL_ENEMY,
                    .colliderMask = CL_WALL,
                    .isActive = true,
                    .autoFitToSprite = false
                };
                enemyCollider.bounds.resize(enemyCollider.shapes.size());
                GetCoordinator().AddComponent(en, enemyCollider);

                GetCoordinator().SetParent(en, enemy);
            }
            SavePrefab("bird", enemy);

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
                      .accel_strength = 300,
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
                    .size = Vec2(7.0f, 2.f),
                    .offset = Vec2(0, -2.75f),
                    .purpose = ColliderPurpose::Physics,
                    .layer = CL_PLAYER,
                    .colliderMask = CL_WALL,
                    .isActive = true,
                    .autoFitToSprite = false
                });

                playerCollider.bounds.resize(playerCollider.shapes.size());
                GetCoordinator().AddComponent(m_Scene->m_player, playerCollider);

                AudioListener audioListner;
                GetCoordinator().AddComponent(m_Scene->m_player, audioListner);
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
                    m_Scene->m_cam,
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
            // please do this only in debug mode
            // so what the test is about:
            // 10k entities
            // rand position rand velocity with texture
            // without collision
            // without lua scripts
            using namespace Uma_ECS;
            GetLuascriptingSystem().Shutdown();
            GetCoordinator().DestroyAllEntities();
           
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
                          .accel_strength = 300,
                          .fric_coeff = 0
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
                }

                // using 1 enemy to duplicate 2500 times and rand its transform
                std::random_device rd;
                std::mt19937 generator(rd());

                // Define spawn area (adjust these values to fit your level bounds)
                std::uniform_real_distribution<float> randPositionX(-50.0f, 100.0f);
                std::uniform_real_distribution<float> randPositionY(-50.0f, 100.0f);

                for (size_t i = 0; i < 10000; i++)
                {
                    Entity tmp = GetCoordinator().DuplicateEntity(enemy);

                    Transform& tf = GetCoordinator().GetComponent<Transform>(tmp);
                    tf.position = Vec2(randPositionX(generator), randPositionY(generator));
                    tf.rotation = Vec2(0, 0);

                    RigidBody& rb = GetCoordinator().GetComponent<RigidBody>(tmp);

                    // Random velocity distribution (adjust ranges as needed)
                    std::uniform_real_distribution<float> randVelocity(-50.0f, 50.0f);

                    rb.velocity = Vec2(randVelocity(generator), randVelocity(generator));
                }
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
                      .accel_strength = 300,
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
                    m_Scene->m_cam,
                    Camera
                    {
                        .mZoom = 1.f,
                        .followPlayer = true
                    });
            }
            GetLuascriptingSystem().Restart();
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

        void LoadPrefab(std::string prefab_name)
        {
           m_Scene->gGameSerializer.loadPrefab(Uma_FilePath::PREFAB_DIR + prefab_name + ".prefab");
        }

        void SavePrefab(std::string prefab_name, Entity entity)
        {
            m_Scene->gGameSerializer.savePrefab(entity, Uma_FilePath::PREFAB_DIR + prefab_name + ".prefab");
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

        Uma_ECS::Entity CreateButtonWithText(
            const std::string& label,
            Vec2               anchoredPos,
            Vec2               size,
            Uma_ECS::Entity    canvas,
            std::function<void(Uma_ECS::Entity)> onClick)
        {
            using namespace Uma_ECS;
            Coordinator& coord = GetCoordinator();

            /* ---------- button entity ---------- */
            Entity btn = m_Scene->CreateEntity();

            coord.AddComponent<Uma_UI::RectTransform>(btn, 
                {
                .anchorMin = Vec2(0.5f, 0.5f),
                .anchorMax = Vec2(0.5f, 0.5f),
                .pivot = Vec2(0.5f, 0.5f),
                .anchoredPosition = anchoredPos,
                .sizeDelta = size,
                .parent = canvas
                });

            coord.AddComponent<Uma_UI::Image>(btn, 
                {
                .textureName = "whitePixel",
                .colour = Uma_UI::Colour(0.2f, 0.6f, 1.f, 1.f),
                .visible = true
                });

            coord.AddComponent<Uma_UI::Button>(btn, 
                {
                .interactable = true,
                .normalColour = Uma_UI::Colour(0.2f, 0.6f, 1.f, 1.f),
                .hoverColour = Uma_UI::Colour(0.3f, 0.7f, 1.f, 1.f),
                .pressedColour = Uma_UI::Colour(0.1f, 0.4f, 0.9f, 1.f),
                .disabledColour = Uma_UI::Colour(0.5f, 0.5f, 0.5f, 0.5f)
                });

            auto& button = coord.GetComponent<Uma_UI::Button>(btn);
            button.onClick = std::move(onClick);

            /* ---------- text child ---------- */
            Entity txt = m_Scene->CreateEntity();
            coord.AddComponent<Uma_UI::RectTransform>(txt, 
                {
                .anchorMin = Vec2(0.5f, 0.5f),
                .anchorMax = Vec2(0.5f, 0.5f),
                .pivot = Vec2(0.5f, 0.5f),
                .anchoredPosition = Vec2(0,0),
                .sizeDelta = size,
                .parent = btn
                });

            coord.AddComponent<Uma_UI::Text>(txt, 
                {
                .text = label,
                .fontName = "default",
                .fontSize = 1.f,
                .colour = Uma_UI::Colour::Black(),
                .alignment = Uma_UI::TextAlignment::Center,
                .visible = true
                });

            return btn;
        }
    };
}