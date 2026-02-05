/*!
\file   GameApplication.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Implements the GameApplication class, the runtime application layer used when
building and launching the engine in **Game Mode**. Unlike the editor version,
GameApplication provides a clean execution environment without any editor-only
systems or tools.

This class configures the engine for fullscreen gameplay, registers only
game-related scripts, subscribes to application events such as quit requests,
and loads the primary game scene. It ensures that the engine initializes
strictly with gameplay behavior, optimized for release/runtime execution.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "GameApplication.h"

#include "Core/SystemManager.h"
#include "Core/EngineConfig.h"
#include "Systems/SceneManager.h"
#include "WIP_Scripts/GameSceneScript.h"

// Events
#include "Events/ApplicationEvents.h"

namespace Uma_Engine
{
    /**
     * \brief Constructs the GameApplication. Initialization occurs in
     * PreInit(), RegisterSystems(), and PostInit().
     * GameApplication strips out all editor features and prepares the engine
     * for a clean runtime execution environment.
     */
    GameApplication::GameApplication()
        : Application()
    {
    }

    /**
     * \brief Registers systems needed for game builds.
     * Since the game mode does not require editor tools, no additional systems
     * are registered here. Core systems are registered in Application::RegisterCoreSystems().
     */
    void GameApplication::RegisterSystems()
    {
        // Game build has NO editor systems
        // All core systems are registered in Application::RegisterCoreSystems()
    }

    /**
     * \brief Executed before engine initialization. Explicitly disables editor mode.
     */
    void GameApplication::PreInit()
    {
        SetIsEditor(false);
    }

    /**
     * \brief Subscribes to application-level events required at runtime,
     * such as handling Quit requests when the user exits the game.
     */
    void GameApplication::SubscribeEvents()
    {
        EventSystem* eventSystem = GetEventSystem();

        eventSystem->Subscribe<Uma_Engine::ApplicationQuitRequest, Application>(
            [this](const ApplicationQuitRequest& e)
            {
                (void)e;
                GetWindow()->Close();
            });
    }

    /**
     * \brief Executed after initialization. Configures window mode for gameplay,
     * sets the render target, registers game-specific scripts, creates the
     * main scene, and loads it as the active scene.
     */
    void GameApplication::PostInit()
    {
        SubscribeEvents();

        // Render directly to the game window
        GetGraphics()->SetRenderTarget(Uma_Engine::RenderTarget::Window);

        // Fullscreen mode for game runtime
        GetWindow()->SetWindowMode(WindowMode::Fullscreen);

        // Get scene manager
        SceneManager* sceneManager = GetSceneManager();

        // Ensure engine is configured for runtime game behavior
        sceneManager->SetEditorMode(false);

        // Register game script only (no editor script)
        sceneManager->RegisterScript<GameSceneScript>("GameBehaviour");

        // Create and setup default game scene
        auto gameScene = sceneManager->CreateScene("main_menu.scn", "main_menu.scn");
        gameScene->g_EngineConfig = *GetConfig();

        // Attach game script to scene
        sceneManager->AttachScriptToScene("main_menu.scn", "GameBehaviour");

        // Load the scene
        sceneManager->LoadScene("main_menu.scn");
    }

    void GameApplication::PreUpdate(float dt)
    {
    }

    bool GameApplication::HandleInterruptions(float deltaTime)
    {
        // Game mode: Full pause when unfocused or minimized
        bool isFocused = glfwGetWindowAttrib(GetGLFWWindow(), GLFW_FOCUSED);
        bool isIconified = glfwGetWindowAttrib(GetGLFWWindow(), GLFW_ICONIFIED);

        if (!isFocused || isIconified)
        {
            // Lost focus or minimized (pause everything)
            if (mWasFocused)
            {
                if (GetSoundManager())
                {
                    GetSoundManager()->pauseAllSounds(true);
                }

                if (GetInputSystem())
                {
                    GetInputSystem()->ResetAllInput();
                }

                mWasFocused = false;
            }

            // Update sound system even when paused
            if (GetSoundManager())
            {
                GetSoundManager()->Update(deltaTime);
            }

            // Wait for events
            //glClear(GL_COLOR_BUFFER_BIT);
            glfwSwapBuffers(GetGLFWWindow());
            glfwWaitEventsTimeout(0.1);

            // Skip system updates and buffer swap
            return false;
        }

        // Regained focus
        if (!mWasFocused)
        {
            if (GetSoundManager())
            {
                GetSoundManager()->pauseAllSounds(false);
            }

            mWasFocused = true;
        }

        // Continue normal frame processing
        return true;
    }


    void GameApplication::PostUpdate(float dt)
    {
        (void)dt;

        if (Uma_Engine::HybridInputSystem::KeyPressed(GLFW_KEY_ESCAPE))
        {
            GamePause() = !GamePause();
            GetEventSystem()->Emit<ApplicationGamePauseRequest>(GamePause());
        }
    }
}
