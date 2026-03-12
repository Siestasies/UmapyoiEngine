/*!
\file   GameApplication.h
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Declares the GameApplication class, the stripped-down application layer used
for the final game build. Unlike EditorApplication, this version excludes all
editor-related systems and tools, providing only the runtime systems required
for the shipped game.

GameApplication handles core system initialization, game-mode configuration,
scene loading, event subscription, and the registration of gameplay-related
scripts such as GameSceneScript. It ensures that the engine runs in pure
game mode (no editor UI, no ImGui, no editor scene management).

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "Core/Application.h"

namespace Uma_Engine
{
    /**
     * \brief Game-only application without editor functionality
     *
     * This application includes:
     * - Core engine systems only
     * - GameSceneScript for game logic
     * - NO EditorSystem, NO ImguiManager
     *
     * This is the build used for the final game installer.
     */
    class GameApplication : public Application
    {
    public:
        /*!
        \brief Constructs the game application and configures game-mode settings.
        */
        GameApplication();
        ~GameApplication() override = default;

    protected:
        /*!
        \brief Registers all runtime systems required for the game build.
        */
        void RegisterSystems() override;

        /*!
        \brief Performs pre-initialization setup before systems are initialized.
        */
        void PreInit() override;

        /*!
        \brief Performs post-initialization setup after all systems are initialized.
        */
        void PostInit() override;

        /*!
        \brief Called before the main update loop each frame.
        \param dt Delta time in seconds since the last frame.
        */
        void PreUpdate(float dt) override;

        /*!
        \brief Called after the main update loop each frame.
        \param dt Delta time in seconds since the last frame.
        */
        void PostUpdate(float dt) override;

        /*!
        \brief Subscribes to engine events such as window focus changes.
        */
        void SubscribeEvents();

        /*!
        \brief Handles application interruptions such as window losing focus.
        \param deltaTime Delta time in seconds since the last frame.
        \return True if the frame should be skipped due to an interruption.
        */
        bool HandleInterruptions(float deltaTime) override;

    private:

        /*!
        \brief Loads and applies PlayFab configuration from the config file.
        */
        void PlayFabConfiguration();

        bool mWasFocused = true;
    };
}
