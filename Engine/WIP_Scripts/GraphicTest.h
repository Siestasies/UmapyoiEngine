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
#include "ECS/Components/Animator.h"
#include "ECS/Systems/RenderingSystem.hpp"
#include "ECS/Systems/PlayerControllerSystem.hpp"
#include "ECS/Systems/PhysicsSystem.hpp"
#include "ECS/Systems/CameraSystem.hpp"
#include "ECS/Systems/AnimatorSystem.hpp"
#include "Systems/Graphics.hpp"
#include "Systems/ResourcesManager.hpp"
#include "Systems/InputSystem.h"
#include "WIP_Scripts/Test_Input_Events.h"
#include "Core/EventSystem.h"
#include "Core/FilePaths.h"
#include <GLFW/glfw3.h>

// Global variables
Uma_Engine::HybridInputSystem* gTestInputSystem;
Uma_Engine::Graphics* gTestGraphics;
Uma_Engine::ResourcesManager* gTestResourcesManager;
Uma_Engine::EventSystem* gTestEventSystem;

Uma_ECS::Coordinator gTestCoordinator;
std::shared_ptr<Uma_ECS::PhysicsSystem> gTestPhysicsSystem;
std::shared_ptr<Uma_ECS::PlayerControllerSystem> gTestPlayerController;
std::shared_ptr<Uma_ECS::RenderingSystem> gTestRenderingSystem;
std::shared_ptr<Uma_ECS::CameraSystem> gTestCameraSystem;
std::shared_ptr<Uma_ECS::AnimatorSystem> gTestAnimatorSystem;
Uma_ECS::Entity gTestPlayer;
Uma_ECS::Entity gTestCam;

namespace Uma_Engine
{
    class GraphicTest : public Scene
    {
    private:
        SystemManager* pSystemManager;
        float fpsTimer = 0.0f;
        int frameCount = 0;
        int currentFPS = 0;
        int playerHealth = 5;
        std::string lastDirection = "down";

    public:
        GraphicTest(SystemManager* sm) : pSystemManager(sm) {}

        void OnLoad() override
        {
            gTestInputSystem = pSystemManager->GetSystem<HybridInputSystem>();
            gTestGraphics = pSystemManager->GetSystem<Graphics>();
            gTestResourcesManager = pSystemManager->GetSystem<ResourcesManager>();
            gTestEventSystem = pSystemManager->GetSystem<EventSystem>();

            LoadAllTextures();

            std::string fontPath = Uma_FilePath::ASSET_ROOT + "fonts/";
            gTestGraphics->LoadFont("title", fontPath + "Urbanist-Regular.ttf", 48);
            gTestGraphics->LoadFont("ui", fontPath + "Neucha.ttf", 24);
            gTestGraphics->SetCurrentFont("ui");

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
            gTestCoordinator.RegisterComponent<Animator>();

            SetupSystems();
            CreateEntities();

            std::cout << "GraphicTest Scene Loaded!" << std::endl;
        }

        void LoadAllTextures()
        {
            std::string texturePath = Uma_FilePath::ASSET_ROOT;
            gTestResourcesManager->LoadTexture("player", texturePath + "test.png");
            gTestResourcesManager->LoadTexture("enemy", texturePath + "cirno.png");
            gTestResourcesManager->LoadTexture("pink_enemy", texturePath + "marisa.png");
            gTestResourcesManager->LoadTexture("background", texturePath + "bg.png");
        }

        void SetupSystems()
        {
            using namespace Uma_ECS;

            // Animator System
            gTestAnimatorSystem = gTestCoordinator.RegisterSystem<AnimatorSystem>();
            {
                Signature sign;
                sign.set(gTestCoordinator.GetComponentType<Animator>());
                gTestCoordinator.SetSystemSignature<AnimatorSystem>(sign);
            }
            gTestAnimatorSystem->Init(&gTestCoordinator);

            // Player Controller
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
                gTestCoordinator.AddComponent(gTestPlayer, Transform{
                    .position = Vec2(0.f, 0.f),
                    .rotation = Vec2(0.f, 0.f),
                    .scale = Vec2(2.f, 2.f)
                    });

                gTestCoordinator.AddComponent(gTestPlayer, RigidBody{
                    .velocity = Vec2(0.0f, 0.0f),
                    .acceleration = Vec2(0.0f, 0.0f),
                    .accel_strength = 500,
                    .fric_coeff = 20
                    });

                gTestCoordinator.AddComponent(gTestPlayer, Player{ .mSpeed = 1.f });

                gTestCoordinator.AddComponent(gTestPlayer, Sprite{
                    .textureName = "player",
                    .flipX = false,
                    .flipY = false,
                    .UseNativeSize = true,
                    .texture = gTestResourcesManager->GetTexture("player")
                    });

                // Add Animator
                Animator playerAnimator;
                playerAnimator.animator.AddClip("idle_down", 4, 4, 0, 1, 0.0f, false);
                playerAnimator.animator.AddClip("walk_down", 4, 4, 0, 4, 10.0f, true);
                playerAnimator.animator.AddClip("idle_left", 4, 4, 4, 1, 0.0f, false);
                playerAnimator.animator.AddClip("walk_left", 4, 4, 4, 4, 10.0f, true);
                playerAnimator.animator.AddClip("idle_right", 4, 4, 8, 1, 0.0f, false);
                playerAnimator.animator.AddClip("walk_right", 4, 4, 8, 4, 10.0f, true);
                playerAnimator.animator.AddClip("idle_up", 4, 4, 12, 1, 0.0f, false);
                playerAnimator.animator.AddClip("walk_up", 4, 4, 12, 4, 10.0f, true);
                playerAnimator.animator.Play("idle_down");

                gTestCoordinator.AddComponent(gTestPlayer, playerAnimator);

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
                gTestCoordinator.AddComponent(enemy1, Transform{
                    .position = Vec2(20.f, 30.f),
                    .rotation = Vec2(0.f, 0.f),
                    .scale = Vec2(5.f, 5.f)
                    });

                gTestCoordinator.AddComponent(enemy1, RigidBody{
                    .velocity = Vec2(0.0f, 0.0f),
                    .acceleration = Vec2(0.0f, 0.0f),
                    .accel_strength = 200,
                    .fric_coeff = 100
                    });

                gTestCoordinator.AddComponent(enemy1, Enemy{ .mSpeed = 1.f });

                gTestCoordinator.AddComponent(enemy1, Sprite{
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
                gTestCoordinator.AddComponent(enemy2, Transform{
                    .position = Vec2(50.f, 50.f),
                    .rotation = Vec2(0.f, 0.f),
                    .scale = Vec2(5.f, 5.f)
                    });

                gTestCoordinator.AddComponent(enemy2, RigidBody{
                    .velocity = Vec2(0.0f, 0.0f),
                    .acceleration = Vec2(0.0f, 0.0f),
                    .accel_strength = 200,
                    .fric_coeff = 100
                    });

                gTestCoordinator.AddComponent(enemy2, Enemy{ .mSpeed = 1.f });

                gTestCoordinator.AddComponent(enemy2, Sprite{
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
                gTestCoordinator.AddComponent(gTestCam, Transform{
                    .position = Vec2(400.0f, 300.0f),
                    .rotation = Vec2(0.f, 0.f),
                    .scale = Vec2(1.f, 1.f)
                    });

                gTestCoordinator.AddComponent(gTestCam, Camera{
                    .mZoom = 1.f,
                    .followPlayer = true
                    });
            }
        }

        void Update(float dt) override
        {
            // FPS calculation
            fpsTimer += dt;
            frameCount++;
            if (fpsTimer >= 1.0f)
            {
                currentFPS = frameCount;
                frameCount = 0;
                fpsTimer = 0.0f;
            }

            // Update systems in order
            gTestPlayerController->Update(dt);
            gTestPhysicsSystem->Update(dt);

            // Update player animation based on movement
            UpdatePlayerAnimation();

            // Update all animators
            gTestAnimatorSystem->Update(dt);

            // Update camera
            gTestCameraSystem->Update(dt);

            // Render
            gTestGraphics->DrawBackground(gTestResourcesManager->GetTexture("background")->tex_id);
            gTestRenderingSystem->Update(dt);

            // Get player components for UI
            auto& playerTransform = gTestCoordinator.GetComponent<Uma_ECS::Transform>(gTestPlayer);
            auto& playerAnimator = gTestCoordinator.GetComponent<Uma_ECS::Animator>(gTestPlayer);
            auto& playerRB = gTestCoordinator.GetComponent<Uma_ECS::RigidBody>(gTestPlayer);

            // WORLD SPACE TEXT
            // Player label
            float textOffsetY = 8.0f;
            std::string playerLabel = "Player";
            float labelWidth = gTestGraphics->MeasureText("ui", playerLabel, 0.08f);
            gTestGraphics->DrawTextWorld("ui", playerLabel,
                playerTransform.position.x - (labelWidth * 0.5f),
                playerTransform.position.y + textOffsetY,
                0.08f, 1.0f, 1.0f, 1.0f);

            // Animation state
            std::string animState = playerAnimator.animator.GetCurrentClip();
            float animWidth = gTestGraphics->MeasureText("ui", animState, 0.06f);
            gTestGraphics->DrawTextWorld("ui", animState,
                playerTransform.position.x - (animWidth * 0.5f),
                playerTransform.position.y + textOffsetY + 2.0f,
                0.06f, 0.5f, 1.0f, 0.5f);

            // SCREEN SPACE TEXT
            // Title - top left
            gTestGraphics->DrawTextScreen("title", "GraphicTest Scene",
                -0.95f, 0.88f, 1.0f);

            // Instructions - bottom left
            gTestGraphics->DrawTextScreen("ui", "Press WASD to move",
                -0.95f, -0.92f, 2.0f, 0.7f, 0.7f, 0.7f);

            // Position info - bottom left
            std::string coordsText = "Pos: (" +
                std::to_string(static_cast<int>(playerTransform.position.x)) + ", " +
                std::to_string(static_cast<int>(playerTransform.position.y)) + ")";
            gTestGraphics->DrawTextScreen("ui", coordsText,
                -0.95f, -0.82f, 2.0f, 0.0f, 1.0f, 0.0f);

            // Speed display
            float speed = sqrtf(playerRB.velocity.x * playerRB.velocity.x +
                playerRB.velocity.y * playerRB.velocity.y);
            std::string speedText = "Speed: " + std::to_string(static_cast<int>(speed));
            gTestGraphics->DrawTextScreen("ui", speedText,
                -0.95f, -0.72f, 2.0f, 1.0f, 1.0f, 0.0f);

            // FPS counter
            std::string fpsText = "FPS: " + std::to_string(currentFPS);
            gTestGraphics->DrawTextScreen("ui", fpsText,
                -0.95f, -0.62f, 2.0f, 0.5f, 0.5f, 0.5f);

            // Draw health bar
            DrawHealthBar();
        }

        // Draw 5 Fumo Cirno sprites as health UI in screen-space (NDC)
        void DrawHealthBar()
        {
            // NDC coordinates
            float startX = 0.40f;
            float startY = 0.75f;

            // Health bar label
            gTestGraphics->DrawTextScreen("ui", "Lives:",
                startX, startY,
                2.0f, 1.0f, 1.0f, 1.0f);

            // Draw 5 Cirno sprites horizontally
            unsigned int cirnoTexture = gTestResourcesManager->GetTexture("enemy")->tex_id;

            float iconSize = 0.1f;
            float spacing = 0.08f;
            float iconsStartX = startX + 0.18f;

            std::vector<Uma_Engine::Sprite_Info> healthIcons;

            for (int i = 0; i < 5; i++)
            {
                float xPos = iconsStartX + (i * spacing);

                // Add Cirno icon
                healthIcons.push_back(Uma_Engine::Sprite_Info{
                    .tex_id = cirnoTexture,
                    .pos = Vec2(xPos, startY),
                    .scale = Vec2(iconSize, iconSize),
                    .rot = 0.0f,
                    .rot_speed = 0.0f,
                    .uvOffset = Vec2(0.0f, 0.0f),
                    .uvSize = Vec2(1.0f, 1.0f)
                    });
            }

            // Draw all health icons in one instanced call
            gTestGraphics->DrawSpritesScreenInstanced(cirnoTexture, healthIcons);
        }

        // UPDATED: Now remembers last direction
        void UpdatePlayerAnimation()
        {
            auto& playerRB = gTestCoordinator.GetComponent<Uma_ECS::RigidBody>(gTestPlayer);
            auto& playerAnimator = gTestCoordinator.GetComponent<Uma_ECS::Animator>(gTestPlayer);

            float velocityThreshold = 0.1f;
            bool isMoving = (abs(playerRB.velocity.x) > velocityThreshold ||
                abs(playerRB.velocity.y) > velocityThreshold);

            // Determine direction based on velocity
            if (isMoving)
            {
                if (abs(playerRB.velocity.y) > abs(playerRB.velocity.x))
                {
                    lastDirection = (playerRB.velocity.y > 0) ? "up" : "down";
                }
                else
                {
                    lastDirection = (playerRB.velocity.x > 0) ? "right" : "left";
                }
            }

            // Use lastDirection for both idle and walk
            std::string desiredAnim = isMoving ? "walk_" + lastDirection : "idle_" + lastDirection;

            if (playerAnimator.animator.GetCurrentClip() != desiredAnim)
            {
                playerAnimator.animator.Play(desiredAnim);
            }
        }

        void Render() override {}

        void OnUnload() override
        {
            std::cout << "GraphicTest Scene Unloaded" << std::endl;
            gTestCoordinator.DestroyAllEntities();
            gTestResourcesManager->UnloadAllTextures();
        }
    };
}