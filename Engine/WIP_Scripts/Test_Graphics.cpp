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
    float playerRotation;
    float rotationSpeed;
    float moveSpeed;
    float scale;
    float zoom;

    // Demo objects
    struct DemoObject
    {
        Uma_Math::Vec2 position;
        Uma_Math::Vec2 scale;
        float rotation;
        float rotationSpeed;
    };

    std::vector<DemoObject> demoObjects;
    bool showDemo = false;
    bool debugDrawingEnabled = false;
    const int DEMO_OBJECTS_COUNT = 2500;
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
        playerRotation = 0.0f;
        rotationSpeed = 45.0f;
        moveSpeed = 200.0f;
        scale = 0.3f;
        zoom = 1.0f;

        // Pre-generate demo objects
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> posX(0.0f, 800.0f); // Standard viewport width
        std::uniform_real_distribution<float> posY(0.0f, 600.0f); // Standard viewport height
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
        std::cout << "Press SPACE to toggle 2500 rotating objects demo" << std::endl;
        std::cout << "Press F1 to toggle debug drawing" << std::endl;
    }

    void Test_Graphics::Update(float dt)
    {
        float deltaTime = dt;
        static bool spacePressed = false;
        static bool f1Pressed = false;

        if (Uma_Engine::InputSystem::KeyPressed(GLFW_KEY_0)) std::cout << "0 PRESSED\n";

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

        // Camera zoom
        if (Uma_Engine::InputSystem::KeyDown(GLFW_KEY_1))
        {
            zoom += 1 * dt;
        }
        if (Uma_Engine::InputSystem::KeyDown(GLFW_KEY_2))
        {
            zoom -= 1 * dt;
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
            scale -= 1.0f * deltaTime;
            if (scale < 0.1f) scale = 0.1f;
        }
        if (Uma_Engine::InputSystem::KeyDown(GLFW_KEY_X))
        {
            scale += 1.0f * deltaTime;
            if (scale > 3.0f) scale = 3.0f;
        }

        // Update demo object rotations
        if (showDemo)
        {
            for (auto& obj : demoObjects)
            {
                obj.rotation += obj.rotationSpeed * deltaTime;
            }
        }

        // Update camera
        graphics->GetCamera().SetPosition(playerPosition);
        graphics->GetCamera().SetZoom(zoom);

        // Clear the background
        graphics->ClearBackground(0.2f, 0.3f, 0.3f);

        // Draw background
        const Texture* backgroundTexture = resourcesManager->GetTexture("background");
        if (backgroundTexture != nullptr)
        {
            graphics->DrawBackground(backgroundTexture->tex_id, backgroundTexture->tex_size);
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

                // Draw enemies and collect positions for debug drawing later
                for (const Vec2& pos : positions)
                {
                    graphics->DrawSprite(enemyTexture->tex_id, enemyTexture->tex_size, pos, enemyScale);

                    // Collect positions for debug drawing
                    if (debugDrawingEnabled)
                    {
                        enemyPositions.push_back(pos);
                    }
                }
            }
        }

        // Draw all 2500 demo objects with random scales and rotation speeds
        if (showDemo)
        {
            const Texture* enemyTexture = resourcesManager->GetTexture("enemy");
            if (enemyTexture != nullptr)
            {
                for (const auto& obj : demoObjects)
                {
                    graphics->DrawSprite(enemyTexture->tex_id, enemyTexture->tex_size,
                        obj.position, obj.scale, obj.rotation);

                    // Collect all demo object positions for debug drawing
                    if (debugDrawingEnabled)
                    {
                        enemyPositions.push_back(obj.position);
                    }
                }
            }
        }

        // Draw player sprite
        const Texture* playerTexture = resourcesManager->GetTexture("player_sprite");
        if (playerTexture != nullptr)
        {
            Vec2 playerPos = { playerPosition.x, playerPosition.y };
            graphics->DrawSprite(playerTexture->tex_id, playerTexture->tex_size, playerPos, Vec2(scale), playerRotation);
        }

        // Draw debug info
        if (debugDrawingEnabled)
        {
            // Debug drawing for player
            Vec2 playerPos = { playerPosition.x, playerPosition.y };

            // Player center point (red)
            graphics->DrawDebugPoint(playerPos, 1.0f, 0.0f, 0.0f);

            // Player collision circle (blue)
            float playerCollisionRadius = 40.0f;
            graphics->DrawDebugCircle(playerPos, playerCollisionRadius, 0.0f, 0.0f, 1.0f);

            // Debug drawing for all enemies/objects
            for (const Vec2& enemyPos : enemyPositions)
            {
                // Center point (red)
                graphics->DrawDebugPoint(enemyPos, 1.0f, 0.0f, 0.0f);

                // Collision rectangle (green)
                Vec2 collisionSize;
                if (showDemo)
                {
                    // Smaller rectangles for demo objects
                    collisionSize = Vec2(15.0f, 25.0f);
                }
                else
                {
                    // Larger rectangles for regular enemies
                    collisionSize = Vec2(30.0f, 60.0f);
                }
                graphics->DrawDebugRect(enemyPos, collisionSize, 0.0f, 1.0f, 0.0f);

                // Line to player (yellow)
                graphics->DrawDebugLine(enemyPos, playerPos, 1.0f, 1.0f, 0.0f);
            }
        }
    }

    void Test_Graphics::Shutdown()
    {
        demoObjects.clear();
    }
}