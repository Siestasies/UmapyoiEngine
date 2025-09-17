#include <iostream>
#include <sstream>
#include <iomanip>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Systems/Window.hpp"
#include "Systems/Graphics.hpp"
#include "Core/SystemManager.h"
#include "Systems/InputSystem.h" 
#include "Systems/ResourcesManager.hpp"

#include "WIP_Scripts/Test_Ecs_System.h"
#include "WIP_Scripts/Test_Graphics.h"

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

    /*
        1. graphics
        ...
        n. resources
    
        test_system
    */

    systemManager.RegisterSystem<Uma_Engine::Graphics>();
    systemManager.RegisterSystem<Uma_Engine::InputSystem>();
    systemManager.RegisterSystem <Uma_Engine::ResourcesManager>();
    systemManager.RegisterSystem<Uma_Engine::Test_Ecs>();
    systemManager.RegisterSystem<Uma_Engine::Test_Graphics>();

    // Initialize all registered systems
    systemManager.Init();
    systemManager.SetWindow(window.GetGLFWWindow());

    // Game loop
    float lastFrame = 0.0f;
    float deltaTime = 0.0f;
    float lastTime = 0.0f;
    float fps = 0.0f;
    int frameCount = 0;
    std::stringstream newTitle;
    while (!window.ShouldClose())
    {
        // calc dt
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        ++frameCount;
        // update only after 1 second
        if (currentFrame - lastTime >= 1.0f)
        {
            fps = frameCount / (currentFrame - lastTime);
            frameCount = 0;
            lastTime = currentFrame;

            // setting of new title/fps
            newTitle.str("");
            newTitle.clear();

            newTitle << "UmapyoiEngine | FPS: " << std::fixed << std::setprecision(2) << fps;
            window.SetTitle(newTitle.str());
        }

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
