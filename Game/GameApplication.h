/*!
\file   GameApplication.h
\brief  Game application for the final game build (no editor)

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
        void PostInit() override;
    };
}
