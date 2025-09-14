#include "Window.hpp"
#include <iostream>
#include <GLFW/glfw3.h>

#include "Core/SystemManager.h"
#include "Systems/InputSystem.h" 
#include "Graphics.hpp"

int main()
{
    // Create window
    Uma_Engine::Window window(800, 600, "UmapyoiEngine");

    // Initialize the engine
    if (!window.Initialize())
    {
        std::cerr << "Failed to initialize window!" << std::endl;
        return -1;
    }

    // Create a systems manager
    Uma_Engine::SystemManager systemManager;

    // Register InputSystem (and potentially other systems like AudioSystem, RenderSystem, etc.)
    systemManager.RegisterSystem<Uma_Engine::InputSystem>();
    auto* graphics = systemManager.RegisterSystem<Uma_Engine::Graphics>();

    // Initialize all registered systems
    systemManager.Init();
    systemManager.SetWindow(window.GetGLFWWindow());

    unsigned int spriteTexture = graphics->LoadTexture("Assets/sprite.jpg");
    unsigned int backgroundTexture = graphics->LoadTexture("Assets/background.jpg");

    // Check if textures loaded
    if (spriteTexture == 0) {
        std::cout << "Warning: Failed to load sprite texture" << std::endl;
    }
    if (backgroundTexture == 0) {
        std::cout << "Warning: Failed to load background texture" << std::endl;
    }

    // Temp game variables to test
    glm::vec2 playerPosition(400.0f, 300.0f);
    float playerRotation = 0.0f;
    float rotationSpeed = 45.0f;
    float moveSpeed = 200.0f;
    float scale = 0.5f;

    // Game loop
    float lastFrame = 0.0f;
    while (!window.ShouldClose())
    {
        // Calculate deltaTime
        float currentFrame = (float)glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Update window
        window.Update();

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

        // Close if ESC key is pressed
        if (Uma_Engine::InputSystem::KeyPressed(GLFW_KEY_ESCAPE))
        {
            glfwSetWindowShouldClose(window.GetGLFWWindow(), GLFW_TRUE);
        }

        // Clear the background
        graphics->ClearBackground(0.2f, 0.3f, 0.3f);

        // Draw background (if loaded)
        if (backgroundTexture != 0)
        {
            graphics->DrawBackground(backgroundTexture);
        }

        // Draw sprite (if loaded)
        if (spriteTexture != 0)
        {
            graphics->DrawSprite(spriteTexture, playerPosition, glm::vec2(scale), playerRotation);
        }

        // Pass deltaTime to systems update (example: physics, rendering, input)
        systemManager.Update(deltaTime);
    }

    systemManager.Shutdown();
    // Shut down when window goes out of scope
    std::cout << "Game closed" << std::endl;
    return 0;
}
