#include "Test_Graphics.h"
#include "Graphics.hpp"
#include "InputSystem.h"
#include "ResourcesManager.hpp"
#include "../Core/SystemManager.h"
#include "Math/Math.hpp"

#include <iostream>
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

        std::cout << "Test_Graphics initialized successfully!" << std::endl;
    }

    void Test_Graphics::Update(float dt)
    {
        float deltaTime = dt;

        Vec2 mouseScreen{ static_cast<float>(Uma_Engine::InputSystem::GetMouseX()),
            static_cast<float>(Uma_Engine::InputSystem::GetMouseY()) };
        Vec2 mouseWorld = graphics->ScreenToWorld(mouseScreen);
        Vec2 mouseScreenConverted = graphics->WorldToScreen(mouseWorld);

        if (Uma_Engine::InputSystem::MouseButtonPressed(GLFW_MOUSE_BUTTON_1))
        {
            std::cout << "=== Mouse Coordinate Test ===" << std::endl;
            std::cout << "Screen pos: (" << mouseScreen.x << ", " << mouseScreen.y << ")" << std::endl;
            std::cout << "World pos:  (" << mouseWorld.x << ", " << mouseWorld.y << ")" << std::endl;
            std::cout << "Back to screen: (" << mouseScreenConverted.x << ", " << mouseScreenConverted.y << ")" << std::endl;
            std::cout << "Player world pos: (" << playerPosition.x << ", " << playerPosition.y << ")" << std::endl;
            std::cout << "=========================" << std::endl;
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

        const Texture* enemyTexture = resourcesManager->GetTexture("enemy");
        if (enemyTexture != nullptr)
        {
            Vec2 scale{ .2f, .2f };
            // Draw enemies at fixed world positions
            graphics->DrawSprite(enemyTexture->tex_id, enemyTexture->tex_size, Vec2(100, 100), scale);
            graphics->DrawSprite(enemyTexture->tex_id, enemyTexture->tex_size, Vec2(600, 200), scale);
            graphics->DrawSprite(enemyTexture->tex_id, enemyTexture->tex_size, Vec2(800, 500), scale);
            graphics->DrawSprite(enemyTexture->tex_id, enemyTexture->tex_size, Vec2(200, 700), scale);
            graphics->DrawSprite(enemyTexture->tex_id, enemyTexture->tex_size, Vec2(1000, 300), scale);
            graphics->DrawSprite(enemyTexture->tex_id, enemyTexture->tex_size, Vec2(1200, 600), scale);
        }

        // Draw sprite
        const Texture* playerTexture = resourcesManager->GetTexture("player_sprite");
        if (playerTexture != nullptr)
        {
            Vec2 playerPos = { playerPosition.x, playerPosition.y };
            graphics->DrawSprite(playerTexture->tex_id, playerTexture->tex_size, playerPos, Vec2(scale), playerRotation);
        }
    }

    void Test_Graphics::Shutdown()
    {
        // EMPTY
    }
}