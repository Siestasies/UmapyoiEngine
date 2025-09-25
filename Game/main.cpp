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
#include "Systems/Sound.hpp"

#include "WIP_Scripts/Test_Ecs_System.h"
#include "WIP_Scripts/Test_Graphics.h"
#include "WIP_Scripts/Test_Input_Events.h"

#include "Debugging/Debugger.hpp"
#include "Debugging/CrashLogger.hpp"

#include "Systems/SceneType.h"
#include "Systems/SceneManager.h"

#define DEBUG

// convenience for printing GLM matrices
std::ostream& operator<<(std::ostream& os, const glm::mat4& m) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++)
            os << m[c][r] << " ";  // glm is column-major
        os << "\n";
    }
    return os;
}

int main()
{
    // Debug
#ifdef DEBUG
    Uma_Engine::Debugger::Init(true);
    Uma_Engine::CrashLogger::StartUp();
#endif // DEBUG

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

    // Register EVENT SYSTEM FIRST
    Uma_Engine::EventSystem* eventSystem = systemManager.RegisterSystem<Uma_Engine::EventSystem>();

    // Register EVENT-ENHANCED INPUT SYSTEM (replaces normal InputSystem)
    Uma_Engine::HybridInputSystem* inputSystem = systemManager.RegisterSystem<Uma_Engine::HybridInputSystem>();

    // Register SIMPLE EVENT LISTENER (just logs events)
    systemManager.RegisterSystem<Uma_Engine::TestEventListener>();

    // Register your other systems normally
    systemManager.RegisterSystem<Uma_Engine::Graphics>();
    systemManager.RegisterSystem<Uma_Engine::Sound>();
    systemManager.RegisterSystem<Uma_Engine::ResourcesManager>();

    // scene
    //systemManager.RegisterSystem<Uma_Engine::SceneManager>();
    systemManager.RegisterSystem<Uma_Engine::Test_Ecs>();
    //systemManager.RegisterSystem<Uma_Engine::Test_Graphics>();


    // Initialize all systems
    systemManager.Init();
    systemManager.SetWindow(window.GetGLFWWindow());

    // Connect InputSystem to EventSystem
    inputSystem->SetEventSystem(eventSystem);

#ifdef DEBUG
    std::cout << "\nEvent listener counts:\n";
    std::cout << "KeyPress listeners: " << eventSystem->GetListenerCount<Uma_Engine::KeyPressEvent>() << "\n";
    std::cout << "KeyRelease listeners: " << eventSystem->GetListenerCount<Uma_Engine::KeyReleaseEvent>() << "\n";
    std::cout << "MouseButton listeners: " << eventSystem->GetListenerCount<Uma_Engine::MouseButtonEvent>() << "\n";
    std::cout << "MouseMove listeners: " << eventSystem->GetListenerCount<Uma_Engine::MouseMoveEvent>() << "\n";
#endif

    // Test GLM-compatibility
    Matrix4x4<float> K(
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
        13, 14, 15, 16
    );


    glm::mat4 G(
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
        13, 14, 15, 16
    );

    std::cout << "Your Matrix K:\n" << K << "\n";
    std::cout << "GLM Matrix G:\n" << G << "\n";

    auto KplusK = K + K;
    auto GplusG = G + G;
    std::cout << "K + K:\n" << KplusK << "\n";
    std::cout << "G + G:\n" << GplusG << "\n";


    // 1. Construct identity and zero matrices
    Matrix4x4<float> I = Matrix4x4<float>::identity();
    Matrix4x4<float> Z = Matrix4x4<float>::zero();

    std::cout << "Identity matrix:\n" << I << "\n\n";
    std::cout << "Zero matrix:\n" << Z << "\n\n";

    // 2. Construct a custom matrix
    Matrix4x4<float> A(
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
        13, 14, 15, 16
    );

    std::cout << "Matrix A:\n" << A << "\n\n";

    // 3. Test addition and subtraction
    Matrix4x4<float> B = A + I;
    Matrix4x4<float> C = A - I;

    std::cout << "A + I:\n" << B << "\n\n";
    std::cout << "A - I:\n" << C << "\n\n";

    // 4. Scalar multiplication/division
    Matrix4x4<float> D = A * 2.0f;
    Matrix4x4<float> E = A / 2.0f;

    std::cout << "A * 2:\n" << D << "\n\n";
    std::cout << "A / 2:\n" << E << "\n\n";

    // 5. Matrix multiplication (I * A = A)
    Matrix4x4<float> F = I * A;
    std::cout << "I * A (should equal A):\n" << F << "\n\n";

    // 6. Transpose
    Matrix4x4<float> At = A.transpose();
    std::cout << "Transpose of A:\n" << At << "\n\n";

    // 7. Determinant (Note: current determinant implementation is only 3x3 placeholder in your code)
    std::cout << "Determinant of A (currently 3x3 placeholder logic): "
        << A.determinant() << "\n\n";

    // 8. Equality check
    std::cout << "A == F ? " << (A == F ? "true" : "false") << "\n";
    std::cout << "A == B ? " << (A == B ? "true" : "false") << "\n";

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

            newTitle.str("");
            newTitle.clear();
            newTitle << "UmapyoiEngine | FPS: " << std::fixed << std::setprecision(2) << fps;
            window.SetTitle(newTitle.str());

#ifdef DEBUG
            //Uma_Engine::Debugger::Update();
#endif // DEBUG
        }

        Uma_Engine::InputSystem::UpdatePreviousFrameState();

        window.Update();

        if (Uma_Engine::InputSystem::KeyPressed(GLFW_KEY_ESCAPE))
        {
            glfwSetWindowShouldClose(window.GetGLFWWindow(), GLFW_TRUE);
        }

        systemManager.Update(deltaTime);
    }

    systemManager.Shutdown();
    Uma_Engine::Debugger::Shutdown();

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