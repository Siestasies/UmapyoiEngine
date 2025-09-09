#include "Window.hpp"
#include <iostream>
#include <GLFW/glfw3.h>
#include "Input.hpp"

int main() 
{
    // Create window
    UmapyoiEngine::Window window(800, 600, "UmapyoiEngine");
    
    // Initialize the engine
    if (!window.Initialize()) 
    {
        std::cerr << "Failed to initialize window" << std::endl;
        return -1;
    }

    // Initialize input system
    UmapyoiEngine::Input::Initialize(window.GetGLFWWindow());
    
    // Game loop
    while (!window.ShouldClose()) 
    {
        // Update window
        window.Update();

        // Close if ESC key is pressed
        if (UmapyoiEngine::Input::KeyPressed(GLFW_KEY_ESCAPE))
        {
            window.Close();
        }

        UmapyoiEngine::Input::Update();
    }
    
    // Shut down when window goes out of scope
    std::cout << "Game closed" << std::endl;
    return 0;
}