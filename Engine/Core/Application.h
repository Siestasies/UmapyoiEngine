/*!
\file   Application.h
\brief  Base application class that encapsulates the engine's core systems.
        Provides a framework for derived applications (Editor, Game) to run.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include <memory>
#include <string>

struct GLFWwindow;

namespace Uma_Engine
{
    class Window;
    class SystemManager;
    class EventSystem;
    class HybridInputSystem;
    class SceneManager;
    struct EngineConfig;

    /**
     * \brief Base application class that manages the engine lifecycle
     *
     * This class encapsulates:
     * - Window management
     * - System registration and lifecycle
     * - Main game loop
     * - Configuration loading
     *
     * Derived classes (EditorApplication, GameApplication) override
     * RegisterSystems() to customize which systems are active.
     */
    class Application
    {
    public:
        Application();
        virtual ~Application();

        /**
         * \brief Initializes the application
         * \return true if initialization succeeded, false otherwise
         */
        bool Init();

        /**
         * \brief Main application loop - runs until window closes
         */
        void Run();

        /**
         * \brief Shuts down the application and cleans up resources
         */
        void Shutdown();

        /**
         * \brief Gets the application window
         */
        Window* GetWindow() const { return mWindow.get(); }

        /**
         * \brief Gets the system manager
         */
        SystemManager* GetSystemManager() const { return mSystemManager.get(); }

        /**
         * \brief Gets the engine configuration
         */
        EngineConfig* GetConfig() const { return mConfig.get(); }

    protected:
        /**
         * \brief Override this to register application-specific systems
         * Called during Init() after base systems are set up
         */
        virtual void RegisterSystems() = 0;

        /**
         * \brief Override this to perform additional initialization after systems are registered
         * Called during Init() after RegisterSystems()
         */
        virtual void PostInit() = 0;

        /**
         * \brief Override this to handle application-specific update logic
         * Called every frame during Run()
         * \param deltaTime Time elapsed since last frame
         */
        virtual void Update(float deltaTime) {}

        /**
         * \brief Override this to perform application-specific shutdown logic
         * Called during Shutdown() before systems are shut down
         */
        virtual void PreShutdown() {}

        // Protected access to key systems for derived classes
        EventSystem* GetEventSystem() const { return mEventSystem; }
        HybridInputSystem* GetInputSystem() const { return mInputSystem; }
        SceneManager* GetSceneManager() const { return mSceneManager; }
        GLFWwindow* GetGLFWWindow() const;

    private:
        void InitializeDebugSystems();
        void LoadConfiguration();
        void MakeWindow();
        void RegisterCoreSystems();
        void MainLoop();

        std::unique_ptr<Window> mWindow;
        std::unique_ptr<SystemManager> mSystemManager;
        std::unique_ptr<EngineConfig> mConfig;

        // Cached pointers to frequently used systems (not owned)
        EventSystem* mEventSystem;
        HybridInputSystem* mInputSystem;
        SceneManager* mSceneManager;

        bool mInitialized;
    };
}
