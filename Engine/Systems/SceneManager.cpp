#include "SceneManager.h"
#include <algorithm>

namespace Uma_Engine
{
    // ISYSTEM OVERRIDES
    void SceneManager::Init()
    {
        std::cout << "SceneManager: Initialized" << std::endl;
        // Scripts will be registered externally
        // Scenes will be created on-demand
    }

    void SceneManager::Update(float dt)
    {
        // Update loading scenes first
        UpdateLoadingScenes();

        // Update active scene
        if (m_ActiveScene && m_ActiveScene->IsLoaded())
        {
            if (imHandler->IsPlaying())
            {
                m_ActiveScene->Update(dt);
            }
            else if (imHandler->IsPaused())
            {
                m_ActiveScene->Update(0.f);
            }
            else
            {
                // things that need to be constantly updated no matter what
                // shoudlnt affect game stop?
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
    }

    // SCENE MANAGEMENT STUFF
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
        // Check if scene exists
        if (!HasScene(name))
        {
            std::cout << "Scene '" << name << "' does not exist! Create it first." << std::endl;
            return nullptr;
        }

        // Check if already loaded
        if (IsSceneLoaded(name))
        {
            std::cout << "Scene '" << name << "' is already loaded!" << std::endl;
            return m_Scenes[name];
        }

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

        std::cout << "Scene '" << name << "' unloaded" << std::endl;
    }

    void SceneManager::RemoveScene(const std::string& name)
    {
        if (!HasScene(name))
        {
            std::cout << "Scene '" << name << "' does not exist!" << std::endl;
            return;
        }

        // Don't remove active scene
        if (m_ActiveScene && m_ActiveScene->GetName() == name)
        {
            std::cout << "Cannot remove active scene '" << name << "'. Switch scenes first." << std::endl;
            return;
        }

        // Unload if loaded
        if (IsSceneLoaded(name))
        {
            UnloadScene(name);
        }

        // Remove from map
        m_Scenes.erase(name);
        std::cout << "Scene '" << name << "' removed" << std::endl;
    }

    void SceneManager::UnloadAllScenes()
    {
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
}