#include "SceneManager.h"
#include "Core/FilePaths.h"
#include <filesystem>
#include <algorithm>

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
                    m_Scenes[name] = nullptr;
                }
            }
        }
    }

    void SceneManager::Update(float dt)
    {
        // Update RUNNING scenes first
        UpdateLoadingScenes();

        // Update active scene
        if (m_ActiveScene && m_ActiveScene->IsLoaded())
        {
            m_ActiveScene->Update(dt);
        }

        // Update all loaded scenes if additive loading is used
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
        UnloadAllScenes();
        m_Scenes.clear();
    }

    //std::shared_ptr<Scene> SceneManager::CreateScene(const std::string& name, const std::string& filepath)
    //{
    //    // Check if scene already exists
    //    if (m_Scenes.find(name) != m_Scenes.end() && m_Scenes[name] != nullptr)
    //    {
    //        std::cout << "Scene '" << name << "' already exists!" << std::endl;
    //        return m_Scenes[name];
    //    }

    //    auto scene = std::make_shared<Scene>(name, filepath, pSystemManager);
    //    m_Scenes[name] = scene;
    //    return scene;
    //}

    std::shared_ptr<Scene> SceneManager::LoadScene(const std::string& filepath, bool additive)
    {
        std::filesystem::path path(filepath);
        std::string name = path.stem().string();

        // Check if already loaded
        if (IsSceneLoaded(name))
        {
            std::cout << "Scene '" << name << "' is already loaded!" << std::endl;
            return GetScene(name);
        }

        // Create scene if it doesn't exist
        std::shared_ptr<Scene> scene;
        if (m_Scenes.find(name) != m_Scenes.end() && m_Scenes[name] != nullptr)
        {
            scene = m_Scenes[name];
        }
        else
        {
            scene = std::make_shared<Scene>(name, filepath, pSystemManager);
            m_Scenes[name] = scene;
        }

        // Load the scene synchronously
        scene->Load();

        // Add to loaded scenes
        m_LoadedScenes.push_back(scene);

        // Handle non-additive loading (unload previous active scene)
        if (!additive && m_ActiveScene)
        {
            UnloadScene(m_ActiveScene->GetName());
        }

        // Set as active scene
        if (!additive || !m_ActiveScene)
        {
            m_ActiveScene = scene;
        }

        return scene;
    }

    void SceneManager::LoadSceneAsync(const std::string& filepath, bool additive)
    {
        std::filesystem::path path(filepath);
        std::string name = path.stem().string();

        // Check if already loaded or loading
        if (IsSceneLoaded(name))
        {
            std::cout << "Scene '" << name << "' is already loaded!" << std::endl;
            return;
        }

        // Check if already in loading queue
        auto it = std::find_if(m_LoadingScenes.begin(), m_LoadingScenes.end(),
            [&name](const std::shared_ptr<Scene>& scene) {
                return scene->GetName() == name;
            });

        if (it != m_LoadingScenes.end())
        {
            std::cout << "Scene '" << name << "' is already loading!" << std::endl;
            return;
        }

        // Create scene if it doesn't exist
        std::shared_ptr<Scene> scene;
        if (m_Scenes.find(name) != m_Scenes.end() && m_Scenes[name] != nullptr)
        {
            scene = m_Scenes[name];
        }
        else
        {
            scene = std::make_shared<Scene>(name, filepath, pSystemManager);
            m_Scenes[name] = scene;
        }

        // Start async load
        scene->LoadAsync();
        m_LoadingScenes.push_back(scene);

        // Handle non-additive loading
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

            // Clear active scene if it's being unloaded
            if (m_ActiveScene == *it)
            {
                m_ActiveScene = nullptr;
            }
        }
    }

    void SceneManager::UnloadAllScenes()
    {
        // Unload all loaded scenes
        for (auto& scene : m_LoadedScenes)
        {
            scene->OnUnload();
        }
        m_LoadedScenes.clear();

        // Wait for any loading scenes to finish then unload
        for (auto& scene : m_LoadingScenes)
        {
            // Scene will handle waiting for async load in Unload
            scene->Unload();
        }
        m_LoadingScenes.clear();

        m_ActiveScene = nullptr;
    }

    void SceneManager::SetActiveScene(const std::string& name)
    {
        auto scene = GetScene(name);
        if (scene && scene->IsLoaded())
        {
            m_ActiveScene = scene;
            std::cout << "Active scene set to: " << name << std::endl;
        }
        else
        {
            std::cout << "Cannot set active scene: '" << name << "' is not loaded!" << std::endl;
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

    //std::vector<std::string> SceneManager::GetAvailableScenes() const
    //{
    //    std::vector<std::string> scenes;
    //    for (const auto& [name, scene] : m_Scenes)
    //    {
    //        scenes.push_back(name);
    //    }
    //    return scenes;
    //}

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
                    std::cout << "Scene '" << scene->GetName() << "' loaded and set as active." << std::endl;
                }
                else
                {
                    std::cout << "Scene '" << scene->GetName() << "' loaded additively." << std::endl;
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
        // Remove scenes that are fully unloaded
        m_LoadedScenes.erase(
            std::remove_if(m_LoadedScenes.begin(), m_LoadedScenes.end(),
                [](const std::shared_ptr<Scene>& scene) {
                    return scene->GetState() == SceneState::SCENE_UNLOADED;
                }),
            m_LoadedScenes.end()
        );
    }
}