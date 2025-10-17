#include "Test_Graphics.h"
#include "Graphics.hpp"
#include "InputSystem.h"
#include "ResourcesManager.hpp"
#include "../Core/SystemManager.h"
#include "Math/Math.h"

#include <iostream>
#include <vector>
#include <random>
#include <GLFW/glfw3.h>

namespace
{
    Uma_Engine::Graphics* graphics;
    Uma_Engine::ResourcesManager* resourcesManager;

    Uma_Math::Vec2 playerPosition;
    Uma_Math::Vec2 cameraPosition;
    float playerRotation;
    float rotationSpeed;
    float moveSpeed;
    float fScale;
    float zoom;
    bool cameraFollowPlayer = true;
    float cameraLerpSpeed = 5.0f;

    // Demo objects
    struct DemoObject
    {
        Uma_Math::Vec2 position{};
        Uma_Math::Vec2 scale{};
        float rotation{};
        float rotationSpeed{};
    };

    std::vector<DemoObject> demoObjects;
    bool showDemo = false;
    bool debugDrawingEnabled = false;
    const int DEMO_OBJECTS_COUNT = 10000;
}

namespace Uma_Engine
{
    void Test_Graphics::Init()
    {
        graphics = pSystemManager->GetSystem<Graphics>();
        resourcesManager = pSystemManager->GetSystem<ResourcesManager>();

        // Load textures using ResourcesManager
        if (!resourcesManager->LoadTexture("player_sprite", "Assets/hello.jpg"))
        {
            std::cout << "Warning: Failed to load player sprite texture" << std::endl;
        }

        if (!resourcesManager->LoadTexture("background", "Assets/background.jpg"))
        {
            std::cout << "Warning: Failed to load background texture" << std::endl;
        }

        if (!resourcesManager->LoadTexture("enemy", "Assets/sprite.jpg"))
        {
            std::cout << "Warning: Failed to load enemy texture" << std::endl;
        }

        resourcesManager->PrintLoadedTextureNames();

        // Initialize game variables
        playerPosition = { 400.0f, 300.0f };
        cameraPosition = { 400.0f, 300.0f };
        playerRotation = 0.0f;
        rotationSpeed = 45.0f;
        moveSpeed = 200.0f;
        fScale = 10.f;
        zoom = 1.0f;

        // Pre-generate demo objects
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> posX(0.0f, 800.0f);
        std::uniform_real_distribution<float> posY(0.0f, 600.0f);
        std::uniform_real_distribution<float> scaleDist(0.01f, 0.05f);
        std::uniform_real_distribution<float> rotSpeedDist(-180.0f, 180.0f);

        demoObjects.clear();
        demoObjects.reserve(DEMO_OBJECTS_COUNT);

        for (int i = 0; i < DEMO_OBJECTS_COUNT; ++i)
        {
            DemoObject obj;
            obj.position.x = posX(gen);
            obj.position.y = posY(gen);
            obj.scale.x = scaleDist(gen);
            obj.scale.y = scaleDist(gen);
            obj.rotation = 0.0f;
            obj.rotationSpeed = rotSpeedDist(gen);

            demoObjects.push_back(obj);
        }

        std::cout << "Test_Graphics initialized with " << DEMO_OBJECTS_COUNT << " demo objects" << std::endl;
        std::cout << "Controls:" << std::endl;
        std::cout << "  WASD/Arrow Keys - Move player" << std::endl;
        std::cout << "  Q/E - Rotate player" << std::endl;
        std::cout << "  Z/X - Scale player" << std::endl;
        std::cout << "  1/2 - Zoom camera in/out" << std::endl;
        std::cout << "  C - Toggle camera follow player" << std::endl;
        std::cout << "  IJKL - Manual camera movement (when follow is off)" << std::endl;
        std::cout << "  SPACE - Toggle 2500 rotating objects demo" << std::endl;
        std::cout << "  F1 - Toggle debug drawing" << std::endl;
    }

    void Test_Graphics::Update(float dt)
    {
        float deltaTime = dt;
        static bool spacePressed = false;
        static bool f1Pressed = false;
        static bool cPressed = false;

        // Toggle demo with SPACE key
        if (Uma_Engine::InputSystem::KeyDown(GLFW_KEY_SPACE))
        {
            if (!spacePressed)
            {
                showDemo = !showDemo;
                std::cout << "Demo " << (showDemo ? "ENABLED" : "DISABLED") << " - " << DEMO_OBJECTS_COUNT << " rotating objects" << std::endl;
            }
            spacePressed = true;
        }
        else
        {
            spacePressed = false;
        }

        // Toggle debug drawing with F1 key
        if (Uma_Engine::InputSystem::KeyDown(GLFW_KEY_F1))
        {
            if (!f1Pressed)
            {
                debugDrawingEnabled = !debugDrawingEnabled;
                std::cout << "Debug drawing " << (debugDrawingEnabled ? "ENABLED" : "DISABLED") << std::endl;
            }
            f1Pressed = true;
        }
        else
        {
            f1Pressed = false;
        }

        // Toggle camera follow with C key
        if (Uma_Engine::InputSystem::KeyDown(GLFW_KEY_C))
        {
            if (!cPressed)
            {
                cameraFollowPlayer = !cameraFollowPlayer;
                std::cout << "Camera follow player " << (cameraFollowPlayer ? "ENABLED" : "DISABLED") << std::endl;
            }
            cPressed = true;
        }
        else
        {
            cPressed = false;
        }

        // Handle input for sprite movement
        if (Uma_Engine::InputSystem::KeyDown(GLFW_KEY_W) || Uma_Engine::InputSystem::KeyDown(GLFW_KEY_UP))
        {
            playerPosition.y += moveSpeed * deltaTime;
        }
        if (Uma_Engine::InputSystem::KeyDown(GLFW_KEY_S) || Uma_Engine::InputSystem::KeyDown(GLFW_KEY_DOWN))
        {
            playerPosition.y -= moveSpeed * deltaTime;
        }
        if (Uma_Engine::InputSystem::KeyDown(GLFW_KEY_A) || Uma_Engine::InputSystem::KeyDown(GLFW_KEY_LEFT))
        {
            playerPosition.x -= moveSpeed * deltaTime;
        }
        if (Uma_Engine::InputSystem::KeyDown(GLFW_KEY_D) || Uma_Engine::InputSystem::KeyDown(GLFW_KEY_RIGHT))
        {
            playerPosition.x += moveSpeed * deltaTime;
        }

        // Manual camera movement (when not following player)
        if (!cameraFollowPlayer)
        {
            float cameraSpeed = 300.0f;
            if (Uma_Engine::InputSystem::KeyDown(GLFW_KEY_I))
            {
                cameraPosition.y += cameraSpeed * deltaTime;
            }
            if (Uma_Engine::InputSystem::KeyDown(GLFW_KEY_K))
            {
                cameraPosition.y -= cameraSpeed * deltaTime;
            }
            if (Uma_Engine::InputSystem::KeyDown(GLFW_KEY_J))
            {
                cameraPosition.x -= cameraSpeed * deltaTime;
            }
            if (Uma_Engine::InputSystem::KeyDown(GLFW_KEY_L))
            {
                cameraPosition.x += cameraSpeed * deltaTime;
            }
        }

        // Camera zoom
        if (Uma_Engine::InputSystem::KeyDown(GLFW_KEY_1))
        {
            zoom += 1.0f * deltaTime;
            if (zoom > 3.0f) zoom = 3.0f;
        }
        if (Uma_Engine::InputSystem::KeyDown(GLFW_KEY_2))
        {
            zoom -= 1.0f * deltaTime;
            if (zoom < 0.1f) zoom = 0.1f;
        }

        // Rotate sprite
        if (Uma_Engine::InputSystem::KeyDown(GLFW_KEY_Q))
        {
            playerRotation -= rotationSpeed * deltaTime;
        }
        if (Uma_Engine::InputSystem::KeyDown(GLFW_KEY_E))
        {
            playerRotation += rotationSpeed * deltaTime;
        }

        // Scale sprite
        if (Uma_Engine::InputSystem::KeyDown(GLFW_KEY_Z))
        {
            fScale -= 1.0f * deltaTime;
            if (fScale < 0.1f) fScale = 0.1f;
        }
        if (Uma_Engine::InputSystem::KeyDown(GLFW_KEY_X))
        {
            fScale += 1.0f * deltaTime;
            if (fScale > 3.0f) fScale = 3.0f;
        }

        // World-to-screen & screen-to-world
        if (Uma_Engine::InputSystem::MouseButtonPressed(GLFW_MOUSE_BUTTON_1))
        {
            Vec2 mouseScreen{ static_cast<float>(Uma_Engine::InputSystem::GetMouseX()), static_cast<float>(Uma_Engine::InputSystem::GetMouseY())};
            Vec2 mouseWorld = graphics->ScreenToWorld(mouseScreen);
            Vec2 backToScreen = graphics->WorldToScreen(mouseWorld);

            std::cout << "Screen: (" << mouseScreen.x << ", " << mouseScreen.y << ")" << std::endl;
            std::cout << "World:  (" << mouseWorld.x << ", " << mouseWorld.y << ")" << std::endl;
            std::cout << "Back:   (" << backToScreen.x << ", " << backToScreen.y << ")" << std::endl;
        }

        // Update camera position
        if (cameraFollowPlayer)
        {
            // Smooth camera follow using lerp
            cameraPosition.x += (playerPosition.x - cameraPosition.x) * cameraLerpSpeed * deltaTime;
            cameraPosition.y += (playerPosition.y - cameraPosition.y) * cameraLerpSpeed * deltaTime;
        }

        // Update demo object rotations
        if (showDemo)
        {
            for (auto& obj : demoObjects)
            {
                obj.rotation += obj.rotationSpeed * deltaTime;
            }
        }

        // Update camera in graphics system using your new SetCamInfo method
        graphics->SetCamInfo(cameraPosition, zoom);

        // Clear the background
        graphics->ClearBackground(0.2f, 0.3f, 0.3f);

        // Draw background
        const Texture* backgroundTexture = resourcesManager->GetTexture("background");
        if (backgroundTexture != nullptr)
        {
            graphics->DrawBackground(backgroundTexture->tex_id);
        }

        // Enemy positions for collision debug drawing
        std::vector<Vec2> enemyPositions;

        // Draw original enemy objects
        if (!showDemo)
        {
            const Texture* enemyTexture = resourcesManager->GetTexture("enemy");
            if (enemyTexture != nullptr)
            {
                Vec2 enemyScale{ .2f, .2f };
                Vec2 positions[] = {
                    Vec2(100, 100), Vec2(600, 200), Vec2(800, 500),
                    Vec2(200, 700), Vec2(1000, 300), Vec2(1200, 600)
                };

                for (const Vec2& pos : positions)
                {
                    graphics->DrawSprite(enemyTexture->tex_id, pos, enemyTexture->tex_size);

                    if (debugDrawingEnabled)
                    {
                        enemyPositions.push_back(pos);
                    }
                }
            }
        }

        if (showDemo && !demoObjects.empty())
        {
            const Texture* enemyTexture = resourcesManager->GetTexture("enemy");
            if (enemyTexture != nullptr)
            {
                // Prepare vectors for instanced rendering
                std::vector<Sprite_Info> sprites;
                

                sprites.reserve(demoObjects.size());
                
                // Collect all instance data
                for (const auto& obj : demoObjects)
                {
                    sprites.push_back(
                        Sprite_Info
                        {
                            .tex_id = enemyTexture->tex_id,
                            .pos = obj.position,
                            .scale = obj.scale,
                            .rot = obj.rotation,
                            .rot_speed = obj.rotationSpeed
                        });
                }

                graphics->DrawSpritesInstanced(
                    enemyTexture->tex_id,
                    //enemyTexture->tex_size, 
                    sprites
                );
            }
        }
        else
        {
            const Texture* enemyTexture = resourcesManager->GetTexture("enemy");
            if (enemyTexture != nullptr)
            {
                Vec2 enemyScale{ .2f, .2f };
                Vec2 positions[] = {
                    Vec2(100, 100), Vec2(600, 200), Vec2(800, 500),
                    Vec2(200, 700), Vec2(1000, 300), Vec2(1200, 600)
                };

                for (const Vec2& pos : positions)
                {
                    graphics->DrawSprite(enemyTexture->tex_id, pos, enemyScale);
                }
            }
        }

        // Draw player sprite
        const Texture* playerTexture = resourcesManager->GetTexture("player_sprite");
        if (playerTexture != nullptr)
        {
            Vec2 playerPos = { playerPosition.x, playerPosition.y };
            graphics->DrawSprite(playerTexture->tex_id, playerPos, Vec2(fScale), playerRotation);
        }

        // Draw debug info
        if (debugDrawingEnabled)
        {
            Vec2 playerPos = { playerPosition.x, playerPosition.y };

            // Player center point (red)
            graphics->DrawDebugPoint(playerPos, 1.0f, 0.0f, 0.0f);

            // Player collision circle (blue)
            float playerCollisionRadius = 40.0f;
            graphics->DrawDebugCircle(playerPos, playerCollisionRadius, 0.0f, 0.0f, 1.0f);

            // Camera center point (magenta)
            graphics->DrawDebugPoint(cameraPosition, 1.0f, 0.0f, 1.0f);

            // Debug drawing for all enemies/objects
            for (const Vec2& enemyPos : enemyPositions)
            {
                graphics->DrawDebugPoint(enemyPos, 1.0f, 0.0f, 0.0f);

                Vec2 collisionSize;
                if (showDemo)
                {
                    collisionSize = Vec2(15.0f, 25.0f);
                }
                else
                {
                    collisionSize = Vec2(30.0f, 60.0f);
                }
                graphics->DrawDebugRect(enemyPos, collisionSize, 0.0f, 1.0f, 0.0f);
                graphics->DrawDebugLine(enemyPos, playerPos, 1.0f, 1.0f, 0.0f);
            }
        }
    }

    void Test_Graphics::Shutdown()
    {
        demoObjects.clear();
    }
}