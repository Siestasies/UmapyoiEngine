#include "Graphics.hpp"

#include "Test_Graphics.h"
#include "InputSystem.h"
#include "../Core/SystemManager.h"

#include "Math/Math.hpp"

#include <iostream>
#include <GLFW/glfw3.h>

Uma_Engine::Graphics* gGraphics;

Vec2  playerPosition;
float playerRotation;
float rotationSpeed;
float moveSpeed;
float scale;

unsigned int spriteTexture;
unsigned int backgroundTexture;

void Uma_Engine::Test_Graphics::Init()
{
    gGraphics = pSystemManager->GetSystem<Graphics>();

    spriteTexture      = gGraphics->LoadTexture("Assets/hello.jpg");
    backgroundTexture  = gGraphics->LoadTexture("Assets/background.jpg");

    // Check if textures loaded
    if (spriteTexture == 0) {
        std::cout << "Warning: Failed to load sprite texture" << std::endl;
    }
    if (backgroundTexture == 0) {
        std::cout << "Warning: Failed to load background texture" << std::endl;
    }

    // Temp game variables to test
    playerPosition = { 400.0f, 300.0f };
    playerRotation = 0.0f;
    rotationSpeed = 45.0f;
    moveSpeed = 200.0f;
    scale = 0.5f;
}

void Uma_Engine::Test_Graphics::Update(float dt)
{
    float deltaTime = dt;

    // Handle input for sprite movement
    if (Uma_Engine::InputSystem::KeyDown(GLFW_KEY_W) || Uma_Engine::InputSystem::KeyDown(GLFW_KEY_UP))
    {
        playerPosition.y -= moveSpeed * deltaTime;
    }
    if (Uma_Engine::InputSystem::KeyDown(GLFW_KEY_S) || Uma_Engine::InputSystem::KeyDown(GLFW_KEY_DOWN))
    {
        playerPosition.y += moveSpeed * deltaTime;
    }
    if (Uma_Engine::InputSystem::KeyDown(GLFW_KEY_A) || Uma_Engine::InputSystem::KeyDown(GLFW_KEY_LEFT))
    {
        playerPosition.x -= moveSpeed * deltaTime;
    }
    if (Uma_Engine::InputSystem::KeyDown(GLFW_KEY_D) || Uma_Engine::InputSystem::KeyDown(GLFW_KEY_RIGHT))
    {
        playerPosition.x += moveSpeed * deltaTime;
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

    // Clear the background
    gGraphics->ClearBackground(0.2f, 0.3f, 0.3f);

    // Draw background (if loaded)
    if (backgroundTexture != 0)
    {
        gGraphics->DrawBackground(backgroundTexture);
    }

    // Draw sprite (if loaded)
    if (spriteTexture != 0)
    {
        Vec2 playerPos = { playerPosition.x, playerPosition .y };
        gGraphics->DrawSprite(spriteTexture, playerPos, Vec2(scale), playerRotation);
    }
}

void Uma_Engine::Test_Graphics::Shutdown()
{

}
