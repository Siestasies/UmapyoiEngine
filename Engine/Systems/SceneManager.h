/*!
\file   SceneManager.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Shahir Rasid (100%)
\par    E-mail: b.muhammadshahir@digipen.edu
\par    DigiPen login: b.muhammadshahir

\brief
This file implements the definition for a Scene manager which stores
and controls the life-cycle of a scene.
Also contains helper functions to add and set active scene.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/
#pragma once
#include "SceneType.h"
#include "Core/SystemType.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <iostream>
#include <functional>

namespace Uma_Engine
{
    class SceneManager : public ISystem
    {
    public:
        void Init() override;
        void Update(float dt) override;
        void Shutdown() override;

        // Template method to register custom scene types
        template<typename T>
        void RegisterSceneType(const std::string& typeName)
        {
            static_assert(std::is_base_of<Scene, T>::value, "T must inherit from Scene");

            m_SceneFactories[typeName] = [this](const std::string& name, const std::string& filepath) -> std::shared_ptr<Scene> {
                return std::make_shared<T>(name, filepath, pSystemManager);
                };
        }

        // Create a scene of a specific registered type
        template<typename T>
        std::shared_ptr<T> CreateScene(const std::string& name, const std::string& filepath)
        {
            // Check if scene already exists
            if (m_Scenes.find(name) != m_Scenes.end() && m_Scenes[name] != nullptr)
            {
                std::cout << "Scene '" << name << "' already exists!" << std::endl;
                return std::dynamic_pointer_cast<T>(m_Scenes[name]);
            }

            auto scene = std::make_shared<T>(name, filepath, pSystemManager);
            m_Scenes[name] = scene;
            return scene;
        }

        // Load a scene by name (must be registered first)
        std::shared_ptr<Scene> LoadScene(const std::string& name, bool additive = false);

        // Load a scene by type
        template<typename T>
        std::shared_ptr<T> LoadSceneOfType(const std::string& name, const std::string& filepath, bool additive = false)
        {
            // Check if already loaded
            if (IsSceneLoaded(name))
            {
                std::cout << "Scene '" << name << "' is already loaded!" << std::endl;
                return std::dynamic_pointer_cast<T>(GetScene(name));
            }

            // Create scene
            auto scene = CreateScene<T>(name, filepath);

            // Load the scene synchronously
            scene->Load();

            // Add to loaded scenes
            m_LoadedScenes.push_back(scene);

            // Handle non-additive loading
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

        void LoadSceneAsync(const std::string& name, bool additive = false);
        void UnloadScene(const std::string& name);
        void UnloadAllScenes();

        void SetActiveScene(const std::string& name);
        std::shared_ptr<Scene> GetScene(const std::string& name);

        template<typename T>
        std::shared_ptr<T> GetSceneAs(const std::string& name)
        {
            return std::dynamic_pointer_cast<T>(GetScene(name));
        }

        std::shared_ptr<Scene> GetActiveScene() { return m_ActiveScene; }
        bool IsSceneLoaded(const std::string& name) const;
        //std::vector<std::string> GetLoadedSceneNames() const;

    private:
        void UpdateLoadingScenes();
        void RemoveUnloadedScenes();

        // Factory function type
        using SceneFactory = std::function<std::shared_ptr<Scene>(const std::string&, const std::string&)>;

        // Map of scene type names to factory functions
        std::unordered_map<std::string, SceneFactory> m_SceneFactories;

        // All known scenes (loaded or not)
        std::unordered_map<std::string, std::shared_ptr<Scene>> m_Scenes;

        // Currently loaded scenes
        std::vector<std::shared_ptr<Scene>> m_LoadedScenes;

        // Scenes currently loading async
        std::vector<std::shared_ptr<Scene>> m_LoadingScenes;

        // The main active scene
        std::shared_ptr<Scene> m_ActiveScene;
    };
}