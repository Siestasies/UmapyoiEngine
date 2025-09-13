#include "Window.hpp"
#include "Graphics.hpp"
#include "Input.hpp"
#include <iostream>
#include <GLFW/glfw3.h>

int main()
{
    std::cout << "Starting UmapyoiEngine..." << std::endl;

    // Create window
    UmapyoiEngine::Window window(800, 600, "UmapyoiEngine");

    // Initialize the window
    if (!window.Initialize())
    {
        std::cerr << "Failed to initialize window" << std::endl;
        return -1;
    }

    // Initialize graphics system
    UmapyoiEngine::Graphics graphics;
    if (!graphics.Initialize())
    {
        std::cerr << "Failed to initialize graphics system" << std::endl;
        return -1;
    }

    // Initialize input system
    UmapyoiEngine::Input::Initialize(window.GetGLFWWindow());

    std::cout << "All systems initialized successfully!" << std::endl;

    // Game loop
    while (!window.ShouldClose())
    {
        // Update input
        UmapyoiEngine::Input::Update();

        // Close if ESC key is pressed
        if (UmapyoiEngine::Input::KeyPressed(GLFW_KEY_ESCAPE))
        {
            window.Close();
        }

        // Rendering
        graphics.ClearBackground();

        // Update window
        window.Update();
    }

    std::cout << "Game closed" << std::endl;
    return 0;
}