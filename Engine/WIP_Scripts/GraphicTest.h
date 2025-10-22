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

#include "Systems/SpriteAnimator.h"

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

Uma_Engine::SpriteAnimator gTestAnimator;
std::string gPlayerDirection = "down";
bool gPlayerMoving = false;

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

            // Load all required textures
            LoadAllTextures();

            // Setup animator
            gTestAnimator.AddClip("idle_down", 4, 4, 0, 1, 0.0f, false);
            gTestAnimator.AddClip("walk_down", 4, 4, 0, 4, 10.0f, true);
            gTestAnimator.AddClip("idle_left", 4, 4, 4, 1, 0.0f, false);
            gTestAnimator.AddClip("walk_left", 4, 4, 4, 4, 10.0f, true);
            gTestAnimator.AddClip("idle_right", 4, 4, 8, 1, 0.0f, false);
            gTestAnimator.AddClip("walk_right", 4, 4, 8, 4, 10.0f, true);
            gTestAnimator.AddClip("idle_up", 4, 4, 12, 1, 0.0f, false);
            gTestAnimator.AddClip("walk_up", 4, 4, 12, 4, 10.0f, true);

            gTestAnimator.Play("idle_down"); // Start facing down

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

            gTestResourcesManager->LoadTexture("player", texturePath + "test.png");
            gTestResourcesManager->LoadTexture("enemy", texturePath + "cirno.png");
            gTestResourcesManager->LoadTexture("pink_enemy", texturePath + "marisa.png");

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
                        .scale = Vec2(10.f, 10.f)
                    });

                gTestCoordinator.AddComponent(
                    gTestPlayer,
                    RigidBody{
                        .velocity = Vec2(0.0f, 0.0f),
                        .acceleration = Vec2(0.0f, 0.0f),
                        .accel_strength = 500,
                        .fric_coeff = 20
                    });

                gTestCoordinator.AddComponent(
                    gTestPlayer,
                    Player{
                        .mSpeed = 1.f
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

            // Get player velocity to check if moving
            auto& playerRB = gTestCoordinator.GetComponent<Uma_ECS::RigidBody>(gTestPlayer);
            float velocityThreshold = 0.1f;
            bool isMoving = (abs(playerRB.velocity.x) > velocityThreshold ||
                abs(playerRB.velocity.y) > velocityThreshold);

            // Determine direction based on velocity
            if (isMoving)
            {
                if (abs(playerRB.velocity.y) > abs(playerRB.velocity.x))
                {
                    // Moving vertically
                    if (playerRB.velocity.y > 0)
                        gPlayerDirection = "up";
                    else
                        gPlayerDirection = "down";
                }
                else
                {
                    // Moving horizontally
                    if (playerRB.velocity.x > 0)
                        gPlayerDirection = "right";
                    else
                        gPlayerDirection = "left";
                }
            }

            // Update animation based on movement and direction
            std::string desiredAnim = isMoving ? "walk_" + gPlayerDirection : "idle_" + gPlayerDirection;

            // Only change animation if different from current
            if (gTestAnimator.GetCurrentClip() != desiredAnim)
            {
                gTestAnimator.Play(desiredAnim);
            }

            gPlayerMoving = isMoving;

            // Update animator
            gTestAnimator.Update(dt);

            // Clear background
            gTestGraphics->ClearBackground(0.2f, 0.3f, 0.3f);

            // Get player transform
            auto& playerTransform = gTestCoordinator.GetComponent<Uma_ECS::Transform>(gTestPlayer);

            // Draw animated player
            unsigned int playerTexID = gTestResourcesManager->GetTexture("player")->tex_id;
            Vec2 uvOffset, uvSize;
            gTestAnimator.GetUVs(uvOffset, uvSize);

            std::vector<Uma_Engine::Sprite_Info> playerSprite;
            playerSprite.push_back(Uma_Engine::Sprite_Info{
                .tex_id = playerTexID,
                .pos = playerTransform.position,
                .scale = Vec2(10.f, 10.f),
                .rot = playerTransform.rotation.x,
                .rot_speed = 0.0f,
                .uvOffset = uvOffset,
                .uvSize = uvSize
                });

            gTestGraphics->DrawSpritesInstanced(playerTexID, playerSprite);

            // Render all other entities (enemies)
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