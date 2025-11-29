/*!
\file   EditorApplication.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Implements the EditorApplication class, the specialized application layer
used when running the engine in Editor Mode. This class extends the base
Application system by registering editor-specific systems, setting up the
editor scene, and subscribing to editor/application events.

EditorApplication configures the SceneManager for editor behavior, registers
editor scripts, initializes the ImGui manager, and loads a default editor
scene used for development workflows.

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

// Events
#include "Events/ApplicationEvents.h"

namespace Uma_Engine
{
    /**
     * \brief Constructs the EditorApplication and initializes pointers.
     * The actual initialization work occurs in PreInit(), RegisterSystems(), and PostInit().
     */
    EditorApplication::EditorApplication()
        : Application()
        , mEditorSystem(nullptr)
        , mImguiManager(nullptr)
    {
    }

    /**
     * \brief Registers all systems required specifically for the editor.
     * This includes EditorSystem and ImguiManager, which provide UI and tool functionality.
     */
    void EditorApplication::RegisterSystems()
    {
        mEditorSystem = GetSystemManager()->RegisterSystem<EditorSystem>();
        mImguiManager = GetSystemManager()->RegisterSystem<ImguiManager>();
    }

    /**
     * \brief Executed before engine initialization. Enables editor mode.
     */
    void EditorApplication::PreInit()
    {
        SetIsEditor(true);
    }

    /**
     * \brief Subscribes editor-related events such as quit requests.
     */
    void EditorApplication::SubscribeEvents()
    {
        EventSystem* eventSystem = GetEventSystem();

        eventSystem->Subscribe<Uma_Engine::ApplicationQuitRequest, Application>(
            [this](const ApplicationQuitRequest&)
            {
                GetWindow()->Close();
            });
    }

    /**
     * \brief Executed after engine initialization.
     * Sets up the editor environment, loads the default editor scene,
     * and registers editor/game scripts to the SceneManager.
     */
    void EditorApplication::PostInit()
    {
        SubscribeEvents();

        SceneManager* sceneManager = GetSceneManager();
        sceneManager->SetEditorMode(true);

        // Register script classes for use in the editor scene
        sceneManager->RegisterScript<GameSceneScript>("GameBehaviour");
        sceneManager->RegisterScript<EditorScript>("EditorBehaviour");

        // Create the editor scene and configure it
        auto editorScene = sceneManager->CreateScene("test_combat.scn", "test_combat.scn");
        editorScene->g_EngineConfig = *GetConfig();

        sceneManager->AttachScriptToScene("test_combat.scn", "GameBehaviour");
        sceneManager->AttachScriptToScene("test_combat.scn", "EditorBehaviour");

        // Load the default scene
        sceneManager->LoadScene("test_combat.scn");
    }

    void EditorApplication::Update(float dt)
    {
        (void)dt;
        if (Uma_Engine::HybridInputSystem::KeyPressed(GLFW_KEY_ESCAPE))
        {
            GamePause() = !GamePause();
            GetEventSystem()->Emit<ApplicationGamePauseRequest>(GamePause());
        }
    }
}
