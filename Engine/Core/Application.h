/*!
\file   Application.h
\brief  Base application class that encapsulates the engine's core systems.
        Provides a framework for derived applications (Editor, Game) to run.

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

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
    class SoundManager;
    class PlayFabManager;
    class Graphics;
    class EngineConfig;
    class PlayFabConfig;

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

        //PlayFabConfig* GetPlayFabConfig() const { return mPlayFabConfig.get(); }

        static bool& GetGamePause() { return mGamePause; }
        static bool& GetCutsceneActive() { return mCutsceneActive; }

        static float GetFps() { return mFps; }

    protected:
        /**
         * \brief Override this to register application-specific systems
         * Called during Init() after base systems are set up
         */
        virtual void RegisterSystems() = 0;

        /**
         * \brief Override this to perform additional initialization before systems are registered
         * Called during Init() after RegisterSystems()
         */
        virtual void PreInit() = 0;

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
        virtual void PreUpdate(float deltaTime) { (void)deltaTime; }

        /**
         * \brief Override this to handle application-specific update logic
         * Called every frame during Run()
         * \param deltaTime Time elapsed since last frame
         */
        virtual void PostUpdate(float deltaTime) { (void)deltaTime; }

        /**
         * \brief Override this to perform application-specific shutdown logic
         * Called during Shutdown() before systems are shut down
         */
        virtual void PreShutdown() {}


        // Protected access to key systems for derived classes
        EventSystem* GetEventSystem() const { return mEventSystem; }
        HybridInputSystem* GetInputSystem() const { return mInputSystem; }
        SceneManager* GetSceneManager() const { return mSceneManager; }
        Graphics* GetGraphics() const { return mGraphics; }
        SoundManager* GetSoundManager() const { return mSoundManager; }
        PlayFabManager* GetPlayFabManager() const { return mPlayFabManager; }
        GLFWwindow* GetGLFWWindow() const;
        bool& GamePause() { return mGamePause; }

        // helpers
        void SetIsEditor(bool isEditor) { mIsEditor = isEditor; }
        virtual bool HandleInterruptions(float deltaTime) = 0;

    private:
        void InitializeDebugSystems();
        void LoadConfiguration();
        void MakeWindow();
        void RegisterCoreSystems();
        void MainLoop();
        void SubscribeToEvents();

        bool mInitialized;
        bool mIsEditor;
        inline static bool mGamePause = false;
        inline static bool mCutsceneActive = false;
        inline static float mFps = 0;

    protected:

        std::unique_ptr<Window> mWindow;
        std::unique_ptr<SystemManager> mSystemManager;
        std::unique_ptr<EngineConfig> mConfig;
        //std::unique_ptr<PlayFabConfig> mPlayFabConfig;

        // Cached pointers to frequently used systems (not owned)
        EventSystem* mEventSystem;
        HybridInputSystem* mInputSystem;
        SoundManager* mSoundManager;
        Graphics* mGraphics;
        SceneManager* mSceneManager;
        PlayFabManager* mPlayFabManager;
    };
}
