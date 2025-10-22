#pragma once
#include "Systems/SceneType.h"
#include "Core/SystemManager.h"
#include "ECS/Core/Coordinator.hpp"
#include "ECS/Components/Transform.h"
#include "ECS/Components/RigidBody.h"
#include "ECS/Components/Player.h"
#include "ECS/Components/Enemy.h"
#include "ECS/Components/Sprite.h"
#include "ECS/Components/Collider.h"
#include "ECS/Components/Camera.h"
#include "ECS/Systems/RenderingSystem.hpp"
#include "ECS/Systems/PlayerControllerSystem.hpp"
#include "ECS/Systems/PhysicsSystem.hpp"
#include "ECS/Systems/CameraSystem.hpp"
#include "Systems/Graphics.hpp"
#include "Systems/ResourcesManager.hpp"
#include "Systems/InputSystem.h"
#include "WIP_Scripts/Test_Input_Events.h"
#include "Core/EventSystem.h"
#include "Core/FilePaths.h"
#include <GLFW/glfw3.h>

// Global variables for this scene
Uma_Engine::HybridInputSystem* gTestInputSystem;
Uma_Engine::Graphics* gTestGraphics;
Uma_Engine::ResourcesManager* gTestResourcesManager;
Uma_Engine::EventSystem* gTestEventSystem;

Uma_ECS::Coordinator gTestCoordinator;
std::shared_ptr<Uma_ECS::PhysicsSystem> gTestPhysicsSystem;
std::shared_ptr<Uma_ECS::PlayerControllerSystem> gTestPlayerController;
std::shared_ptr<Uma_ECS::RenderingSystem> gTestRenderingSystem;
std::shared_ptr<Uma_ECS::CameraSystem> gTestCameraSystem;
Uma_ECS::Entity gTestPlayer;
Uma_ECS::Entity gTestCam;

namespace Uma_Engine
{
    class GraphicTest : public Scene
    {
    private:
        SystemManager* pSystemManager;

    public:
        GraphicTest(SystemManager* sm) : pSystemManager(sm) {}

        void OnLoad() override
        {
            // Get engine systems
            gTestInputSystem = pSystemManager->GetSystem<HybridInputSystem>();
            gTestGraphics = pSystemManager->GetSystem<Graphics>();
            gTestResourcesManager = pSystemManager->GetSystem<ResourcesManager>();
            gTestEventSystem = pSystemManager->GetSystem<EventSystem>();

            // Load all required textures FIRST
            LoadAllTextures();

            // Initialize ECS
            using namespace Uma_ECS;

            gTestCoordinator.Init(gTestEventSystem);

            // Register components
            gTestCoordinator.RegisterComponent<Transform>();
            gTestCoordinator.RegisterComponent<RigidBody>();
            gTestCoordinator.RegisterComponent<Collider>();
            gTestCoordinator.RegisterComponent<Sprite>();
            gTestCoordinator.RegisterComponent<Camera>();
            gTestCoordinator.RegisterComponent<Player>();
            gTestCoordinator.RegisterComponent<Enemy>();

            // Setup systems
            SetupSystems();

            // Create entities
            CreateEntities();

            std::cout << "GraphicTest Scene Loaded!" << std::endl;
        }

        void LoadAllTextures()
        {
            // Load textures manually - adjust paths to match your asset structure
            std::string texturePath = Uma_FilePath::ASSET_ROOT;

            gTestResourcesManager->LoadTexture("player", texturePath + "hello.jpg");
            gTestResourcesManager->LoadTexture("enemy", texturePath + "cirno.png");
            gTestResourcesManager->LoadTexture("pink_enemy", texturePath + "marisa.png");

            // Optional: Load background if you have one
            // gTestResourcesManager->LoadTexture("background", texturePath + "background.png");

            std::cout << "Textures loaded successfully" << std::endl;
        }

        void SetupSystems()
        {
            using namespace Uma_ECS;

            // Player controller
            gTestPlayerController = gTestCoordinator.RegisterSystem<PlayerControllerSystem>();
            {
                Signature sign;
                sign.set(gTestCoordinator.GetComponentType<RigidBody>());
                sign.set(gTestCoordinator.GetComponentType<Transform>());
                sign.set(gTestCoordinator.GetComponentType<Player>());
                gTestCoordinator.SetSystemSignature<PlayerControllerSystem>(sign);
            }
            gTestPlayerController->Init(gTestEventSystem, gTestInputSystem, &gTestCoordinator);

            // Physics System
            gTestPhysicsSystem = gTestCoordinator.RegisterSystem<PhysicsSystem>();
            {
                Signature sign;
                sign.set(gTestCoordinator.GetComponentType<RigidBody>());
                sign.set(gTestCoordinator.GetComponentType<Transform>());
                gTestCoordinator.SetSystemSignature<PhysicsSystem>(sign);
            }
            gTestPhysicsSystem->Init(&gTestCoordinator);

            // Rendering System
            gTestRenderingSystem = gTestCoordinator.RegisterSystem<RenderingSystem>();
            {
                Signature sign;
                sign.set(gTestCoordinator.GetComponentType<Sprite>());
                sign.set(gTestCoordinator.GetComponentType<Transform>());
                gTestCoordinator.SetSystemSignature<RenderingSystem>(sign);
            }
            gTestRenderingSystem->Init(gTestGraphics, gTestResourcesManager, &gTestCoordinator);

            // Camera System
            gTestCameraSystem = gTestCoordinator.RegisterSystem<CameraSystem>();
            {
                Signature sign;
                sign.set(gTestCoordinator.GetComponentType<Camera>());
                sign.set(gTestCoordinator.GetComponentType<Transform>());
                gTestCoordinator.SetSystemSignature<CameraSystem>(sign);
            }
            gTestCameraSystem->Init(&gTestCoordinator);
        }

        void CreateEntities()
        {
            using namespace Uma_ECS;

            // Create Player
            gTestPlayer = gTestCoordinator.CreateEntity();
            {
                gTestCoordinator.AddComponent(
                    gTestPlayer,
                    Transform{
                        .position = Vec2(0.f, 0.f),
                        .rotation = Vec2(0.f, 0.f),
                        .scale = Vec2(1.5f, 1.5f)
                    });

                gTestCoordinator.AddComponent(
                    gTestPlayer,
                    RigidBody{
                        .velocity = Vec2(0.0f, 0.0f),
                        .acceleration = Vec2(0.0f, 0.0f),
                        .accel_strength = 500,
                        .fric_coeff = 5
                    });

                gTestCoordinator.AddComponent(
                    gTestPlayer,
                    Player{
                        .mSpeed = 1.f
                    });

                gTestCoordinator.AddComponent(
                    gTestPlayer,
                    Sprite{
                        .textureName = "player",
                        .flipX = false,
                        .flipY = false,
                        .UseNativeSize = true,
                        .texture = gTestResourcesManager->GetTexture("player")
                    });

                Collider playerCollider;
                playerCollider.shapes.push_back(ColliderShape{
                    .purpose = ColliderPurpose::Physics,
                    .layer = CL_PLAYER,
                    .colliderMask = CL_ENEMY,
                    .isActive = true,
                    .autoFitToSprite = true
                    });
                playerCollider.bounds.resize(playerCollider.shapes.size());
                gTestCoordinator.AddComponent(gTestPlayer, playerCollider);
            }

            // Create Enemy 1
            Entity enemy1 = gTestCoordinator.CreateEntity();
            {
                gTestCoordinator.AddComponent(
                    enemy1,
                    Transform{
                        .position = Vec2(25.f, 25.f),
                        .rotation = Vec2(0.f, 0.f),
                        .scale = Vec2(5.f, 5.f)
                    });

                gTestCoordinator.AddComponent(
                    enemy1,
                    RigidBody{
                        .velocity = Vec2(0.0f, 0.0f),
                        .acceleration = Vec2(0.0f, 0.0f),
                        .accel_strength = 200,
                        .fric_coeff = 100
                    });

                gTestCoordinator.AddComponent(
                    enemy1,
                    Enemy{
                        .mSpeed = 1.f
                    });

                gTestCoordinator.AddComponent(
                    enemy1,
                    Sprite{
                        .textureName = "enemy",
                        .flipX = false,
                        .flipY = false,
                        .UseNativeSize = true,
                        .texture = gTestResourcesManager->GetTexture("enemy")
                    });

                Collider enemyCollider;
                enemyCollider.shapes.push_back(ColliderShape{
                    .purpose = ColliderPurpose::Physics,
                    .layer = CL_ENEMY,
                    .colliderMask = CL_PLAYER,
                    .isActive = true,
                    .autoFitToSprite = true
                    });
                enemyCollider.bounds.resize(enemyCollider.shapes.size());
                gTestCoordinator.AddComponent(enemy1, enemyCollider);
            }

            // Create Enemy 2
            Entity enemy2 = gTestCoordinator.CreateEntity();
            {
                gTestCoordinator.AddComponent(
                    enemy2,
                    Transform{
                        .position = Vec2(50.f, 50.f),
                        .rotation = Vec2(0.f, 0.f),
                        .scale = Vec2(5.f, 5.f)
                    });

                gTestCoordinator.AddComponent(
                    enemy2,
                    RigidBody{
                        .velocity = Vec2(0.0f, 0.0f),
                        .acceleration = Vec2(0.0f, 0.0f),
                        .accel_strength = 200,
                        .fric_coeff = 100
                    });

                gTestCoordinator.AddComponent(
                    enemy2,
                    Enemy{
                        .mSpeed = 1.f
                    });

                gTestCoordinator.AddComponent(
                    enemy2,
                    Sprite{
                        .textureName = "pink_enemy",
                        .flipX = false,
                        .flipY = false,
                        .UseNativeSize = true,
                        .texture = gTestResourcesManager->GetTexture("pink_enemy")
                    });

                Collider enemyCollider2;
                enemyCollider2.shapes.push_back(ColliderShape{
                    .purpose = ColliderPurpose::Physics,
                    .layer = CL_ENEMY,
                    .colliderMask = CL_PLAYER,
                    .isActive = true,
                    .autoFitToSprite = true
                    });
                enemyCollider2.bounds.resize(enemyCollider2.shapes.size());
                gTestCoordinator.AddComponent(enemy2, enemyCollider2);
            }

            // Create Camera
            gTestCam = gTestCoordinator.CreateEntity();
            {
                gTestCoordinator.AddComponent(
                    gTestCam,
                    Transform{
                        .position = Vec2(400.0f, 300.0f),
                        .rotation = Vec2(0.f, 0.f),
                        .scale = Vec2(1.f, 1.f)
                    });

                gTestCoordinator.AddComponent(
                    gTestCam,
                    Camera{
                        .mZoom = 1.f,
                        .followPlayer = true
                    });
            }
        }

        void Update(float dt) override
        {
            // Update systems
            gTestPlayerController->Update(dt);
            gTestPhysicsSystem->Update(dt);
            gTestCameraSystem->Update(dt);

            // Clear background
            gTestGraphics->ClearBackground(0.2f, 0.3f, 0.3f);

            // Optional: Draw background texture if you have one
            // auto* bgTexture = gTestResourcesManager->GetTexture("background");
            // if (bgTexture) {
            //     gTestGraphics->DrawBackground(bgTexture->tex_id);
            // }

            // Render all entities
            gTestRenderingSystem->Update(dt);
        }

        void Render() override
        {
            // Empty - rendering handled in Update
        }

        void OnUnload() override
        {
            std::cout << "GraphicTest Scene Unloaded" << std::endl;
            gTestCoordinator.DestroyAllEntities();
            gTestResourcesManager->UnloadAllTextures();
        }
    };
}