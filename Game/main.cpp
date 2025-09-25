#include <iostream>
#include <sstream>
#include <iomanip>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

// ImGui includes
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Systems/Window.hpp"
#include "Systems/Graphics.hpp"
#include "Core/SystemManager.h"
#include "Core/EventSystem.h"
#include "Systems/ResourcesManager.hpp"
#include "Systems/Sound.hpp"

#include "WIP_Scripts/Test_Ecs_System.h"
#include "WIP_Scripts/Test_Graphics.h"
#include "WIP_Scripts/Test_Input_Events.h"

#include "Debugging/Debugger.hpp"
#include "Debugging/CrashLogger.hpp"

#include "Systems/SceneType.h"
#include "Systems/SceneManager.h"

#define DEBUG

// ImGui setup and cleanup functions
bool InitializeImGui(GLFWwindow* window)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags &= ~ImGuiConfigFlags_DockingEnable;         // DISable Docking
    //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    io.ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable; // Disable multi-viewports

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    const char* glsl_version = "#version 130";
    ImGui_ImplOpenGL3_Init(glsl_version);

    return true;
}

void ShutdownImGui()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void StartImGuiFrame()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void RenderImGui()
{
    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
    //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

void CreateImGuiDebugWindows(Uma_Engine::SystemManager& systemManager, float fps, float deltaTime)
{
    // Engine debug window
    static bool show_engine_debug = true;
    if (show_engine_debug)
    {
        ImGui::Begin("Engine Debug", &show_engine_debug, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

        // Performance stats
        ImGui::Text("Performance");
        ImGui::Separator();
        ImGui::Text("FPS: %.1f", fps);
        ImGui::Text("Frame Time: %.3f ms", deltaTime * 1000.0f);
        ImGui::Text("Delta Time: %.6f s", deltaTime);

        ImGui::Spacing();

        // System info
        ImGui::Text("Systems");
        ImGui::Separator();
        ImGui::Text("Systems Manager: Active");
        // Add more system status info here as needed

        ImGui::Spacing();

        // OpenGL info
        ImGui::Text("Graphics");
        ImGui::Separator();
        ImGui::Text("OpenGL Version: %s", glGetString(GL_VERSION));
        ImGui::Text("Renderer: %s", glGetString(GL_RENDERER));

        ImGui::End();
    }

    // ImGui Demo window (useful for learning ImGui features)
    static bool show_demo_window = true;
    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    // Event system debug window
    static bool show_event_debug = true;
    if (show_event_debug)
    {
        ImGui::Begin("Event System Debug", &show_event_debug);

        ImGui::Text("Event System Status");
        ImGui::Separator();

        // You can add event system debug info here
        ImGui::Text("Event system is active");

        // Add buttons to test events
        if (ImGui::Button("Test Button"))
        {
            std::cout << "ImGui test button clicked!" << std::endl;
        }

        ImGui::End();
    }
}

int main()
{
    // Debug
#ifdef DEBUG
    Uma_Engine::Debugger::Init(true);
    Uma_Engine::CrashLogger::StartUp();
#endif // DEBUG

    // Create window
    Uma_Engine::Window window(800, 600, "UmapyoiEngine - Event System Test with ImGui");

    // Initialize the engine
    if (!window.Initialize())
    {
        std::cerr << "Failed to initialize window!" << std::endl;
        return -1;
    }

    // Initialize ImGui
    if (!InitializeImGui(window.GetGLFWWindow()))
    {
        std::cerr << "Failed to initialize ImGui!" << std::endl;
        return -1;
    }

    std::cout << "\n=== TESTING EVENT SYSTEM + INPUT SYSTEM + IMGUI ===\n";

    // Create a systems manager
    Uma_Engine::SystemManager systemManager;

    // Register EVENT-ENHANCED INPUT SYSTEM (replaces normal InputSystem)
    std::cout << "Registering EventInputSystem (InputSystem + Events)...\n";
    HybridInputSystem* inputSystem = systemManager.RegisterSystem<HybridInputSystem>();

    // Register EVENT SYSTEM FIRST
    std::cout << "Registering EventSystem...\n";
    Uma_Engine::EventSystem* eventSystem = systemManager.RegisterSystem<Uma_Engine::EventSystem>();

    // Register SIMPLE EVENT LISTENER (just logs events)
    std::cout << "Registering TestEventListener...\n";
    systemManager.RegisterSystem<TestEventListener>();

    // Register your other systems normally
    systemManager.RegisterSystem<Uma_Engine::Graphics>();
    systemManager.RegisterSystem<Uma_Engine::Sound>();
    systemManager.RegisterSystem<Uma_Engine::ResourcesManager>();

    // scene
    //systemManager.RegisterSystem<Uma_Engine::SceneManager>();
    systemManager.RegisterSystem<Uma_Engine::Test_Ecs>();
    //systemManager.RegisterSystem<Uma_Engine::Test_Graphics>();

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

    std::cout << "Starting event test loop with ImGui...\n\n";

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

        Uma_Engine::InputSystem::UpdatePreviousFrameState();

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

        StartImGuiFrame();

        // Create your ImGui windows
        CreateImGuiDebugWindows(systemManager, fps, deltaTime);

        // Render everything
        // Your engine's rendering happens in systemManager.Update()
        // Now render ImGui on top
        RenderImGui();
        // Start ImGui frame

        // Present the frame (this might be handled by your Graphics system)
        // If not, you may need to add glfwSwapBuffers(window.GetGLFWWindow()); here
    }

    std::cout << "\n=== Event System + ImGui Test Complete ===\n";

    // Cleanup
    ShutdownImGui();
    systemManager.Shutdown();
    Uma_Engine::Debugger::Shutdown();

    std::cout << "Event + ImGui test finished!\n";
    return 0;
}

// JED FALLBACK PLAN
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