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

// Events
#include "Events/ApplicationEvents.h"

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

    void GameApplication::SubscribeEvents()
    {
        EventSystem* eventSystem = GetEventSystem();

        eventSystem->Subscribe<Uma_Engine::ApplicationQuitRequest, Application>([this](const ApplicationQuitRequest& e)
           {
               GetWindow()->Close();
           });
    }

    void GameApplication::PostInit()
    {
        SubscribeEvents();

        GetGraphics()->SetRenderTarget(Uma_Engine::RenderTarget::Window);

        GetWindow()->SetWindowMode(WindowMode::Fullscreen);

        // Get scene manager
        SceneManager* sceneManager = GetSceneManager();

        // Configure for game mode
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
}
