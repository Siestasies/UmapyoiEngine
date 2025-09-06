#include "Window.hpp"
#include <iostream>
#include <GLFW/glfw3.h>

int main() 
{
    // Create window
    UmapyoiEngine::Window window(800, 600, "UmapyoiEngine - Game Name");
    
    // Initialize the engine
    if (!window.Initialize()) 
    {
        std::cerr << "Failed to initialize UmapyoiEngine!" << std::endl;
        return -1;
    }
    
    std::cout << "Press ESC to exit." << std::endl;
    
    // Game loop
    while (!window.ShouldClose()) {
        // Check for ESC key
        if (window.IsKeyPressed(GLFW_KEY_ESCAPE)) {
            std::cout << "ESC pressed, exiting..." << std::endl;
            break;
        }
        
        // Update the engine (handles rendering and events)
        window.Update();
    }
    
    // Shut down when window goes out of scope
    std::cout << "Game ended successfully!" << std::endl;
    return 0;
}