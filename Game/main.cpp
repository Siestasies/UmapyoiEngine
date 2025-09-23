//#include <iostream>
//#include <sstream>
//#include <iomanip>
//
//#include <glad/glad.h>
//#include <GLFW/glfw3.h>
//
//#include "Systems/Window.hpp"
//#include "Systems/Graphics.hpp"
//#include "Core/SystemManager.h"
//#include "Core/EventSystem.h"
//#include "Systems/InputSystem.h" 
//#include "Systems/ResourcesManager.hpp"
//
//#include "WIP_Scripts/Test_Ecs_System.h"
//#include "WIP_Scripts/Test_Graphics.h"
//
//#include "Debugging/Debugger.hpp"
//#include "Debugging/CrashLogger.hpp"
//
//#define DEBUG
//
//int main()
//{
//    // Debug
//    #ifdef DEBUG
//        Uma_Engine::Debugger::Init(true);
//        Uma_Engine::CrashLogger::StartUp();
//    #endif // DEBUG
//
//    // Create window
//    Uma_Engine::Window window(800, 600, "UmapyoiEngine");
//
//    // Initialize the engine
//    if (!window.Initialize())
//    {
//        std::cerr << "Failed to initialize window!" << std::endl;
//        return -1;
//    }
//
//    // Create a systems manager
//    Uma_Engine::SystemManager systemManager;
//
//    // Register InputSystem (and potentially other systems like AudioSystem, RenderSystem, etc.)
//    /*
//        1. graphics
//        ...
//        n. resources
//    
//        test_system
//    */    
//    systemManager.RegisterSystem<Uma_Engine::Graphics>();
//    // Audio
//    systemManager.RegisterSystem<Uma_Engine::InputSystem>();
//    systemManager.RegisterSystem <Uma_Engine::ResourcesManager>();
//
//    // scene
//    systemManager.RegisterSystem<Uma_Engine::Test_Ecs>();
//    systemManager.RegisterSystem<Uma_Engine::Test_Graphics>();
//
//    // Initialize all registered systems
//    systemManager.Init();
//    systemManager.SetWindow(window.GetGLFWWindow());
//
//    // Game loop
//    float lastFrame = 0.0f;
//    float deltaTime = 0.0f;
//    float lastTime = 0.0f;
//    float fps = 0.0f;
//    int frameCount = 0;
//    std::stringstream newTitle;
//    while (!window.ShouldClose())
//    {
//        // calc dt
//        float currentFrame = (float)glfwGetTime();
//        deltaTime = currentFrame - lastFrame;
//        lastFrame = currentFrame;
//        ++frameCount;
//        // update only after 1 second
//        if (currentFrame - lastTime >= 1.0f)
//        {
//            fps = frameCount / (currentFrame - lastTime);
//            frameCount = 0;
//            lastTime = currentFrame;
//
//            // setting of new title/fps
//            newTitle.str("");
//            newTitle.clear();
//
//            newTitle << "UmapyoiEngine | FPS: " << std::fixed << std::setprecision(2) << fps;
//            window.SetTitle(newTitle.str());
//
//            #ifdef DEBUG
//            //Uma_Engine::Debugger::Update();
//            #endif // DEBUG
//        }
//
//        // Update window
//        window.Update();
//
//        // Close if ESC key is pressed
//        if (Uma_Engine::InputSystem::KeyPressed(GLFW_KEY_ESCAPE))
//        {
//            glfwSetWindowShouldClose(window.GetGLFWWindow(), GLFW_TRUE);
//        }
//
//        // Pass deltaTime to systems update (example: physics, rendering, input)
//        systemManager.Update(deltaTime);
//    }
//
//    systemManager.Shutdown();
//    Uma_Engine::Debugger::Shutdown();
//    // Shut down when window goes out of scope
//    std::cout << "Game closed" << std::endl;
//    return 0;
//}

#include <iostream>
#include <sstream>
#include <iomanip>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Systems/Window.hpp"
#include "Systems/Graphics.hpp"
#include "Core/SystemManager.h"
#include "Core/EventSystem.h"
#include "Systems/ResourcesManager.hpp"

#include "WIP_Scripts/Test_Ecs_System.h"
#include "WIP_Scripts/Test_Graphics.h"
#include "WIP_Scripts/Test_Input_Events.h"

#include "Debugging/Debugger.hpp"
#include "Debugging/CrashLogger.hpp"

#include "Systems/SceneType.h"
#include "Systems/SceneManager.h"

#define DEBUG

int main()
{
    // Debug
#ifdef DEBUG
    Uma_Engine::Debugger::Init(true);
    Uma_Engine::CrashLogger::StartUp();
#endif // DEBUG

    // Create window
    Uma_Engine::Window window(800, 600, "UmapyoiEngine - Event System Test");

    // Initialize the engine
    if (!window.Initialize())
    {
        std::cerr << "Failed to initialize window!" << std::endl;
        return -1;
    }

    std::cout << "\n=== TESTING EVENT SYSTEM + INPUT SYSTEM ===\n";

    // Create a systems manager
    Uma_Engine::SystemManager systemManager;

    // Register EVENT SYSTEM FIRST
    std::cout << "Registering EventSystem...\n";
    Uma_Engine::EventSystem* eventSystem = systemManager.RegisterSystem<Uma_Engine::EventSystem>();

    // Register EVENT-ENHANCED INPUT SYSTEM (replaces normal InputSystem)
    std::cout << "Registering EventInputSystem (InputSystem + Events)...\n";
    HybridInputSystem* inputSystem = systemManager.RegisterSystem<HybridInputSystem>();

    // Register SIMPLE EVENT LISTENER (just logs events)
    std::cout << "Registering TestEventListener...\n";
    systemManager.RegisterSystem<TestEventListener>();

    // Register your other systems normally
    systemManager.RegisterSystem<Uma_Engine::Graphics>();
    // Audio
    systemManager.RegisterSystem <Uma_Engine::ResourcesManager>();

    // scene
    systemManager.RegisterSystem<Uma_Engine::SceneManager>();
    systemManager.RegisterSystem<Uma_Engine::ResourcesManager>();
    //systemManager.RegisterSystem<Uma_Engine::Test_Ecs>();
    systemManager.RegisterSystem<Uma_Engine::Test_Graphics>();


    // Initialize all systems
    std::cout << "\nInitializing all systems...\n";
    systemManager.Init();
    systemManager.SetWindow(window.GetGLFWWindow());

    // IMPORTANT: Connect InputSystem to EventSystem
    inputSystem->SetEventSystem(eventSystem);

    // Show what we're listening for
    std::cout << "\nEvent listener counts:\n";
    std::cout << "KeyPress listeners: " << eventSystem->GetListenerCount<Events::KeyPressEvent>() << "\n";
    std::cout << "KeyRelease listeners: " << eventSystem->GetListenerCount<Events::KeyReleaseEvent>() << "\n";
    std::cout << "MouseButton listeners: " << eventSystem->GetListenerCount<Events::MouseButtonEvent>() << "\n";
    std::cout << "MouseMove listeners: " << eventSystem->GetListenerCount<Events::MouseMoveEvent>() << "\n";

    // Game loop
    float lastFrame = 0.0f;
    float deltaTime = 0.0f;
    float lastTime = 0.0f;
    float fps = 0.0f;
    int frameCount = 0;
    std::stringstream newTitle;

    std::cout << "Starting event test loop...\n\n";

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

            newTitle.str("");
            newTitle.clear();
            newTitle << "UmapyoiEngine | FPS: " << std::fixed << std::setprecision(2) << fps;
            window.SetTitle(newTitle.str());

#ifdef DEBUG
            //Uma_Engine::Debugger::Update();
#endif // DEBUG
        }

        // Update window (processes GLFW events -> triggers your InputSystem callbacks)
        window.Update();

        // Check for ESC to quit (using your original InputSystem method)
        if (Uma_Engine::InputSystem::KeyPressed(GLFW_KEY_ESCAPE))
        {
            std::cout << "\nESC pressed - quitting event test\n";
            glfwSetWindowShouldClose(window.GetGLFWWindow(), GLFW_TRUE);
        }

        // Update all systems (this will trigger event dispatching!)
        systemManager.Update(deltaTime);
    }

    std::cout << "\n=== Event System Test Complete ===\n";
    systemManager.Shutdown();
    Uma_Engine::Debugger::Shutdown();

    std::cout << "Event test finished!\n";
    return 0;
} 