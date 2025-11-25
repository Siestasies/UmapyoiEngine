/*!
\file   GameApplication.cpp
\brief  Implementation of GameApplication

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "GameApplication.h"

#include "Core/SystemManager.h"
#include "Core/EngineConfig.h"
#include "Systems/SceneManager.h"
#include "WIP_Scripts/GameSceneScript.h"

namespace Uma_Engine
{
    GameApplication::GameApplication()
        : Application()
    {
    }

    void GameApplication::RegisterSystems()
    {
        // Game build has NO editor systems
        // All core systems are registered in Application::RegisterCoreSystems()
    }

    void GameApplication::PreInit()
    {
        SetIsEditor(false);
    }

    void GameApplication::PostInit()
    {
        GetGraphics()->SetRenderTarget(Uma_Engine::RenderTarget::Window);

        // Get scene manager
        SceneManager* sceneManager = GetSceneManager();

        // Configure for game mode
        sceneManager->SetEditorMode(false);

        // Register game script only (no editor script)
        sceneManager->RegisterScript<GameSceneScript>("GameBehaviour");

        // Create and setup default game scene
        auto gameScene = sceneManager->CreateScene("test_default.scn", "test_default.scn");
        gameScene->g_EngineConfig = *GetConfig();

        // Attach game script to scene
        sceneManager->AttachScriptToScene("test_default.scn", "GameBehaviour");

        // Load the scene
        sceneManager->LoadScene("test_default.scn");
    }
}
