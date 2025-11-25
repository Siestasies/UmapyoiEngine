/*!
\file   EditorApplication.cpp
\brief  Implementation of EditorApplication

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "EditorApplication.h"

#include "Core/SystemManager.h"
#include "Core/EngineConfig.h"
#include "Systems/SceneManager.h"
#include "Editor/Core/EditorSystem.h"
#include "Scripts/ImguiManager.h"
#include "Scripts/EditorScript.h"
#include "WIP_Scripts/GameSceneScript.h"

namespace Uma_Engine
{
    EditorApplication::EditorApplication()
        : Application()
        , mEditorSystem(nullptr)
        , mImguiManager(nullptr)
    {
    }

    void EditorApplication::RegisterSystems()
    {
        // Register editor-specific systems
        mEditorSystem = GetSystemManager()->RegisterSystem<EditorSystem>();
        mImguiManager = GetSystemManager()->RegisterSystem<ImguiManager>();
    }

    void EditorApplication::PreInit()
    {
        SetIsEditor(true);
    }

    void EditorApplication::PostInit()
    {
        // Get scene manager
        SceneManager* sceneManager = GetSceneManager();

        // Configure for editor mode
        sceneManager->SetEditorMode(true);

        // Register scripts
        sceneManager->RegisterScript<GameSceneScript>("GameBehaviour");
        sceneManager->RegisterScript<EditorScript>("EditorBehaviour");

        // Create and setup default editor scene
        auto editorScene = sceneManager->CreateScene("test_default.scn", "test_default.scn");
        editorScene->g_EngineConfig = *GetConfig();

        // Attach scripts to scene
        sceneManager->AttachScriptToScene("test_default.scn", "GameBehaviour");
        sceneManager->AttachScriptToScene("test_default.scn", "EditorBehaviour");

        // Load the scene
        sceneManager->LoadScene("test_default.scn");
    }
}
