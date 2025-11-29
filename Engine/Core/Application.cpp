/*!
\file   Application.cpp

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong
\brief  Implementation of the core Application class responsible for
        initializing, running, and shutting down the engine framework.

This class handles:
- Window creation and GLFW initialization
- Loading engine configuration
- Registering and initializing core systems (input, graphics, audio, scene manager)
- Handling the main update loop, delta time, and framerate limiting
- Event system initialization and dispatch
- Focus/minimize behaviour
- Debug and memory systems (in debug builds)

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/


#include "Application.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Systems/Window.hpp"
#include "Core/SystemManager.h"
#include "Core/EventSystem.h"
#include "Core/EngineConfig.h"
#include "Core/EngineConfigSerializer.h"
#include "Core/FilePaths.h"
#include "Systems/HybridInputSystem.h"
#include "Systems/Graphics.hpp"
#include "Systems/SoundManager.hpp"
#include "Systems/ResourcesManager.hpp"
#include "Systems/SceneManager.h"
#include "Debugging/Debugger.hpp"
#include "Debugging/CrashLogger.hpp"
#include "FileSystem/DropCallback.hpp"

// Events
#include "Events/ApplicationEvents.h"

#ifdef _DEBUG
    #define _CRTDBG_MAP_ALLOC
    #include <crtdbg.h>
    #include "MemoryManager/MemoryManager.hpp"
#endif

namespace Uma_Engine
{
    Application::Application()
        : mWindow(nullptr)
        , mSystemManager(nullptr)
        , mConfig(nullptr)
        , mEventSystem(nullptr)
        , mInputSystem(nullptr)
        , mSceneManager(nullptr)
        , mSoundManager(nullptr)
        , mInitialized(false)
        , mIsEditor(true)
    {
    }

    Application::~Application()
    {
        if (mInitialized)
        {
            Shutdown();
        }
    }

    bool Application::Init()
    {
        if (mInitialized)
        {
            std::cerr << "Application already initialized!" << std::endl;
            return false;
        }

        // Call child PreInit
        PreInit();

        // Initialize debug systems
        InitializeDebugSystems();

        // Load configuration
        LoadConfiguration();

        // Create window
        MakeWindow();

        // Create system manager
        mSystemManager = std::make_unique<SystemManager>();

        // Register core engine systems (common to all applications)
        RegisterCoreSystems();

        // Let derived class register its specific systems (editor or game)
        RegisterSystems();

        // Initialize all systems
        mSystemManager->Init();
        mSystemManager->SetWindow(mWindow->GetGLFWWindow());

        // Connect InputSystem to EventSystem
        mInputSystem->SetEventSystem(mEventSystem);

        // Setup File Drop Callback
        glfwSetDropCallback(mWindow->GetGLFWWindow(), FileDropHandler::DropCallback);

#ifdef _DEBUG
        // Log event listener counts
        Debugger::Log(WarningLevel::eInfo, "\nEvent listener counts:");
        Debugger::Log(WarningLevel::eInfo, "KeyPress listeners: " +
            std::to_string(mEventSystem->GetListenerCount<KeyPressEvent>()));
        Debugger::Log(WarningLevel::eInfo, "KeyRelease listeners: " +
            std::to_string(mEventSystem->GetListenerCount<KeyReleaseEvent>()));
        Debugger::Log(WarningLevel::eInfo, "MouseButton listeners: " +
            std::to_string(mEventSystem->GetListenerCount<MouseButtonEvent>()));
        Debugger::Log(WarningLevel::eInfo, "MouseMove listeners: " +
            std::to_string(mEventSystem->GetListenerCount<MouseMoveEvent>()));
#endif

        SubscribeToEvents();

        // Let derived class perform post-initialization
        PostInit();

        mInitialized = true;
        return true;
    }

    void Application::Run()
    {
        if (!mInitialized)
        {
            std::cerr << "Cannot run application - not initialized!" << std::endl;
            return;
        }

        MainLoop();
    }

    void Application::Shutdown()
    {
        if (!mInitialized)
        {
            return;
        }

        // Let derived class perform pre-shutdown cleanup
        PreShutdown();

        // Shutdown all systems
        if (mSystemManager)
        {
            mSystemManager->Shutdown();
        }

#ifdef _DEBUG
        MemoryManager::Disable();
        MemoryManager::ReportLeaks();
#endif

        mInitialized = false;
    }

    GLFWwindow* Application::GetGLFWWindow() const
    {
        return mWindow ? mWindow->GetGLFWWindow() : nullptr;
    }

    void Application::InitializeDebugSystems()
    {
#ifdef _DEBUG
        MemoryManager::Enable();
#endif
        CrashLogger::StartUp();
    }

    void Application::LoadConfiguration()
    {
        mConfig = std::make_unique<EngineConfig>();
        EngineConfigSerializer configSerializer;
        configSerializer.Register(mConfig.get());
        configSerializer.load(Uma_FilePath::CONFIG_ROOT + "config.json");
    }

    void Application::MakeWindow()
    {
        mWindow = std::make_unique<Window>(
            mConfig->screenWidth,
            mConfig->screenHeight,
            mConfig->windowTitle,
            mIsEditor
        );

        if (!mWindow->Initialize())
        {
            std::cerr << "Failed to initialize window!" << std::endl;
            throw std::runtime_error("Window initialization failed");
        }
    }

    void Application::RegisterCoreSystems()
    {
        // Register EVENT SYSTEM FIRST (other systems depend on it)
        mEventSystem = mSystemManager->RegisterSystem<EventSystem>();

        // Register EVENT-ENHANCED INPUT SYSTEM
        mInputSystem = mSystemManager->RegisterSystem<HybridInputSystem>();

        // Register test event listener (for debugging)
        mSystemManager->RegisterSystem<TestEventListener>();

        // Register core engine systems
        mSystemManager->RegisterSystem<Debugger>();
        mGraphics = mSystemManager->RegisterSystem<Graphics>();
        mSoundManager = mSystemManager->RegisterSystem<SoundManager>();
        mSystemManager->RegisterSystem<ResourcesManager>();

        // Register scene manager
        mSceneManager = mSystemManager->RegisterSystem<SceneManager>();
        mSceneManager->SetSystemManager(mSystemManager.get());
    }

    void Application::SubscribeToEvents()
    {
       /* mEventSystem->Subscribe<Uma_Engine::ApplicationQuitRequest, Application>([this](const ApplicationQuitRequest& e)
            {
                mWindow->Close();
            });*/
    }

    void Application::MainLoop()
    {
        // Game loop timing variables
        float lastFrame = 0.0f;
        float deltaTime = 0.0f;
        float lastTime = 0.0f;
        float fps = 0.0f;
        int frameCount = 0;

        // Frame rate limiting
        const double targetFrameTime = 1.0 / static_cast<double>(mConfig->fps);
        double frameStartTime = glfwGetTime();

        std::stringstream titleStream;

        bool lastFrameFocused = true;

        while (!mWindow->ShouldClose())
        {
            // Frame rate limiting - wait until it's time for next frame
            double currentTime = glfwGetTime();
            double elapsed = currentTime - frameStartTime;

            if (elapsed < targetFrameTime)
            {
                // Busy-wait for remaining time (more accurate than sleep)
                while (glfwGetTime() - frameStartTime < targetFrameTime)
                {
                    // Spin-wait for precise timing
                }
            }

            frameStartTime = glfwGetTime();  // Reset frame timer

            // Calculate delta time
            float currentFrame = static_cast<float>(glfwGetTime());
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            // Cap delta time to prevent large jumps
            deltaTime = min(deltaTime, 1.0f / 30.0f); // cap to 30 FPS worst-case

            ++frameCount;

            // Update FPS display once per second
            if (currentFrame - lastTime >= 1.0f)
            {
                fps = frameCount / (currentFrame - lastTime);
                frameCount = 0;
                lastTime = currentFrame;

                titleStream.str("");
                titleStream.clear();
                titleStream << mConfig->windowTitle << " | FPS: " << std::fixed
                           << std::setprecision(2) << fps;
                mWindow->SetTitle(titleStream.str());
            }

            // Update input state tracking
            HybridInputSystem::UpdatePreviousFrameState();

            // Update window (processes GLFW events)
            mWindow->Update();

            // Handle focus/minimization interruptions
            if (!HandleInterruptions(deltaTime))
            {
                continue; // Skip frame update
            }

            if (Uma_Engine::HybridInputSystem::KeyPressed(GLFW_KEY_F11))
            {
                mWindow->ToggleFullscreen();
            }

            // Update all systems
            mSystemManager->Update(deltaTime);

            // Swap front and back buffers
            glfwSwapBuffers(mWindow->GetGLFWWindow());

            // Let derived class perform custom update logic
            Update(deltaTime);
        }
    }
}