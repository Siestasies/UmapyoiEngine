/*!
\file    SceneManager.cpp
\par     Project: GAM200
\par     Course: CSD2401
\par     Section A
\par     Software Engineering Project 3

\author Shahir Rasid (Everything else)
\par     E-mail: b.muhammadshahir@digipen.edu
\par     DigiPen login: b.muhammadshahir

\co-author Javier Chua Dong Qing (EditorCamera implementation)
\par     E-mail: javierdongqing.chua@digipen.edu
\par     DigiPen login: javierdongqing.chua

\brief
Implements the SceneManager class. This file contains the logic for
initializing the manager, updating scenes based on play mode, managing
the editor camera, handling scene (load/unload/save) operations,
and managing the scripts.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "SceneManager.h"

#include "Core/EventSystem.h"
#include "../Events/IMGUIEvents.h"
#include "../Events/EditorEvents.h"
#include "../Events/SceneEvents.h"

#include <algorithm>

namespace Uma_Engine
{
    // ISYSTEM OVERRIDES
    void SceneManager::Init()
    {
        std::cout << "SceneManager: Initialized" << std::endl;
        // Scripts will be registered externally
        // Scenes will be created on-demand

        // Initialize playMode based on editor mode flag
        playMode = m_IsEditorMode ? PLAYMODE::PM_STOP : PLAYMODE::PM_PLAY;

        // Initialize editor camera
        m_EditorCamera.SetPosition(Vec2(0.0f, 0.0f));
        m_EditorCamera.SetZoom(10.0f);
        m_EditorCamera.SetPanSpeed(500.0f);
        m_EditorCamera.SetZoomSpeed(1.0f);
        m_EditorCamera.SetZoomLimits(0.1f, 20.0f);
        m_EditorCamera.SetActive(false);
        m_UseEditorCamera = false;

        // sub to events
        pEventSystem = pSystemManager->GetSystem<EventSystem>();

        pEventSystem->Subscribe<PlaySceneRequest, SceneManager>(
            [&](const PlaySceneRequest& e) {
                (void)e;
                playMode = PLAYMODE::PM_PLAY;
            }
        );

        pEventSystem->Subscribe<PauseSceneRequest, SceneManager>(
            [&](const PauseSceneRequest& e) {
                (void)e;
                playMode = PLAYMODE::PM_PAUSE;
            }
        );

        pEventSystem->Subscribe<StopSceneRequest, SceneManager>(
            [&](const StopSceneRequest& e) {
                (void)e;
                playMode = PLAYMODE::PM_STOP;
            }
        );

        pEventSystem->Subscribe<CreateNewSceneRequest, SceneManager>(
            [this](const CreateNewSceneRequest& e) {
                (void)e;
                CreateNewScene(); 
            }
        );

        pEventSystem->Subscribe<DeleteCurrSceneRequest, SceneManager>(
            [this](const DeleteCurrSceneRequest& e) {
                RemoveScene(e.name);
            }
        );

        pEventSystem->Subscribe<SaveSceneRequest, SceneManager>(
            [this](const SaveSceneRequest& e) {
                SaveScene(e.name);
            }
        );

        pEventSystem->Subscribe<SaveCurrSceneRequest, SceneManager>(
            [this](const SaveCurrSceneRequest& e) {
                (void)e;
                SaveScene(m_ActiveScene->GetName());
            }
        );

        pEventSystem->Subscribe<LoadSceneRequestEvent, SceneManager>(
            [this](const LoadSceneRequestEvent& e) {
                LoadScene(e.name, false);
                m_UseEditorCamera = false;
                playMode = PLAYMODE::PM_STOP;
            }
        );

        pEventSystem->Subscribe<PrefabSceneRequestEvent, SceneManager>(
            [this](const PrefabSceneRequestEvent& e) {
                LoadScene(e.scene_name, false);
                //SetActiveScene(e.scene_name);
                m_UseEditorCamera = false;
                playMode = PLAYMODE::PM_STOP;
            }
        );

        pEventSystem->Subscribe<UpdateMouseOverUIEvent, SceneManager>(
            [this](const UpdateMouseOverUIEvent& e) {
                m_isMouseOverUI = e.isFocus;
            }
        );
    }

    void SceneManager::Update(float dt)
    {
        //std::cout << "use editor cam : " << (m_UseEditorCamera ? "yes" : "no") << std::endl;
        // Update loading scenes first
        UpdateLoadingScenes();

        // Update active scene
        if (m_ActiveScene && m_ActiveScene->IsLoaded())
        {
            // Auto-switch camera based on play mode
            if (playMode == PLAYMODE::PM_STOP || playMode == PLAYMODE::PM_PAUSE)
            {
                // Use editor camera when stopped or paused
                if (!m_UseEditorCamera)
                {
                    m_UseEditorCamera = true;
                    m_EditorCamera.SetActive(true);

                    // Tell RenderingSystem to not override camera
                    if (m_ActiveScene->m_RenderingSystem)
                    {
                        m_ActiveScene->m_RenderingSystem->SetUpdateCamera(false);
                    }

                    std::cout << "Switched to editor camera" << std::endl;
                }
            }
            else
            {
                // Use game camera when playing
                if (m_UseEditorCamera)
                {
                    m_UseEditorCamera = false;
                    m_EditorCamera.SetActive(false);

                    // Tell RenderingSystem to update camera
                    if (m_ActiveScene->m_RenderingSystem)
                    {
                        m_ActiveScene->m_RenderingSystem->SetUpdateCamera(true);
                    }

                    std::cout << "Switched to game camera" << std::endl;
                }
            }

            // Update editor camera if active
            if (m_Scenes.size() > 0)
            {
                if (m_UseEditorCamera && !m_isMouseOverUI)
                {
                    // Get input and graphics from scene
                    auto* input = m_ActiveScene->GetInputSystem();
                    auto* graphics = m_ActiveScene->GetGraphics();

                    if (input)
                    {
                        m_EditorCamera.Update(input, dt);

                        // Update graphics system with editor camera
                        if (graphics)
                        {
                            graphics->SetCamInfo(m_EditorCamera.GetPosition(), m_EditorCamera.GetZoom());
                        }
                    }
                }
            }

            if (playMode == PLAYMODE::PM_PLAY)
            {
                m_ActiveScene->Update(dt);
            }
            else if (playMode == PLAYMODE::PM_PAUSE)
            {
                m_ActiveScene->Update(0.f);
            }
            else
            {
                //std::filesystem::remove("Assets/Scenes/" + m_ActiveScene->GetName());

                // things that need to be constantly updated no matter what
                // shouldn't affect game stop?
                m_ActiveScene->UpdateSelective(0.f);
            }
        }
   
        // Update all loaded scenes if using additive loading
        for (auto& scene : m_LoadedScenes)
        {
            if (scene != m_ActiveScene && scene->IsLoaded())
            {
                scene->Update(dt);
            }
        }

        // Clean up unloaded scenes
        RemoveUnloadedScenes();
    }

    void SceneManager::Shutdown()
    {
        std::cout << "SceneManager: Shutting down" << std::endl;

        UnloadAllScenes();
        m_Scenes.clear();
        m_ScriptFactories.clear();

        pEventSystem->UnsubscribeSystem<SceneManager>();
    }

    // SCENE MANAGEMENT STUFF
    void SceneManager::CreateNewScene()
    {
        std::string filename;
        int sceneNo = 0;
        do {
            filename = "Scene" + std::to_string(sceneNo) + ".scn";
            ++sceneNo;
        } while (std::filesystem::exists("Assets/Scenes/" + filename));

        CreateScene(filename, filename);
        AttachScriptToScene(filename, "EditorBehaviour");
        LoadScene(filename);
    }

    std::shared_ptr<Scene> SceneManager::CreateScene(const std::string& name, const std::string& filepath)
    {
        // Check if scene already exists
        if (m_Scenes.find(name) != m_Scenes.end())
        {
            std::cout << "Scene '" << name << "' already exists!" << std::endl;
            return m_Scenes[name];
        }

        // Create new scene
        auto scene = std::make_shared<Scene>(name, Uma_FilePath::SCENES_DIR + filepath, pSystemManager);
        m_Scenes[name] = scene;

        std::cout << "Scene '" << name << "' created" << std::endl;
        return scene;
    }

    std::shared_ptr<Scene> SceneManager::LoadScene(const std::string& name, bool additive)
    {
        if (isUnloading)
        {
            std::cout << "A scene is unloading... try again later.\n";
            return nullptr;
        }

        // Check if scene exists
        if (!HasScene(name))
        {
            //std::cout << "Scene '" << name << "' does not exist! Create it first." << std::endl;
            //return nullptr;

            // create a scene based on the scene name and try to load it
            CreateScene(name, name);
            AttachScriptToScene(name, "EditorBehaviour");
            //LoadScene(name);
        }

        // Check if already loaded
        if (IsSceneLoaded(name))
        {
            std::cout << "Scene '" << name << "' is already loaded!" << std::endl;
            return m_Scenes[name];
        }

        // call for stop mode

        auto scene = m_Scenes[name];

        // Unload previous active scene if not additive
        if (!additive && m_ActiveScene)
        {
            UnloadScene(m_ActiveScene->GetName());
        }


        // Load the scene synchronously
        scene->Load();

        // Add to loaded scenes
        m_LoadedScenes.push_back(scene);

        // Set as active if not additive or no active scene
        if (!additive || !m_ActiveScene)
        {
            m_ActiveScene = scene;
        }

        // passing message using event system
        UpdateIMGUIWindow();

        // set coordinator for gizmos 
        pEventSystem->Emit<SceneLoadedEvent>(name);

        std::cout << "Scene '" << name << "' loaded" << (additive ? " additively" : "") << std::endl;
        return scene;
    }

    void SceneManager::LoadSceneAsync(const std::string& name, bool additive)
    {
        // Check if scene exists
        if (!HasScene(name))
        {
            std::cout << "Scene '" << name << "' does not exist! Create it first." << std::endl;
            return;
        }

        // Check if already loaded or loading
        if (IsSceneLoaded(name))
        {
            std::cout << "Scene '" << name << "' is already loaded!" << std::endl;
            return;
        }

        if (IsSceneLoading(name))
        {
            std::cout << "Scene '" << name << "' is already loading!" << std::endl;
            return;
        }

        auto scene = m_Scenes[name];

        // Unload previous active scene if not additive
        if (!additive && m_ActiveScene)
        {
            UnloadScene(m_ActiveScene->GetName());
            m_ActiveScene = nullptr; // Will be set when loading completes
        }

        // Start async load
        scene->LoadAsync();
        m_LoadingScenes.push_back(scene);

        std::cout << "Scene '" << name << "' started loading asynchronously..." << std::endl;
    }

    void SceneManager::UnloadScene(const std::string& name)
    {
        isUnloading = true;

        pEventSystem->Emit<SceneUnloadedEvent>(name);

        playMode = PLAYMODE::PM_STOP;
        //pSystemManager->GetSystem<EditorSystem>()->SetCoordinator(nullptr);

        if (!HasScene(name))
        {
            std::cout << "Scene '" << name << "' does not exist!" << std::endl;
            return;
        }

        auto scene = m_Scenes[name];

        // Unload the scene
        scene->Unload();

        // Clear active scene if it's being unloaded
        if (m_ActiveScene == scene)
        {
            m_ActiveScene = nullptr;
        }

        isUnloading = false;
        std::cout << "Scene '" << name << "' unloaded" << std::endl;
    }

    void SceneManager::SaveScene(const std::string& name)
    {
        if (!HasScene(name))
        {
            std::cout << "Scene '" << name << "' does not exist!" << std::endl;
            return;
        }

        auto scene = m_Scenes[name];

        scene->gGameSerializer.save(scene->GetFilePath());
    }

    void SceneManager::RemoveScene(const std::string& name)
    {
        if (!HasScene(name))
        {
            std::cout << "Scene '" << name << "' does not exist!" << std::endl;
            return;
        }

        /*if (m_Scenes.size() == 0)
        {
            std::cout << "Cannot remove because this is the last scene!\n";
            return;
        }*/

        // Don't remove active scene
        //if (m_ActiveScene && m_ActiveScene->GetName() == name)
        //{
        //    std::cout << "Cannot remove active scene '" << name << "'. Switch scenes first." << std::endl;
        //    return;
        //}

        // Unload if loaded
        if (IsSceneLoaded(name))
        {
            UnloadScene(name);
        }

        // Remove from map
        m_Scenes.erase(name);
        std::cout << "Scene '" << name << "' removed" << std::endl;
        if (m_Scenes.size() > 0)
            LoadScene(m_Scenes.begin()->second->GetName());
        UpdateIMGUIWindow();
    }



    void SceneManager::UnloadAllScenes()
    {
        // texh debt i dk why i dont have to send event here
        
        // Unload all loaded scenes
        for (auto& scene : m_LoadedScenes)
        {
            scene->Unload();
        }
        m_LoadedScenes.clear();

        // Wait for loading scenes to finish and unload
        for (auto& scene : m_LoadingScenes)
        {
            scene->Unload();
        }
        m_LoadingScenes.clear();

        m_ActiveScene = nullptr;
        std::cout << "All scenes unloaded" << std::endl;
    }

    void SceneManager::SetActiveScene(const std::string& name)
    {
        if (!HasScene(name))
        {
            std::cout << "Scene '" << name << "' does not exist!" << std::endl;
            return;
        }

        if (!IsSceneLoaded(name))
        {
            std::cout << "Scene '" << name << "' is not loaded! Load it first." << std::endl;
            return;
        }

        m_ActiveScene = m_Scenes[name];
        std::cout << "Active scene set to: " << name << std::endl;
    }

    std::shared_ptr<Scene> SceneManager::GetScene(const std::string& name)
    {
        auto it = m_Scenes.find(name);
        return (it != m_Scenes.end()) ? it->second : nullptr;
    }

    // SCRIPT STUFF
    std::shared_ptr<SceneScript> SceneManager::CreateScript(const std::string& scriptName)
    {
        auto it = m_ScriptFactories.find(scriptName);
        if (it == m_ScriptFactories.end())
        {
            std::cout << "Script '" << scriptName << "' is not registered!" << std::endl;
            return nullptr;
        }

        return it->second(); // Call factory function
    }

    void SceneManager::AttachScriptToScene(const std::string& sceneName, const std::string& scriptName)
    {
        auto scene = GetScene(sceneName);
        if (!scene)
        {
            std::cout << "Scene '" << sceneName << "' not found!" << std::endl;
            return;
        }

        auto script = CreateScript(scriptName);
        if (!script)
        {
            std::cout << "Failed to create script '" << scriptName << "'" << std::endl;
            return;
        }

        scene->AttachScript(script);
    }

    void SceneManager::DetachScriptFromScene(const std::string& sceneName, const std::string& scriptName)
    {
        auto scene = GetScene(sceneName);
        if (!scene)
        {
            std::cout << "Scene '" << sceneName << "' not found!" << std::endl;
            return;
        }

        scene->DetachScript(scriptName);
    }

    std::vector<std::string> SceneManager::GetRegisteredScriptNames() const
    {
        std::vector<std::string> names;
        for (const auto& [name, factory] : m_ScriptFactories)
        {
            names.push_back(name);
        }
        return names;
    }

    // SCENE STUFF
    bool SceneManager::HasScene(const std::string& name) const
    {
        return m_Scenes.find(name) != m_Scenes.end();
    }

    bool SceneManager::IsSceneLoaded(const std::string& name) const
    {
        auto it = std::find_if(m_LoadedScenes.begin(), m_LoadedScenes.end(),
            [&name](const std::shared_ptr<Scene>& scene) {
                return scene->GetName() == name;
            });
        return it != m_LoadedScenes.end() && (*it)->IsLoaded();
    }

    bool SceneManager::IsSceneLoading(const std::string& name) const
    {
        auto it = std::find_if(m_LoadingScenes.begin(), m_LoadingScenes.end(),
            [&name](const std::shared_ptr<Scene>& scene) {
                return scene->GetName() == name;
            });
        return it != m_LoadingScenes.end();
    }

    SceneState SceneManager::GetSceneState(const std::string& name) const
    {
        if (!HasScene(name))
            return SceneState::SCENE_UNLOADED;

        return m_Scenes.at(name)->GetState();
    }

    float SceneManager::GetSceneLoadProgress(const std::string& name) const
    {
        if (!HasScene(name))
            return 0.0f;

        return m_Scenes.at(name)->GetLoadProgress();
    }

    std::vector<std::string> SceneManager::GetAllSceneNames() const
    {
        std::vector<std::string> names;
        for (const auto& [name, scene] : m_Scenes)
        {
            names.push_back(name);
        }
        return names;
    }

    std::vector<std::string> SceneManager::GetLoadedSceneNames() const
    {
        std::vector<std::string> names;
        for (const auto& scene : m_LoadedScenes)
        {
            names.push_back(scene->GetName());
        }
        return names;
    }

    std::string SceneManager::GetActiveSceneName() const
    {
        return m_ActiveScene ? m_ActiveScene->GetName() : "";
    }

    Uma_ECS::Coordinator* SceneManager::GetActiveSceneCoordinator() const
    {
        return &m_ActiveScene->GetCoordinator();
    }

    void SceneManager::UpdateLoadingScenes()
    {
        auto it = m_LoadingScenes.begin();
        while (it != m_LoadingScenes.end())
        {
            auto& scene = *it;

            // Check if scene finished loading
            if (scene->GetState() == SceneState::SCENE_RUNNING)
            {
                // Move to loaded scenes
                m_LoadedScenes.push_back(scene);

                // Set as active if no active scene
                if (!m_ActiveScene)
                {
                    m_ActiveScene = scene;
                    std::cout << "Scene '" << scene->GetName() << "' loaded and set as active" << std::endl;
                }
                else
                {
                    std::cout << "Scene '" << scene->GetName() << "' loaded additively" << std::endl;
                }

                // Remove from loading queue
                it = m_LoadingScenes.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    void SceneManager::RemoveUnloadedScenes()
    {
        m_LoadedScenes.erase(
            std::remove_if(m_LoadedScenes.begin(), m_LoadedScenes.end(),
                [](const std::shared_ptr<Scene>& scene) {
                    return scene->GetState() == SceneState::SCENE_UNLOADED;
                }),
            m_LoadedScenes.end()
        );
    }

    void SceneManager::UpdateIMGUIWindow()
    {
        std::vector<std::string> vec = GetAllSceneNames();
        std::vector<std::string> vec2;
        int no = 0;
        int index = 0;
        for (const auto& pair : m_Scenes)
        {
            //vec.push_back(pair.first);
            vec2.push_back(pair.second->GetFilePath());
            if (pair.second == m_ActiveScene)
            {
                index = no;
            }
            ++no;
        }
        EventSystem* eventSystem = pSystemManager->GetSystem<EventSystem>();
        eventSystem->Emit<SceneInfoRequest>(vec, vec2, index);
    }

    void SceneManager::SetEditorMode(bool isEditor)
    {
        m_IsEditorMode = isEditor;
        playMode = isEditor ? PLAYMODE::PM_STOP : PLAYMODE::PM_PLAY;
    }
}