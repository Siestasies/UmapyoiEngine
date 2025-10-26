#pragma once
#include "SceneType.h"
#include "Core/GameSerializer.h"
#include <random>

namespace Uma_Engine
{
    class EditorSceneTest : public Scene
    {
    public:
        EditorSceneTest(const std::string& name, const std::string& filepath, SystemManager* sm)
            : Scene(name, filepath, sm)
        {
        }

        ~EditorSceneTest() override = default;

    protected:
        // Override virtual methods from base Scene class
        void OnLoad() override
        {
            // Subscribe to events specific to editor scene
            SubscribeToEvents();

            // You can spawn default entities here if you don't want to deserialize
            // Or leave it empty if you're loading from file
        }

        void OnUnload() override
        {
            // Unsubscribe from events
            // Clean up editor-specific resources
            // Base class will handle entity destruction and resource unloading
        }

        void OnUpdate(float dt) override
        {
            // ECS systems are already updated by base class
            // Add editor-specific update logic here

            //Handle input for saving/loading
            if (m_HybridInputSystem->KeyPressed(GLFW_KEY_1))
            {
                Serialize(); // Save to current filepath
            }

            if (m_HybridInputSystem->KeyPressed(GLFW_KEY_2))
            {
                m_Coordinator.DestroyAllEntities();
                Deserialize(); // Load from current filepath
            }

            if (m_HybridInputSystem->KeyPressed(GLFW_KEY_3))
            {
                ResetScene();
            }

            if (m_HybridInputSystem->KeyPressed(GLFW_KEY_4))
            {
                m_Coordinator.DestroyAllEntities();
                SpawnDefaultEntities();
            }

            if (m_HybridInputSystem->KeyPressed(GLFW_KEY_P))
            {
                m_Sound->playSound(m_ResourcesManager->GetSound("explosion"));
            }
        }

    private:
        void SubscribeToEvents()
        {
            m_EventSystem->Subscribe<Uma_Engine::QueryActiveEntitiesEvent>(
                [this](const Uma_Engine::QueryActiveEntitiesEvent& e) {
                    e.mActiveEntityCnt = m_Coordinator.GetEntityCount();
                }
            );

            m_EventSystem->Subscribe<Uma_Engine::SaveSceneRequestEvent>(
                [this](const Uma_Engine::SaveSceneRequestEvent& e) {
                    (void)e;
                    Serialize();
                }
            );

            m_EventSystem->Subscribe<Uma_Engine::LoadSceneRequestEvent>(
                [this](const Uma_Engine::LoadSceneRequestEvent& e) {
                    (void)e;
                    m_Coordinator.DestroyAllEntities();
                    Deserialize();
                }
            );

            m_EventSystem->Subscribe<Uma_Engine::ClearSceneRequestEvent>(
                [this](const Uma_Engine::ClearSceneRequestEvent& e) {
                    (void)e;
                    ResetScene();
                }
            );

            m_EventSystem->Subscribe<Uma_Engine::StressTestRequestEvent>(
                [this](const Uma_Engine::StressTestRequestEvent& e) {
                    (void)e;
                    StressTest();
                }
            );

            m_EventSystem->Subscribe<Uma_Engine::ShowEntityInVPRequestEvent>(
                [this](const Uma_Engine::ShowEntityInVPRequestEvent& e) {
                    (void)e;
                    SpawnDefaultEntities();
                }
            );

            m_EventSystem->Subscribe<Uma_Engine::ChangeEnemyRotRequestEvent>(
                [this](const Uma_Engine::ChangeEnemyRotRequestEvent& e) {
                    ChangeAllEnemyRot(e.rot);
                }
            );

            m_EventSystem->Subscribe<Uma_Engine::ShowBBoxRequestEvent>(
                [this](const Uma_Engine::ShowBBoxRequestEvent& e) {
                    ShowBBox(e.show);
                }
            );

            m_EventSystem->Subscribe<Uma_Engine::CloneEntityRequestEvent>(
                [this](const Uma_Engine::CloneEntityRequestEvent& e) {
                    (void)e;
                    DuplicateOrCreateEntity();
                }
            );

            m_EventSystem->Subscribe<Uma_Engine::DestroyEntityRequestEvent>(
                [this](const Uma_Engine::DestroyEntityRequestEvent& e) {
                    (void)e;
                    DestroyRandomEntity();
                }
            );
        }

        void ResetScene()
        {
            m_Coordinator.DestroyAllEntities();

            using namespace Uma_ECS;

            // Create player
            m_Player = m_Coordinator.CreateEntity();
            {
                m_Coordinator.AddComponent(m_Player, Transform{
                    .position = Vec2(0.f, 0.f),
                    .rotation = Vec2(0.f, 0.f),
                    .scale = Vec2(1, 1),
                    });

                m_Coordinator.AddComponent(m_Player, RigidBody{
                    .velocity = Vec2(0.0f, 0.0f),
                    .acceleration = Vec2(0.0f, 0.0f),
                    .accel_strength = 500,
                    .fric_coeff = 5
                    });

                m_Coordinator.AddComponent(m_Player, Player{ .mSpeed = 1.f });

                std::string texName = "player";
                m_Coordinator.AddComponent(m_Player, Sprite{
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
                m_Coordinator.AddComponent(m_Player, playerCollider);
            }

            // Create camera
            m_Cam = m_Coordinator.CreateEntity();
            {
                m_Coordinator.AddComponent(m_Cam, Transform{
                    .position = Vec2(400.0f, 300.0f),
                    .rotation = Vec2(0, 0),
                    .scale = Vec2(1, 1),
                    });

                m_Coordinator.AddComponent(m_Player, Camera{
                    .mZoom = 1.f,
                    .followPlayer = true
                    });
            }
        }

        void SpawnDefaultEntities()
        {
            // Use the base class implementation or override with your own
            //Scene::SpawnDefaultEntities();
        }

        void DuplicateOrCreateEntity()
        {
            using namespace Uma_ECS;

            // find an active entity
            auto& eArray = m_Coordinator.GetComponentArray<Uma_ECS::Enemy>();

            std::default_random_engine generator(std::random_device{}());
            std::uniform_real_distribution<float> randPositionX(-400, 400);
            std::uniform_real_distribution<float> randPositionY(-400, 400);
            //std::uniform_real_distribution<float> randRotation(10.0f, 15.0f);
            std::uniform_real_distribution<float> randScale(1, 1);

            if (eArray.Size() == 0)
            {
                // Create Entity and save it to the prefab file

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
                          .scale = Vec2(1.f, 1.f)
                        });

                    std::string texName = "pink_enemy";
                    m_Coordinator.AddComponent(
                        enemy,
                        Sprite{
                          .textureName = texName,
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
                        .purpose = ColliderPurpose::Environment,
                        .layer = CL_WALL,
                        .colliderMask = CL_WALL,
                        .isActive = true,
                        .autoFitToSprite = false
                        });

                    enemyCollider.bounds.resize(enemyCollider.shapes.size());
                    m_Coordinator.AddComponent(enemy, enemyCollider);
                }

                m_GameSerializer.savePrefab(enemy, Uma_FilePath::PREFAB_DIR + "enemy.json");

                Transform& tf = m_Coordinator.GetComponent<Transform>(enemy);

                tf.position = Vec2(randPositionX(generator), randPositionY(generator));
            }
            else
            {
                // duplicate existing entity

                Entity tmp = m_Coordinator.DuplicateEntity(eArray.GetEntity(0));

                Transform& tf = m_Coordinator.GetComponent<Transform>(tmp);

                tf.position = Vec2(randPositionX(generator), randPositionY(generator));
                tf.rotation = Vec2(0, 0);
                tf.scale = Vec2(randScale(generator), randScale(generator));

                Sprite& sr = m_Coordinator.GetComponent<Sprite>(tmp);

                // set texture randomly
                sr.textureName = (randPositionX(generator) > 0.f) ? "pink_enemy" : "enemy";
                sr.texture = m_ResourcesManager->GetTexture(sr.textureName);
            }
        }

        void DestroyRandomEntity()
        {
            auto& eArray = m_Coordinator.GetComponentArray<Uma_ECS::Enemy>();

            std::default_random_engine generator(std::random_device{}());
            std::uniform_real_distribution<float> randPositionX(-400, 400);
            std::uniform_real_distribution<float> randPositionY(-400, 400);
            std::uniform_real_distribution<float> randScale(10.0f, 15.0f);

            if (eArray.Size() != 0)
            {
                m_Coordinator.DestroyEntity(eArray.GetEntity(0));
            }
        }

        void StressTest()
        {
            m_Coordinator.DestroyAllEntities();

            using namespace Uma_ECS;

            // create entities
            {
                std::default_random_engine generator;
                std::uniform_real_distribution<float> randPositionX(-1920.f, 1920.f);
                std::uniform_real_distribution<float> randPositionY(-1080.f, 1080.f);
                std::uniform_real_distribution<float> randRotation(0.0f, 0.0f);
                std::uniform_real_distribution<float> randScale(1, 1);

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
                          .scale = Vec2(1.f, 1.f)
                        });

                    std::string texName = "pink_enemy";
                    m_Coordinator.AddComponent(
                        enemy,
                        Sprite{
                          .textureName = texName,
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
                        .purpose = ColliderPurpose::Environment,
                        .layer = CL_WALL,
                        .colliderMask = CL_WALL,
                        .isActive = true,
                        .autoFitToSprite = false
                        });

                    enemyCollider.bounds.resize(enemyCollider.shapes.size());
                    m_Coordinator.AddComponent(enemy, enemyCollider);
                }

                // using 1 enemy to duplicate 2500 times and rand its transform
                for (size_t i = 0; i < 10000 - 3; i++)
                {
                    Entity tmp = m_Coordinator.DuplicateEntity(enemy);

                    Transform& tf = m_Coordinator.GetComponent<Transform>(tmp);

                    tf.position = Vec2(randPositionX(generator), randPositionY(generator));

                    Sprite& sr = m_Coordinator.GetComponent<Sprite>(tmp);

                    //sr.textureName = (i > 1250) ? "pink_enemy" : "enemy";
                    sr.texture = m_ResourcesManager->GetTexture(sr.textureName);
                }
            }

            // create player
            m_Player = m_Coordinator.CreateEntity();
            {
                m_Coordinator.AddComponent(
                    m_Player,
                    Transform
                    {
                        .position = Vec2(0.f, 0.f),
                        .rotation = Vec2(0.f, 0.f),
                        .scale = Vec2(1,1),
                    });

                m_Coordinator.AddComponent(
                    m_Player,
                    RigidBody{
                      .velocity = Vec2(0.0f, 0.0f),
                      .acceleration = Vec2(0.0f, 0.0f),
                      .accel_strength = 500,
                      .fric_coeff = 5
                    });

                m_Coordinator.AddComponent(
                    m_Player,
                    Player{
                        .mSpeed = 1.f
                    });

                std::string texName = "player";
                m_Coordinator.AddComponent(
                    m_Player,
                    Sprite{
                      .textureName = texName,
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
                    .offset = Vec2(0, -3.f),
                    .purpose = ColliderPurpose::Environment,
                    .layer = CL_WALL,
                    .colliderMask = CL_WALL,
                    .isActive = true,
                    .autoFitToSprite = false
                    });

                playerCollider.bounds.resize(playerCollider.shapes.size());
                m_Coordinator.AddComponent(m_Player, playerCollider);
            }

            // create camera
            m_Cam = m_Coordinator.CreateEntity();
            {
                m_Coordinator.AddComponent(
                    m_Cam,
                    Transform
                    {
                        .position = Vec2(400.0f, 300.0f),
                        .rotation = Vec2(0,0),
                        .scale = Vec2(1,1),
                    });

                m_Coordinator.AddComponent(
                    m_Player,
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

            auto& eArray = m_Coordinator.GetComponentArray<Uma_ECS::Enemy>();
            auto& tfArray = m_Coordinator.GetComponentArray<Uma_ECS::Transform>();

            for (size_t i = 0; i < eArray.Size(); i++)
            {
                auto& tf = tfArray.GetData(eArray.GetEntity(i));

                //tf.scale = //Vec2{randScale(generator), randScale(generator)} * scale;

                tf.rotation.y = rot;
            }
        }

        void ChangeAllEnemyXPos(float xPos)
        {
            using namespace Uma_ECS;

            std::default_random_engine generator(std::random_device{}());
            std::uniform_real_distribution<float> randfloat(1.0f, 5.0f);

            auto& eArray = m_Coordinator.GetComponentArray<Uma_ECS::Enemy>();
            auto& rbArray = m_Coordinator.GetComponentArray<Uma_ECS::RigidBody>();

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

            auto& eArray = m_Coordinator.GetComponentArray<Uma_ECS::Enemy>();
            auto& tfArray = m_Coordinator.GetComponentArray<Uma_ECS::Transform>();

            for (size_t i = 0; i < eArray.Size(); i++)
            {
                auto& tf = tfArray.GetData(eArray.GetEntity(i));

                tf.scale = Vec2{ randScale(generator), randScale(generator) } *scale;
            }
        }

        void ShowBBox(bool isShow)
        {
            using namespace Uma_ECS;

            auto& tfArray = m_Coordinator.GetComponentArray<Uma_ECS::Transform>();
            auto& cArray = m_Coordinator.GetComponentArray<Uma_ECS::Collider>();

            for (size_t i = 0; i < tfArray.Size(); i++)
            {
                if (!cArray.Has(tfArray.GetEntity(i))) continue;
                auto& c = cArray.GetData(tfArray.GetEntity(i));

                c.showBBox = isShow;
            }


        }

        void LoadPrefab()
        {
            m_GameSerializer.loadPrefab(Uma_FilePath::PREFAB_DIR + "enemy.json");
        }
    };
}