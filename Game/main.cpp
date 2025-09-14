#include "Window.hpp"
#include <iostream>
#include <GLFW/glfw3.h>

#include "Core/SystemManager.h"
#include "Systems/InputSystem.h"

int main()
{
    // Create window
    Uma_Engine::Window window(800, 600, "UmapyoiEngine");

    // Initialize the engine
    if (!window.Initialize())
    {
        std::cerr << "Failed to initialize window" << std::endl;
        return -1;
    }

    // Create a systems manager
    Uma_Engine::SystemManager systemManager;

    // Register InputSystem (and potentially other systems like AudioSystem, RenderSystem, etc.)
    systemManager.RegisterSystem<Uma_Engine::InputSystem>();

    // Initialize all registered systems
    systemManager.Init();
    systemManager.SetWindow(window.GetGLFWWindow());

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

        // Close if ESC key is pressed
        if (Uma_Engine::InputSystem::KeyPressed(GLFW_KEY_ESCAPE))
        {
            glfwSetWindowShouldClose(window.GetGLFWWindow(), GLFW_TRUE);
        }

        // Pass deltaTime to systems update (example: physics, rendering, input)
        systemManager.Update(deltaTime);
    }

    systemManager.Shutdown();
    // Shut down when window goes out of scope
    std::cout << "Game closed" << std::endl;
    return 0;
}
