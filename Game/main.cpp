#include "Window.hpp"
#include "Graphics.hpp"
#include "Input.hpp"
#include <iostream>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>

int main()
{
    std::cout << "Starting UmapyoiEngine with OpenGL 4.5..." << std::endl;

    // Create and initialize window
    UmapyoiEngine::Window window(800, 600, "UmapyoiEngine");
    if (!window.Initialize())
    {
        std::cerr << "Failed to initialize window!" << std::endl;
        return -1;
    }

    // Create and initialize graphics system
    UmapyoiEngine::Graphics graphics;
    if (!graphics.Initialize())
    {
        std::cerr << "Failed to initialize graphics!" << std::endl;
        return -1;
    }

    // Initialize input system
    UmapyoiEngine::Input::Initialize(window.GetGLFWWindow());

    // Load textures
    unsigned int backgroundTexture = graphics.LoadTexture("Assets/background.jpg");
    unsigned int playerTexture = graphics.LoadTexture("Assets/sprite.jpg");

    // Validate texture loading
    if (backgroundTexture == 0)
    {
        std::cout << "Warning: Could not load background" << std::endl;
    }
    if (playerTexture == 0)
    {
        std::cout << "Warning: Could not load sprite" << std::endl;
    }

    // Enable VSync
    graphics.SetVSync(true);

    // Game state variables
    glm::vec2 playerPos(400.0f, 300.0f); // Center of screen
    glm::vec2 playerScale(0.25f, 0.25f);
    float playerRotation = 0.0f;

    // For delta time calculation
    float lastFrameTime = 0.0f;

    // Game loop
    while (!window.ShouldClose())
    {
        // Calculate delta time
        float currentFrameTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        // Handle input
        UmapyoiEngine::Input::Update();

        // Exit on ESC
        if (UmapyoiEngine::Input::KeyPressed(GLFW_KEY_ESCAPE))
        {
            window.Close();
        }

        if (UmapyoiEngine::Input::KeyDown(GLFW_KEY_1))
        {
            playerScale += 0.5f * deltaTime;
        }
        if (UmapyoiEngine::Input::KeyDown(GLFW_KEY_2))
        {
            playerScale -= 0.5f * deltaTime;
        }

        if (UmapyoiEngine::Input::KeyDown(GLFW_KEY_Q))
        {
            playerRotation += 180.0f * deltaTime;
        }
        if (UmapyoiEngine::Input::KeyDown(GLFW_KEY_E))
        {
            playerRotation -= 180.0f * deltaTime;
        }

        // Clear screen with dark blue background
        graphics.ClearBackground(0.1f, 0.2f, 0.4f);

        // Draw background
        if (backgroundTexture != 0)
        {
            graphics.DrawBackground(backgroundTexture);
        }

        // Draw player sprite
        if (playerTexture != 0)
        {
            graphics.DrawSprite(playerTexture, playerPos, playerScale, playerRotation);
        }

        // Update the frame
        window.Update();
    }

    // Cleanup
    std::cout << "Cleaning up resources..." << std::endl;
    if (backgroundTexture != 0) graphics.UnloadTexture(backgroundTexture);
    if (playerTexture != 0) graphics.UnloadTexture(playerTexture);

    std::cout << "UmapyoiEngine closed successfully!" << std::endl;
    return 0;
}