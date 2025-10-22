#include "SceneManager.h"
#include "Core/FilePaths.h"
#include <filesystem>

namespace Uma_Engine
{
    void SceneManager::Init()
    {
        // Scan for available scene files
        std::filesystem::path scenePath(Uma_FilePath::SCENES_DIR);
        if (std::filesystem::exists(scenePath))
        {
            for (const auto& entry : std::filesystem::directory_iterator(scenePath))
            {
                if (entry.path().extension() == ".json")
                {
                    std::string name = entry.path().stem().string();
                    m_Scenes[name] = nullptr; // Not loaded yet
                }
            }
        }
    }

    void SceneManager::Update(float dt)
    {
        // Update loading scenes
        UpdateLoadingScenes();

        // Update active scene
        if (m_ActiveScene && m_ActiveScene->IsLoaded())
        {
            m_ActiveScene->OnUpdate(dt);
        }

        // Update all loaded scenes if additive
        for (auto& scene : m_LoadedScenes)
        {
            if (scene != m_ActiveScene && scene->IsLoaded())
            {
                scene->OnUpdate(dt);
            }
        }

        // Remove unloaded scenes
        RemoveUnloadedScenes();
    }

    void SceneManager::Shutdown()
    {
        UnloadAllScenes();
        m_Scenes.clear();
    }

    std::shared_ptr<Scene> SceneManager::CreateScene(const std::string& name)
    {
        auto scene = std::make_shared<Scene>(name, path, pSystemManager);
        m_Scenes[name] = scene;
        return scene;
    }

    std::shared_ptr<Scene> SceneManager::LoadScene(const std::string& filepath, bool additive)
    {
        std::filesystem::path path(filepath);
        std::string name = path.stem().string();

        // Check if already loaded
        if (IsSceneLoaded(name))
            return GetScene(name);

        // Create and load scene
        auto scene = std::make_shared<Scene>(name, path, pSystemManager);
        scene->OnLoad();

        m_LoadedScenes.push_back(scene);
        m_Scenes[name] = scene;

        if (!additive)
        {
            if (m_ActiveScene)
                UnloadScene(m_ActiveScene->GetName());
            m_ActiveScene = scene;
        }

        return scene;
    }

    void SceneManager::LoadSceneAsync(const std::string& filepath, bool additive)
    {
        std::filesystem::path path(filepath);
        std::string name = path.stem().string();

        if (IsSceneLoaded(name))
            return;

        auto scene = std::make_shared<Scene>(name, path, pSystemManager);
        scene->LoadAsync(); // Non-blocking load

        m_LoadingScenes.push_back(scene);
        m_Scenes[name] = scene;

        if (!additive && m_ActiveScene)
        {
            UnloadScene(m_ActiveScene->GetName());
        }
    }

    void SceneManager::UnloadScene(const std::string& name)
    {
        auto it = std::find_if(m_LoadedScenes.begin(), m_LoadedScenes.end(),
            [&name](const std::shared_ptr<Scene>& scene) {
                return scene->GetName() == name;
            });

        if (it != m_LoadedScenes.end())
        {
            (*it)->Unload();

            if (m_ActiveScene == *it)
                m_ActiveScene = nullptr;
        }
    }

    void SceneManager::UnloadAllScenes()
    {
        for (auto& scene : m_LoadedScenes)
        {
            scene->Unload();
        }
        m_LoadedScenes.clear();
        m_ActiveScene = nullptr;
    }

    void SceneManager::SetActiveScene(const std::string& name)
    {
        auto scene = GetScene(name);
        if (scene && scene->IsLoaded())
        {
            m_ActiveScene = scene;
        }
    }

    std::shared_ptr<Scene> SceneManager::GetScene(const std::string& name)
    {
        auto it = m_Scenes.find(name);
        return (it != m_Scenes.end()) ? it->second : nullptr;
    }

    bool SceneManager::IsSceneLoaded(const std::string& name) const
    {
        auto it = std::find_if(m_LoadedScenes.begin(), m_LoadedScenes.end(),
            [&name](const std::shared_ptr<Scene>& scene) {
                return scene->GetName() == name;
            });
        return it != m_LoadedScenes.end() && (*it)->IsLoaded();
    }

    std::vector<std::string> SceneManager::GetAvailableScenes() const
    {
        std::vector<std::string> scenes;
        for (const auto& [name, scene] : m_Scenes)
        {
            scenes.push_back(name);
        }
        return scenes;
    }

    void SceneManager::UpdateLoadingScenes()
    {
        auto it = m_LoadingScenes.begin();
        while (it != m_LoadingScenes.end())
        {
            auto& scene = *it;

            if (scene->GetState() == SceneState::Loaded)
            {
                // Loading complete
                m_LoadedScenes.push_back(scene);

                if (!m_ActiveScene)
                    m_ActiveScene = scene;

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
                    return scene->GetState() == SceneState::Unloaded;
                }),
            m_LoadedScenes.end()
        );
    }
}