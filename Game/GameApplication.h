/*!
\file   GameApplication.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

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
        GameApplication();
        ~GameApplication() override = default;

    protected:
        void RegisterSystems() override;
        void PreInit() override;
        void PostInit() override;
        void PreUpdate(float dt) override;
        void PostUpdate(float dt) override;
        void SubscribeEvents();
        bool HandleInterruptions(float deltaTime) override;

    private:

        void PlayFabConfiguration();

        bool mWasFocused = true;
    };
}
