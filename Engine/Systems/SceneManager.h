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
Scene manager with script registry system for data-driven scene creation.
Supports async loading, additive scenes, and runtime script attachment.

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
#include <functional>
#include <iostream>

namespace Uma_Engine
{
    class SceneManager : public ISystem
    {
    public:
        // ==================== ISystem Interface ====================

        void Init() override;
        void Update(float dt) override;
        void Shutdown() override;

        // ==================== Scene Management ====================

        // Create a new empty scene
        std::shared_ptr<Scene> CreateScene(const std::string& name, const std::string& filepath = "");

        // Load scene synchronously (blocks until loaded)
        std::shared_ptr<Scene> LoadScene(const std::string& name, bool additive = false);

        // Load scene asynchronously (non-blocking)
        void LoadSceneAsync(const std::string& name, bool additive = false);

        // Unload a scene (but keep it in memory)
        void UnloadScene(const std::string& name);

        // Remove scene completely from memory
        void RemoveScene(const std::string& name);

        // Unload all scenes
        void UnloadAllScenes();

        // Set the active scene (must be loaded)
        void SetActiveScene(const std::string& name);

        // Get a scene by name
        std::shared_ptr<Scene> GetScene(const std::string& name);

        // Get the currently active scene
        std::shared_ptr<Scene> GetActiveScene() { return m_ActiveScene; }

        // ==================== Script Registry ====================

        // Register a script factory (for creating scripts by name)
        template<typename T>
        void RegisterScript(const std::string& scriptName)
        {
            static_assert(std::is_base_of<SceneScript, T>::value, "T must inherit from SceneScript");

            m_ScriptFactories[scriptName] = []() -> std::shared_ptr<SceneScript> {
                return std::make_shared<T>();
                };

            std::cout << "Registered script: " << scriptName << std::endl;
        }

        // Create a script instance by name
        std::shared_ptr<SceneScript> CreateScript(const std::string& scriptName);

        // Attach a script to a scene by name
        void AttachScriptToScene(const std::string& sceneName, const std::string& scriptName);

        // Detach a script from a scene
        void DetachScriptFromScene(const std::string& sceneName, const std::string& scriptName);

        // Get all registered script names (for ImGui dropdowns)
        std::vector<std::string> GetRegisteredScriptNames() const;

        // ==================== Scene Queries ====================

        // Check if scene exists
        bool HasScene(const std::string& name) const;

        // Check if scene is loaded
        bool IsSceneLoaded(const std::string& name) const;

        // Check if scene is currently loading
        bool IsSceneLoading(const std::string& name) const;

        // Get scene state
        SceneState GetSceneState(const std::string& name) const;

        // Get scene load progress (0.0 to 1.0)
        float GetSceneLoadProgress(const std::string& name) const;

        // Get all scene names
        std::vector<std::string> GetAllSceneNames() const;

        // Get loaded scene names
        std::vector<std::string> GetLoadedSceneNames() const;

        // Get active scene name
        std::string GetActiveSceneName() const;

    private:
        // ==================== Internal Helpers ====================

        void UpdateLoadingScenes();
        void RemoveUnloadedScenes();

        // ==================== Data Members ====================

        // Script factory type
        using ScriptFactory = std::function<std::shared_ptr<SceneScript>()>;

        // Registry of script factories
        std::unordered_map<std::string, ScriptFactory> m_ScriptFactories;

        // All scenes (loaded or not)
        std::unordered_map<std::string, std::shared_ptr<Scene>> m_Scenes;

        // Loaded scenes
        std::vector<std::shared_ptr<Scene>> m_LoadedScenes;

        // Scenes currently loading asynchronously
        std::vector<std::shared_ptr<Scene>> m_LoadingScenes;

        // Currently active scene
        std::shared_ptr<Scene> m_ActiveScene;
    };
}