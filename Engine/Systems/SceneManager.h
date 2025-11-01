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
#include "ImguiManager.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>
#include <iostream>

namespace Uma_Engine
{
    enum PLAYMODE
    {
        PM_PLAY,
        PM_PAUSE,
        PM_STOP
    };


    class SceneManager : public ISystem
    {
    public:
        // ISYSTEM OVERRIDES
        void Init() override;
        void Update(float dt) override;
        void Shutdown() override;

        // SCENE MANAGEMENT STUFF
        void SetImguiHandler(ImguiManager* im) { imHandler = im; }

        // Create a new empty scene
        std::shared_ptr<Scene> CreateScene(const std::string& name, const std::string& filepath = "");
        // Load scene synchronously (blocks until loaded)
        std::shared_ptr<Scene> LoadScene(const std::string& name, bool additive = false);
        // Load scene asynchronously (non-blocking)
        void LoadSceneAsync(const std::string& name, bool additive = false);
        void UnloadScene(const std::string& name);
        void RemoveScene(const std::string& name);
        void UnloadAllScenes();
        void SetActiveScene(const std::string& name);

        std::shared_ptr<Scene> GetScene(const std::string& name);
        std::shared_ptr<Scene> GetActiveScene() { return m_ActiveScene; }

        // SCRIPT STUFF
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
        void AttachScriptToScene(const std::string& sceneName, const std::string& scriptName);
        void DetachScriptFromScene(const std::string& sceneName, const std::string& scriptName);

        std::vector<std::string> GetRegisteredScriptNames() const;

        // SCENE STUFF
        bool HasScene(const std::string& name) const;
        bool IsSceneLoaded(const std::string& name) const;
        bool IsSceneLoading(const std::string& name) const;
        SceneState GetSceneState(const std::string& name) const;

        // Get scene load progress (0.0 to 1.0)
        float GetSceneLoadProgress(const std::string& name) const;

        std::vector<std::string> GetAllSceneNames() const;
        std::vector<std::string> GetLoadedSceneNames() const;
        std::string GetActiveSceneName() const;

    private:
        void UpdateLoadingScenes();
        void RemoveUnloadedScenes();

        // Script factory type
        using ScriptFactory = std::function<std::shared_ptr<SceneScript>()>;
        std::unordered_map<std::string, ScriptFactory> m_ScriptFactories;

        std::unordered_map<std::string, std::shared_ptr<Scene>> m_Scenes;
        std::vector<std::shared_ptr<Scene>> m_LoadedScenes;
        std::vector<std::shared_ptr<Scene>> m_LoadingScenes;
        std::shared_ptr<Scene> m_ActiveScene;

        ImguiManager* imHandler;

        PLAYMODE playMode = PLAYMODE::PM_STOP;
    };
}