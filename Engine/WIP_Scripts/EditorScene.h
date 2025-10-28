//#pragma once
//#include "SceneType.h"
//
//#include <vector>
//#include <random>
//#include <iostream>
//#include <iomanip>
//
//#include <GLFW/glfw3.h>
//
//
//namespace Uma_Engine
//{
//    class EditorScene : public Scene
//	  {
//    private:
//        SystemManager* pSystemManager;
//
//    public:
//        EditorScene(SystemManager* sm) : pSystemManager(sm) {}
//
//		    void OnLoad() override
//		    {
//            m_FilePath = "test_collider.json";
//
//            m_HybridInputSystem = pSystemManager->GetSystem<HybridInputSystem>();
//            m_Graphics = pSystemManager->GetSystem<Graphics>();
//            m_ResourcesManager = pSystemManager->GetSystem<ResourcesManager>();
//            m_EventSystem = pSystemManager->GetSystem<EventSystem>();
//            m_Sound = pSystemManager->GetSystem<Sound>();
//
//            // event system stuffs
//            // subscribe to events
//            m_EventSystem->Subscribe<Uma_Engine::QueryActiveEntitiesEvent>([this](const Uma_Engine::QueryActiveEntitiesEvent& e) { e.mActiveEntityCnt = m_Coordinator.GetEntityCount(); });
//           
//            m_EventSystem->Subscribe<Uma_Engine::SaveSceneRequestEvent>([this](const Uma_Engine::SaveSceneRequestEvent& e) { (void)e; gGameSerializer.save(Uma_FilePath::SCENES_DIR + m_FilePath); });
//            m_EventSystem->Subscribe<Uma_Engine::LoadSceneRequestEvent>([this](const Uma_Engine::LoadSceneRequestEvent& e) { (void)e; m_Coordinator.DestroyAllEntities(); gGameSerializer.load(Uma_FilePath::SCENES_DIR + m_FilePath); });
//            m_EventSystem->Subscribe<Uma_Engine::ClearSceneRequestEvent>([this](const Uma_Engine::ClearSceneRequestEvent& e) { (void)e; ResetAll(); });
//            m_EventSystem->Subscribe<Uma_Engine::StressTestRequestEvent>([this](const Uma_Engine::StressTestRequestEvent& e) { (void)e; StressTest(); });
//            m_EventSystem->Subscribe<Uma_Engine::ShowEntityInVPRequestEvent>([this](const Uma_Engine::ShowEntityInVPRequestEvent& e) { (void)e; SpawnDefaultEntities(); });
//
//            m_EventSystem->Subscribe<Uma_Engine::ChangeEnemyRotRequestEvent>([this](const Uma_Engine::ChangeEnemyRotRequestEvent& e) { ChangeAllEnemyRot(e.rot); });
//            m_EventSystem->Subscribe<Uma_Engine::ChangeEnemyXposRequestEvent>([this](const Uma_Engine::ChangeEnemyXposRequestEvent& e) { ChangeAllEnemyXPos(e.xpos); });
//            m_EventSystem->Subscribe<Uma_Engine::ChangeEnemyScaleRequestEvent>([this](const Uma_Engine::ChangeEnemyScaleRequestEvent& e) { ChangeAllEnemyScale(e.scale); });
//            m_EventSystem->Subscribe<Uma_Engine::ShowBBoxRequestEvent>([this](const Uma_Engine::ShowBBoxRequestEvent& e) {  ShowBBox(e.show); });
//
//            m_EventSystem->Subscribe<Uma_Engine::CloneEntityRequestEvent>([this](const Uma_Engine::CloneEntityRequestEvent& e)
//                { 
//                    (void)e;
//                    DuplicateOrCreateEntity();
//                });
//
//            m_EventSystem->Subscribe<Uma_Engine::LoadPrefabRequestEvent>([this](const Uma_Engine::LoadPrefabRequestEvent& e)
//                {
//                    (void)e;
//                    LoadPrefab();
//                });
//
//            m_EventSystem->Subscribe<Uma_Engine::DestroyEntityRequestEvent>([this](const Uma_Engine::DestroyEntityRequestEvent& e)
//                { 
//                    (void)e;
//                    DestroyRandomEntity();
//                });
//
//
//            // Ecs stuff
//            using namespace Uma_ECS;
//
//            m_Coordinator.Init(m_EventSystem);
//
//            // register components
//            m_Coordinator.RegisterComponent<Transform>();
//            m_Coordinator.RegisterComponent<RigidBody>();
//            m_Coordinator.RegisterComponent<Collider>();
//            m_Coordinator.RegisterComponent<Sprite>();
//            m_Coordinator.RegisterComponent<Camera>();
//            m_Coordinator.RegisterComponent<Player>();
//            m_Coordinator.RegisterComponent<Enemy>();
//
//            // Player controller
//            m_playerController = m_Coordinator.RegisterSystem<PlayerControllerSystem>();
//            {
//                Signature sign;
//                sign.set(m_Coordinator.GetComponentType<RigidBody>());
//                sign.set(m_Coordinator.GetComponentType<Transform>());
//                sign.set(m_Coordinator.GetComponentType<Player>());
//                m_Coordinator.SetSystemSignature<PlayerControllerSystem>(sign);
//            }
//            m_playerController->Init(m_EventSystem, m_HybridInputSystem, &m_Coordinator);
//
//            // Physics System
//            m_physicsSystem = m_Coordinator.RegisterSystem<PhysicsSystem>();
//            {
//                Signature sign;
//                sign.set(m_Coordinator.GetComponentType<RigidBody>());
//                sign.set(m_Coordinator.GetComponentType<Transform>());
//                m_Coordinator.SetSystemSignature<PhysicsSystem>(sign);
//            }
//            m_physicsSystem->Init(&m_Coordinator);
//
//            // Collision System
//            m_collisionSystem = m_Coordinator.RegisterSystem<CollisionSystem>();
//            {
//                Signature sign;
//                sign.set(m_Coordinator.GetComponentType<RigidBody>());
//                sign.set(m_Coordinator.GetComponentType<Transform>());
//                sign.set(m_Coordinator.GetComponentType<Collider>());
//                m_Coordinator.SetSystemSignature<CollisionSystem>(sign);
//            }
//            m_collisionSystem->Init(&m_Coordinator);
//
//            // Rendering System
//            m_renderingSystem = m_Coordinator.RegisterSystem<RenderingSystem>();
//            {
//                Signature sign;
//                sign.set(m_Coordinator.GetComponentType<Sprite>());
//                sign.set(m_Coordinator.GetComponentType<Transform>());
//                m_Coordinator.SetSystemSignature<RenderingSystem>(sign);
//            }
//            m_renderingSystem->Init(m_Graphics, m_ResourcesManager, &m_Coordinator);
//
//            m_cameraSystem = m_Coordinator.RegisterSystem<CameraSystem>();
//            {
//                Signature sign;
//                sign.set(m_Coordinator.GetComponentType<Camera>());
//                sign.set(m_Coordinator.GetComponentType<Transform>());
//                m_Coordinator.SetSystemSignature<CameraSystem>(sign);
//            }
//            m_cameraSystem->Init(&m_Coordinator);
//
//            // Init the game serializer
//            gGameSerializer.Register(m_ResourcesManager);
//            gGameSerializer.Register(&m_Coordinator);
//
//
//            //deserialize and spawn all the entities
//            //m_Coordinator.DeserializeAllEntities("Assets/Scenes/data.json");
//            gGameSerializer.load(Uma_FilePath::SCENES_DIR + m_FilePath);
//
//		    }
//		    void OnUnload() override
//		    {
//			      std::cout << "Test Scene 1: UNLOADED" << std::endl;
//
//            // resources unload
//                  m_ResourcesManager->UnloadAllTextures();
//                  m_ResourcesManager->UnloadAllSound();
//		    }
//		    void Update(float dt) override
//		    {
//            static bool firstFrame = true;
//            static float smoothedDt = 0.0f;
//            if (firstFrame) {
//                smoothedDt = dt; // seed filter with a realistic value
//                firstFrame = false;
//            }
//            else {
//                smoothedDt = 0.9f * smoothedDt + 0.1f * dt;
//            }
//
//            m_playerController->Update(dt);
//
//            m_physicsSystem->Update(smoothedDt);
//
//            m_collisionSystem->Update(dt);
//
//            m_cameraSystem->Update(dt);
//
//            // save to file
//            if (m_HybridInputSystem->KeyPressed(GLFW_KEY_1))
//            {
//                std::string filepath = Uma_FilePath::SCENES_DIR + m_FilePath;
//
//                gGameSerializer.save(filepath);
//            }
//
//            // load from file
//            if (m_HybridInputSystem->KeyPressed(GLFW_KEY_2))
//            {
//                m_Coordinator.DestroyAllEntities();
//
//                std::string filepath = Uma_FilePath::SCENES_DIR + m_FilePath;
//                
//                gGameSerializer.load(filepath);
//            }
//
//            // reset
//            if (m_HybridInputSystem->KeyPressed(GLFW_KEY_3))
//            {
//                ResetAll();
//            }
//
//            // Spawn Default
//            if (HybridInputSystem::KeyPressed(GLFW_KEY_4))
//            {
//                m_Coordinator.DestroyAllEntities();
//                SpawnDefaultEntities();
//            }
//
//            if (HybridInputSystem::KeyPressed(GLFW_KEY_P))
//            {
//                m_Sound->playSound(m_ResourcesManager->GetSound("explosion"));
//            }
//
//            if (HybridInputSystem::KeyPressed(GLFW_KEY_O))
//            {
//                m_Sound->playSound(m_ResourcesManager->GetSound("cave"));
//            }
//
//            m_Graphics->ClearBackground(0.2f, 0.3f, 0.3f);
//            //m_Graphics->DrawBackground(m_ResourcesManager->GetTexture("background")->tex_id);
//            m_renderingSystem->Update(dt);
//		    }
//		    void Render() override
//		    {
//
//		    }
//
//        void ResetAll()
//        {
//            m_Coordinator.DestroyAllEntities();
//
//            using namespace Uma_ECS;
//            
//            // create player
//            m_player = m_Coordinator.CreateEntity();
//            {
//                m_Coordinator.AddComponent(
//                    m_player,
//                    Transform
//                    {
//                        .position = Vec2(0.f, 0.f),
//                        .rotation = Vec2(0.f, 0.f),
//                        .scale = Vec2(1,1),
//                    });
//
//                m_Coordinator.AddComponent(
//                    m_player,
//                    RigidBody{
//                      .velocity = Vec2(0.0f, 0.0f),
//                      .acceleration = Vec2(0.0f, 0.0f),
//                      .accel_strength = 500,
//                      .fric_coeff = 5
//                    });
//
//                m_Coordinator.AddComponent(
//                    m_player,
//                    Player{
//                        .mSpeed = 1.f
//                    });
//
//                std::string texName = "player";
//                m_Coordinator.AddComponent(
//                    m_player,
//                    Sprite{
//                      .textureName = texName,
//                      .flipX = false,
//                      .flipY = false,
//                      .UseNativeSize = true,
//                      .texture = m_ResourcesManager->GetTexture(texName),
//                    });
//
//                // Create collider with two shapes
//                Collider playerCollider;
//
//                playerCollider.shapes[0] = ColliderShape{
//                        .purpose = ColliderPurpose::Physics,
//                        .layer = CL_PLAYER,
//                        .colliderMask = CL_ENEMY | CL_PROJECTILE,
//                        .isActive = true,
//                        .autoFitToSprite = true
//                };
//
//                playerCollider.shapes.push_back(ColliderShape{
//                    .size = Vec2(7.0f, 0.5f),
//                    .offset = Vec2(0, -3.f),
//                    .purpose = ColliderPurpose::Physics,
//                    .layer = CL_PLAYER,
//                    .colliderMask = CL_WALL,
//                    .isActive = true,
//                    .autoFitToSprite = false
//                    });
//
//                playerCollider.bounds.resize(playerCollider.shapes.size());
//                m_Coordinator.AddComponent(m_player, playerCollider);
//            }
//
//            // create camera
//            m_cam = m_Coordinator.CreateEntity();
//            {
//                m_Coordinator.AddComponent(
//                    m_cam,
//                    Transform
//                    {
//                        .position = Vec2(400.0f, 300.0f),
//                        .rotation = Vec2(0,0),
//                        .scale = Vec2(1,1),
//                    });
//
//                m_Coordinator.AddComponent(
//                    m_player,
//                    Camera
//                    {
//                        .mZoom = 1.f,
//                        .followPlayer = true
//                    });
//            }
//
//            //std::string log;
//            //std::stringstream ss(log);
//            //ss << "Created Entity : " << 10000;
//            //Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo, ss.str());
//        }
//
//        void SpawnDefaultEntities()
//        {
//            m_Coordinator.DestroyAllEntities();
//
//            using namespace Uma_ECS;
//
//            Entity kappa;
//            {
//                kappa = m_Coordinator.CreateEntity();
//
//                m_Coordinator.AddComponent(
//                    kappa,
//                    RigidBody{
//                      .velocity = Vec2(0.0f, 0.0f),
//                      .acceleration = Vec2(0.0f, 0.0f),
//                      .accel_strength = 200,
//                      .fric_coeff = 100
//                    });
//
//                m_Coordinator.AddComponent(
//                    kappa,
//                    Transform{
//                      .position = Vec2(30, 35),
//                      .rotation = Vec2(0, 0),
//                      .scale = Vec2(3.f, 3.f)
//                    });
//
//                std::string texName = "kappa_statue";
//                m_Coordinator.AddComponent(
//                    kappa,
//                    Sprite{
//                      .textureName = texName,
//                      .renderLayer = RL_ENV,
//                      .flipX = false,
//                      .flipY = false,
//                      .UseNativeSize = true,
//                      .texture = m_ResourcesManager->GetTexture(texName),
//                    });
//            }
//
//            Entity wall;
//            {
//                wall = m_Coordinator.CreateEntity();
//
//                m_Coordinator.AddComponent(
//                    wall,
//                    RigidBody{
//                      .velocity = Vec2(0.0f, 0.0f),
//                      .acceleration = Vec2(0.0f, 0.0f),
//                      .accel_strength = 200,
//                      .fric_coeff = 100
//                    });
//
//                m_Coordinator.AddComponent(
//                    wall,
//                    Transform{
//                      .position = Vec2(20, 0),
//                      .rotation = Vec2(0, 0),
//                      .scale = Vec2(1.f, 1.f)
//                    });
//
//                std::string texName = "wall_top";
//                m_Coordinator.AddComponent(
//                    wall,
//                    Sprite{
//                      .textureName = texName,
//                      .renderLayer = RL_WALL,
//                      .flipX = false,
//                      .flipY = false,
//                      .UseNativeSize = true,
//                      .texture = m_ResourcesManager->GetTexture(texName),
//                    });
//
//                // Create collider with two shapes
//                Collider wallCollider;
//
//                // Primary shape: Body hitbox (for taking damage)
//                wallCollider.shapes[0] = ColliderShape{
//                    .purpose = ColliderPurpose::Environment,
//                    .layer = CL_WALL,
//                    .colliderMask = CL_PLAYER | CL_ENEMY,  // Blocks entities,
//                    .isActive = true,
//                    .autoFitToSprite = true  // Will be 128x128 (64*2 scale)
//                };
//
//                wallCollider.bounds.resize(wallCollider.shapes.size());
//                m_Coordinator.AddComponent(wall, wallCollider);
//
//                for (size_t i = 0; i < 5; i++)
//                {
//                    Entity tmp = m_Coordinator.DuplicateEntity(wall);
//
//                    Transform& tf = m_Coordinator.GetComponent<Transform>(tmp);
//
//                    tf.position = Vec2(20 + (i * 5), 0);
//
//                    Sprite& sr = m_Coordinator.GetComponent<Sprite>(tmp);
//
//                    // set texture randomly
//                    sr.textureName = "wall_btm";
//                    sr.texture = m_ResourcesManager->GetTexture(sr.textureName);
//                }
//
//                for (size_t i = 0; i < 6; i++)
//                {
//                    Entity tmp = m_Coordinator.DuplicateEntity(wall);
//
//                    Transform& tf = m_Coordinator.GetComponent<Transform>(tmp);
//
//                    tf.position = Vec2(15 + (6 * 5), 5 + (i * 5));
//
//                    Sprite& sr = m_Coordinator.GetComponent<Sprite>(tmp);
//
//                    // set texture randomly
//                    sr.textureName = "wall_right";
//                    sr.texture = m_ResourcesManager->GetTexture(sr.textureName);
//                }
//
//                for (size_t i = 0; i < 5; i++)
//                {
//                    Entity tmp = m_Coordinator.DuplicateEntity(wall);
//
//                    Transform& tf = m_Coordinator.GetComponent<Transform>(tmp);
//
//                    tf.position = Vec2(20 + (i * 5), 15 + (4 * 5));
//
//                    Sprite& sr = m_Coordinator.GetComponent<Sprite>(tmp);
//
//                    // set texture randomly
//                    sr.textureName = "wall_top";
//                    sr.texture = m_ResourcesManager->GetTexture(sr.textureName);
//                }
//            }
//
//            Entity floor;
//            {
//                floor = m_Coordinator.CreateEntity();
//
//                m_Coordinator.AddComponent(
//                    floor,
//                    RigidBody{
//                      .velocity = Vec2(0.0f, 0.0f),
//                      .acceleration = Vec2(0.0f, 0.0f),
//                      .accel_strength = 200,
//                      .fric_coeff = 100
//                    });
//
//                m_Coordinator.AddComponent(
//                    floor,
//                    Transform{
//                      .position = Vec2(20, 7.5),
//                      .rotation = Vec2(0, 0),
//                      .scale = Vec2(2.f, 2.f)
//                    });
//
//                std::string texName = "floor_tatami";
//                m_Coordinator.AddComponent(
//                    floor,
//                    Sprite{
//                      .textureName = texName,
//                      .renderLayer = RL_WALL,
//                      .flipX = false,
//                      .flipY = false,
//                      .UseNativeSize = true,
//                      .texture = m_ResourcesManager->GetTexture(texName),
//                    });
//
//                for (size_t i = 0; i < 5; i++)
//                {
//                    for (size_t j = 0; j < 3; j++)
//                    {
//                        Entity tmp = m_Coordinator.DuplicateEntity(floor);
//
//                        Transform& tf = m_Coordinator.GetComponent<Transform>(tmp);
//
//                        tf.position = Vec2(20 + (i * 5), 7.5 + (j * 10));
//                    }
//                }
//            }
//
//            // create entities
//            {
//                //std::default_random_engine generator;
//                //std::uniform_real_distribution<float> randPositionX(-1920.f * 0.1f, 1920.f * 0.1f);
//                //std::uniform_real_distribution<float> randPositionY(-1080.f * 0.1f, 1080.f * 0.1f);
//                ////std::uniform_real_distribution<float> randRotation(10.0f, 15.0f);
//                //std::uniform_real_distribution<float> randScale(1.f, 1.f);
//
//                Entity enemy;
//                {
//                    enemy = m_Coordinator.CreateEntity();
//
//                    m_Coordinator.AddComponent(
//                        enemy,
//                        Enemy{
//                            .mSpeed = 1.f
//                        });
//
//                    m_Coordinator.AddComponent(
//                        enemy,
//                        RigidBody{
//                          .velocity = Vec2(0.0f, 0.0f),
//                          .acceleration = Vec2(0.0f, 0.0f),
//                          .accel_strength = 200,
//                          .fric_coeff = 100
//                        });
//
//                    m_Coordinator.AddComponent(
//                        enemy,
//                        Transform{
//                          .position = Vec2(-10, 0),
//                          .rotation = Vec2(0, 0),
//                          .scale = Vec2(2.f, 2.f)
//                        });
//
//                    std::string texName = "pink_enemy";
//                    m_Coordinator.AddComponent(
//                        enemy,
//                        Sprite{
//                          .textureName = texName,
//                          .renderLayer = RL_ENEMY,
//                          .flipX = false,
//                          .flipY = false,
//                          .UseNativeSize = true,
//                          .texture = m_ResourcesManager->GetTexture(texName),
//                        });
//
//                    // Create collider with two shapes
//                    Collider enemyCollider;
//
//                    enemyCollider.shapes[0] = ColliderShape{
//                        .size = Vec2(3.f, 3.f),
//                        .offset = Vec2(0.f, 1.f),
//                        .purpose = ColliderPurpose::Physics,
//                        .layer = CL_ENEMY,
//                        .colliderMask = CL_PLAYER | CL_PROJECTILE,
//                        .isActive = true,
//                        .autoFitToSprite = false
//                    };
//                
//                    enemyCollider.shapes.push_back(ColliderShape{
//                        .size = Vec2(2.f, 0.5f),
//                        .offset = Vec2(0.f, -2.f),
//                        .purpose = ColliderPurpose::Physics,
//                        .layer = CL_ENEMY,
//                        .colliderMask = CL_WALL,
//                        .isActive = true,
//                        .autoFitToSprite = false
//                    });
//
//                    enemyCollider.bounds.resize(enemyCollider.shapes.size());
//                    m_Coordinator.AddComponent(enemy, enemyCollider);
//                }
//
//                // using 1 enemy to duplicate 2500 times and rand its transform
//                /*for (size_t i = 0; i < 2500 - 3; i++)
//                {
//                    Entity tmp = m_Coordinator.DuplicateEntity(enemy);
//
//                    Transform& tf = m_Coordinator.GetComponent<Transform>(tmp);
//
//                    tf.position = Vec2(randPositionX(generator), randPositionY(generator));
//                    tf.rotation = Vec2(0, 0);
//                    tf.scale = Vec2(randScale(generator), randScale(generator));
//
//                    Sprite& sr = m_Coordinator.GetComponent<Sprite>(tmp);
//
//                    sr.textureName = (i > 1250) ? "pink_enemy" : "enemy";
//                    sr.texture = m_ResourcesManager->GetTexture(sr.textureName);
//                }*/
//            }
//
//            // create player
//            m_player = m_Coordinator.CreateEntity();
//            {
//                m_Coordinator.AddComponent(
//                    m_player,
//                    Transform
//                    {
//                        .position = Vec2(0.f, 0.f),
//                        .rotation = Vec2(0.f, 0.f),
//                        .scale = Vec2(1,1),
//                    });
//
//                m_Coordinator.AddComponent(
//                    m_player,
//                    RigidBody{
//                      .velocity = Vec2(0.0f, 0.0f),
//                      .acceleration = Vec2(0.0f, 0.0f),
//                      .accel_strength = 500,
//                      .fric_coeff = 5
//                    });
//
//                m_Coordinator.AddComponent(
//                    m_player,
//                    Player{
//                        .mSpeed = 1.f
//                    });
//
//                std::string texName = "player";
//                m_Coordinator.AddComponent(
//                    m_player,
//                    Sprite{
//                      .textureName = texName,
//                      .renderLayer = RL_PLAYER,
//                      .flipX = false,
//                      .flipY = false,
//                      .UseNativeSize = true,
//                      .texture = m_ResourcesManager->GetTexture(texName),
//                    });
//
//                // Create collider with two shapes
//                Collider playerCollider;
//
//                playerCollider.shapes[0] = ColliderShape{
//                        .purpose = ColliderPurpose::Physics,
//                        .layer = CL_PLAYER,
//                        .colliderMask = CL_ENEMY | CL_PROJECTILE,
//                        .isActive = true,
//                        .autoFitToSprite = true
//                    };
//
//                playerCollider.shapes.push_back(ColliderShape{
//                    .size = Vec2(7.0f, 0.5f),
//                    .offset = Vec2(0, -2.75f),
//                    .purpose = ColliderPurpose::Physics,
//                    .layer = CL_PLAYER,
//                    .colliderMask = CL_WALL,
//                    .isActive = true,
//                    .autoFitToSprite = false
//                    });
//
//                playerCollider.bounds.resize(playerCollider.shapes.size());
//                m_Coordinator.AddComponent(m_player, playerCollider);
//            }
//
//            // create camera
//            m_cam = m_Coordinator.CreateEntity();
//            {
//                m_Coordinator.AddComponent(
//                    m_cam,
//                    Transform
//                    {
//                        .position = Vec2(400.0f, 300.0f),
//                        .rotation = Vec2(0,0),
//                        .scale = Vec2(1,1),
//                    });
//
//                m_Coordinator.AddComponent(
//                    m_player,
//                    Camera
//                    {
//                        .mZoom = 1.f,
//                        .followPlayer = true
//                    });
//            }
//        }
//
//        void DuplicateOrCreateEntity()
//        {
//            using namespace Uma_ECS;
//
//            // find an active entity
//            auto& eArray = m_Coordinator.GetComponentArray<Uma_ECS::Enemy>();
//
//            std::default_random_engine generator(std::random_device{}());
//            std::uniform_real_distribution<float> randPositionX(-400, 400);
//            std::uniform_real_distribution<float> randPositionY(-400, 400);
//            //std::uniform_real_distribution<float> randRotation(10.0f, 15.0f);
//            std::uniform_real_distribution<float> randScale(1, 1);
//
//            if (eArray.Size() == 0)
//            {
//                // Create Entity and save it to the prefab file
//
//                Entity enemy;
//                {
//                    enemy = m_Coordinator.CreateEntity();
//
//                    m_Coordinator.AddComponent(
//                        enemy,
//                        Enemy{
//                            .mSpeed = 1.f
//                        });
//
//                    m_Coordinator.AddComponent(
//                        enemy,
//                        RigidBody{
//                          .velocity = Vec2(0.0f, 0.0f),
//                          .acceleration = Vec2(0.0f, 0.0f),
//                          .accel_strength = 200,
//                          .fric_coeff = 100
//                        });
//
//                    m_Coordinator.AddComponent(
//                        enemy,
//                        Transform{
//                          .position = Vec2(-10, 0),
//                          .rotation = Vec2(0, 0),
//                          .scale = Vec2(1.f, 1.f)
//                        });
//
//                    std::string texName = "pink_enemy";
//                    m_Coordinator.AddComponent(
//                        enemy,
//                        Sprite{
//                          .textureName = texName,
//                          .flipX = false,
//                          .flipY = false,
//                          .UseNativeSize = true,
//                          .texture = m_ResourcesManager->GetTexture(texName),
//                        });
//
//                    // Create collider with two shapes
//                    Collider enemyCollider;
//
//                    enemyCollider.shapes[0] = ColliderShape{
//                        .size = Vec2(3.f, 3.f),
//                        .offset = Vec2(0.f, 1.f),
//                        .purpose = ColliderPurpose::Physics,
//                        .layer = CL_ENEMY,
//                        .colliderMask = CL_PLAYER | CL_PROJECTILE,
//                        .isActive = true,
//                        .autoFitToSprite = false
//                        };
//
//                    enemyCollider.shapes.push_back(ColliderShape{
//                        .size = Vec2(2.f, 0.5f),
//                        .offset = Vec2(0.f, -2.f),
//                        .purpose = ColliderPurpose::Environment,
//                        .layer = CL_WALL,
//                        .colliderMask = CL_WALL,
//                        .isActive = true,
//                        .autoFitToSprite = false
//                        });
//
//                    enemyCollider.bounds.resize(enemyCollider.shapes.size());
//                    m_Coordinator.AddComponent(enemy, enemyCollider);
//                }
//
//                gGameSerializer.savePrefab(enemy, Uma_FilePath::PREFAB_DIR + "enemy.json");
//
//                Transform& tf = m_Coordinator.GetComponent<Transform>(enemy);
//
//                tf.position = Vec2(randPositionX(generator), randPositionY(generator));
//            }
//            else
//            {
//                // duplicate existing entity
//
//                Entity tmp = m_Coordinator.DuplicateEntity(eArray.GetEntity(0));
//
//                Transform& tf = m_Coordinator.GetComponent<Transform>(tmp);
//
//                tf.position = Vec2(randPositionX(generator), randPositionY(generator));
//                tf.rotation = Vec2(0, 0);
//                tf.scale = Vec2(randScale(generator), randScale(generator));
//
//                Sprite& sr = m_Coordinator.GetComponent<Sprite>(tmp);
//
//                // set texture randomly
//                sr.textureName = (randPositionX(generator) > 0.f) ? "pink_enemy" : "enemy";
//                sr.texture = m_ResourcesManager->GetTexture(sr.textureName);
//            }
//        }
//
//        void DestroyRandomEntity()
//        {
//            auto& eArray = m_Coordinator.GetComponentArray<Uma_ECS::Enemy>();
//
//            std::default_random_engine generator(std::random_device{}());
//            std::uniform_real_distribution<float> randPositionX(-400, 400);
//            std::uniform_real_distribution<float> randPositionY(-400, 400);
//            std::uniform_real_distribution<float> randScale(10.0f, 15.0f);
//
//            if (eArray.Size() != 0)
//            {
//                m_Coordinator.DestroyEntity(eArray.GetEntity(0));
//            }
//        }
//
//        void StressTest()
//        {
//            m_Coordinator.DestroyAllEntities();
//
//            using namespace Uma_ECS;
//
//            // create entities
//            {
//                std::default_random_engine generator;
//                std::uniform_real_distribution<float> randPositionX(-1920.f, 1920.f);
//                std::uniform_real_distribution<float> randPositionY(-1080.f, 1080.f);
//                std::uniform_real_distribution<float> randRotation(0.0f, 0.0f);
//                std::uniform_real_distribution<float> randScale(1, 1);
//
//                Entity enemy;
//                {
//                    enemy = m_Coordinator.CreateEntity();
//
//                    m_Coordinator.AddComponent(
//                        enemy,
//                        Enemy{
//                            .mSpeed = 1.f
//                        });
//
//                    m_Coordinator.AddComponent(
//                        enemy,
//                        RigidBody{
//                          .velocity = Vec2(0.0f, 0.0f),
//                          .acceleration = Vec2(0.0f, 0.0f),
//                          .accel_strength = 200,
//                          .fric_coeff = 100
//                        });
//
//                    m_Coordinator.AddComponent(
//                        enemy,
//                        Transform{
//                          .position = Vec2(-10, 0),
//                          .rotation = Vec2(0, 0),
//                          .scale = Vec2(1.f, 1.f)
//                        });
//
//                    std::string texName = "pink_enemy";
//                    m_Coordinator.AddComponent(
//                        enemy,
//                        Sprite{
//                          .textureName = texName,
//                          .flipX = false,
//                          .flipY = false,
//                          .UseNativeSize = true,
//                          .texture = m_ResourcesManager->GetTexture(texName),
//                        });
//
//                    // Create collider with two shapes
//                    Collider enemyCollider;
//
//                    enemyCollider.shapes[0] = ColliderShape{
//                        .size = Vec2(3.f, 3.f),
//                        .offset = Vec2(0.f, 1.f),
//                        .purpose = ColliderPurpose::Physics,
//                        .layer = CL_ENEMY,
//                        .colliderMask = CL_PLAYER | CL_PROJECTILE,
//                        .isActive = true,
//                        .autoFitToSprite = false
//                    };
//
//                    enemyCollider.shapes.push_back(ColliderShape{
//                        .size = Vec2(2.f, 0.5f),
//                        .offset = Vec2(0.f, -2.f),
//                        .purpose = ColliderPurpose::Environment,
//                        .layer = CL_WALL,
//                        .colliderMask = CL_WALL,
//                        .isActive = true,
//                        .autoFitToSprite = false
//                        });
//
//                    enemyCollider.bounds.resize(enemyCollider.shapes.size());
//                    m_Coordinator.AddComponent(enemy, enemyCollider);
//                }
//
//                // using 1 enemy to duplicate 2500 times and rand its transform
//                for (size_t i = 0; i < 10000 - 3; i++)
//                {
//                    Entity tmp = m_Coordinator.DuplicateEntity(enemy);
//
//                    Transform& tf = m_Coordinator.GetComponent<Transform>(tmp);
//
//                    tf.position = Vec2(randPositionX(generator), randPositionY(generator));
//
//                    Sprite& sr = m_Coordinator.GetComponent<Sprite>(tmp);
//
//                    //sr.textureName = (i > 1250) ? "pink_enemy" : "enemy";
//                    sr.texture = m_ResourcesManager->GetTexture(sr.textureName);
//                }
//            }
//
//            // create player
//            m_player = m_Coordinator.CreateEntity();
//            {
//                m_Coordinator.AddComponent(
//                    m_player,
//                    Transform
//                    {
//                        .position = Vec2(0.f, 0.f),
//                        .rotation = Vec2(0.f, 0.f),
//                        .scale = Vec2(1,1),
//                    });
//
//                m_Coordinator.AddComponent(
//                    m_player,
//                    RigidBody{
//                      .velocity = Vec2(0.0f, 0.0f),
//                      .acceleration = Vec2(0.0f, 0.0f),
//                      .accel_strength = 500,
//                      .fric_coeff = 5
//                    });
//
//                m_Coordinator.AddComponent(
//                    m_player,
//                    Player{
//                        .mSpeed = 1.f
//                    });
//
//                std::string texName = "player";
//                m_Coordinator.AddComponent(
//                    m_player,
//                    Sprite{
//                      .textureName = texName,
//                      .flipX = false,
//                      .flipY = false,
//                      .UseNativeSize = true,
//                      .texture = m_ResourcesManager->GetTexture(texName),
//                    });
//
//                // Create collider with two shapes
//                Collider playerCollider;
//
//                playerCollider.shapes[0] = ColliderShape{
//                        .purpose = ColliderPurpose::Physics,
//                        .layer = CL_PLAYER,
//                        .colliderMask = CL_ENEMY | CL_PROJECTILE,
//                        .isActive = true,
//                        .autoFitToSprite = true
//                    };
//
//                playerCollider.shapes.push_back(ColliderShape{
//                    .size = Vec2(7.0f, 0.5f),
//                    .offset = Vec2(0, -3.f),
//                    .purpose = ColliderPurpose::Environment,
//                    .layer = CL_WALL,
//                    .colliderMask = CL_WALL,
//                    .isActive = true,
//                    .autoFitToSprite = false
//                    });
//
//                playerCollider.bounds.resize(playerCollider.shapes.size());
//                m_Coordinator.AddComponent(m_player, playerCollider);
//            }
//
//            // create camera
//            m_cam = m_Coordinator.CreateEntity();
//            {
//                m_Coordinator.AddComponent(
//                    m_cam,
//                    Transform
//                    {
//                        .position = Vec2(400.0f, 300.0f),
//                        .rotation = Vec2(0,0),
//                        .scale = Vec2(1,1),
//                    });
//
//                m_Coordinator.AddComponent(
//                    m_player,
//                    Camera
//                    {
//                        .mZoom = 1.f,
//                        .followPlayer = true
//                    });
//            }
//        }
//
//        void ChangeAllEnemyRot(float rot)
//        {
//            using namespace Uma_ECS;
//
//            //std::default_random_engine generator(std::random_device{}());
//            //std::uniform_real_distribution<float> randScale(10.0f, 15.0f);
//
//            auto& eArray = m_Coordinator.GetComponentArray<Uma_ECS::Enemy>();
//            auto& tfArray = m_Coordinator.GetComponentArray<Uma_ECS::Transform>();
//
//            for (size_t i = 0; i < eArray.Size(); i++)
//            {
//                auto& tf = tfArray.GetData(eArray.GetEntity(i));
//
//                //tf.scale = //Vec2{randScale(generator), randScale(generator)} * scale;
//
//                tf.rotation.y = rot;
//            }
//        }
//
//        void ChangeAllEnemyXPos(float xPos)
//        {
//            using namespace Uma_ECS;
//
//            std::default_random_engine generator(std::random_device{}());
//            std::uniform_real_distribution<float> randfloat(1.0f, 5.0f);
//
//            auto& eArray = m_Coordinator.GetComponentArray<Uma_ECS::Enemy>();
//            auto& rbArray = m_Coordinator.GetComponentArray<Uma_ECS::RigidBody>();
//
//            for (size_t i = 0; i < eArray.Size(); i++)
//            {
//                auto& rb = rbArray.GetData(eArray.GetEntity(i));
//
//                rb.acceleration = Vec2{ 5000.f * xPos, 0 };
//            }
//        }
//
//        void ChangeAllEnemyScale(float scale)
//        {
//            using namespace Uma_ECS;
//
//            std::default_random_engine generator(std::random_device{}());
//            std::uniform_real_distribution<float> randScale(10.0f, 15.0f);
//
//            auto& eArray = m_Coordinator.GetComponentArray<Uma_ECS::Enemy>();
//            auto& tfArray = m_Coordinator.GetComponentArray<Uma_ECS::Transform>();
//
//            for (size_t i = 0; i < eArray.Size(); i++)
//            {
//                auto& tf = tfArray.GetData(eArray.GetEntity(i));
//
//                tf.scale = Vec2{randScale(generator), randScale(generator)} * scale;
//            }
//        }
//
//        void ShowBBox(bool isShow)
//        {
//            using namespace Uma_ECS;
//
//            auto& tfArray = m_Coordinator.GetComponentArray<Uma_ECS::Transform>();
//            auto& cArray = m_Coordinator.GetComponentArray<Uma_ECS::Collider>();
//
//            for (size_t i = 0; i < tfArray.Size(); i++)
//            {
//                if (!cArray.Has(tfArray.GetEntity(i))) continue;
//                auto& c = cArray.GetData(tfArray.GetEntity(i));
//
//                c.showBBox = isShow;
//            }
//
//
//        }
//
//        void LoadPrefab()
//        {
//            gGameSerializer.loadPrefab(Uma_FilePath::PREFAB_DIR + "enemy.json");
//        }
//	  };
//}
