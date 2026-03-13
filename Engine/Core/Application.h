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
        /*!
        \brief Constructs the Application with default state.
        */
        Application();

        /*!
        \brief Virtual destructor for safe polymorphic cleanup.
        */
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

        /*!
        \brief Gets a reference to the game pause flag.
        \return Reference to the static game pause boolean.
        */
        static bool& GetGamePause() { return mGamePause; }

        /*!
        \brief Gets a reference to the cutscene active flag.
        \return Reference to the static cutscene active boolean.
        */
        static bool& GetCutsceneActive() { return mCutsceneActive; }

        /*!
        \brief Gets the current frames per second.
        \return The current FPS value.
        */
        static float GetFps() { return mFps; }

        static void StartTimer(bool startNow)
        {
            playTime = (startNow == true) ? 0.f : playTime;
            startPlayTimeTimer = startNow;
        }

        static float GetPlayTime() { return playTime; }

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

        /*!
        \brief Gets the event system.
        \return Pointer to the EventSystem instance.
        */
        EventSystem* GetEventSystem() const { return mEventSystem; }

        /*!
        \brief Gets the hybrid input system.
        \return Pointer to the HybridInputSystem instance.
        */
        HybridInputSystem* GetInputSystem() const { return mInputSystem; }

        /*!
        \brief Gets the scene manager.
        \return Pointer to the SceneManager instance.
        */
        SceneManager* GetSceneManager() const { return mSceneManager; }

        /*!
        \brief Gets the graphics system.
        \return Pointer to the Graphics instance.
        */
        Graphics* GetGraphics() const { return mGraphics; }

        /*!
        \brief Gets the sound manager.
        \return Pointer to the SoundManager instance.
        */
        SoundManager* GetSoundManager() const { return mSoundManager; }

        /*!
        \brief Gets the PlayFab manager.
        \return Pointer to the PlayFabManager instance.
        */
        PlayFabManager* GetPlayFabManager() const { return mPlayFabManager; }

        /*!
        \brief Gets the underlying GLFW window handle.
        \return Pointer to the GLFWwindow.
        */
        GLFWwindow* GetGLFWWindow() const;

        /*!
        \brief Gets a reference to the game pause flag.
        \return Reference to the static game pause boolean.
        */
        bool& GamePause() { return mGamePause; }

        // helpers

        /*!
        \brief Sets whether this application instance is running as an editor.
        \param isEditor True if running as editor, false otherwise.
        */
        void SetIsEditor(bool isEditor) { mIsEditor = isEditor; }

        /*!
        \brief Handles application interruptions such as window focus loss.
        \param deltaTime Time elapsed since last frame.
        \return True if an interruption was handled and the frame should be skipped.
        */
        virtual bool HandleInterruptions(float deltaTime) = 0;

    private:
        /*!
        \brief Initializes debug-only systems such as memory tracking.
        */
        void InitializeDebugSystems();

        /*!
        \brief Loads engine configuration from the config files.
        */
        void LoadConfiguration();

        /*!
        \brief Creates and initializes the application window.
        */
        void MakeWindow();

        /*!
        \brief Registers the core engine systems required by all application types.
        */
        void RegisterCoreSystems();

        /*!
        \brief Executes the main game loop, processing updates and rendering each frame.
        */
        void MainLoop();

        /*!
        \brief Subscribes the application to relevant engine events.
        */
        void SubscribeToEvents();

        bool mInitialized;
        bool mIsEditor;
        inline static bool mGamePause = false;
        inline static bool mCutsceneActive = false;
        inline static float mFps = 0;
        inline static float playTime = 0.f;
        inline static bool startPlayTimeTimer = false;

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
