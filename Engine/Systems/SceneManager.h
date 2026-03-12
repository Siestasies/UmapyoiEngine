/*!
\file   SceneManager.h
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Shahir Rasid (Everything else)
\par    E-mail: b.muhammadshahir@digipen.edu
\par    DigiPen login: b.muhammadshahir

\co-author Javier Chua Dong Qing (EditorCamera)
\par       E-mail: javierdongqing.chua@digipen.edu
\par       DigiPen login: javierdongqing.chua

\brief
Scene manager with script registry system for data-driven scene creation.
Supports async loading, additive scenes, and runtime script attachment.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/
#pragma once
#include "SceneType.h"
#include "Core/SystemType.h"
#include "../EditorApp/Scripts/ImguiManager.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>
#include <iostream>
#include "Systems/EditorCamera.h"
#include "Core/EventSystem.h"

namespace Uma_ECS
{
    class Coordinator;
}

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
        /*!
        \brief Initializes the SceneManager, setting up internal state and event listeners.
        */
        void Init() override;

        /*!
        \brief Updates all active and loading scenes each frame.
        \param dt Delta time in seconds since the last frame.
        */
        void Update(float dt) override;

        /*!
        \brief Shuts down the SceneManager, unloading all scenes and releasing resources.
        */
        void Shutdown() override;

        /**
        * \brief Creates a new empty unloaded scene object and registers it
        * \param name The unique name for the new scene
        * \param filepath The file pathfor saving/loading
        * \return A shared pointer to the newly created Scene object
        */
        std::shared_ptr<Scene> CreateScene(const std::string& name, const std::string& filepath = "");

        /**
        * \brief Creates a new empty scene with a default unique name and loads it
        */
        void CreateNewScene();

        /**
        * \brief Loads a scene from its file path synchronously
        * \param name The name of the scene to load
        * \param additive If true, loads the scene without unloading current scenes
        * \return A shared pointer to the loaded Scene object, or nullptr if loading is blocked
        */
        std::shared_ptr<Scene> LoadScene(const std::string& name, bool additive = false);

        /**
        * \brief Begins loading a scene asynchronously
        * \param name The name of the scene to load
        * \param additive If true, loads the scene without unloading current scenes
        */
        void LoadSceneAsync(const std::string& name, bool additive = false);

        /**
        * \brief Unloads a specific scene releasing its resources
        * \param name The name of the scene to unload
        */
        void UnloadScene(const std::string& name);

        /**
        * \brief Saves a specific scene to its associated file path
        * \param name The name of the scene to save
        */
        void SaveScene(const std::string& name);

        /**
        * \brief Unloads and removes a scene from the manager's tracking
        * \param name The name of the scene to remove
        */
        void RemoveScene(const std::string& name, bool needLoadNewScene);

        /**
        * \brief Unloads all currently loaded and loading scenes
        */
        void UnloadAllScenes();

        /**
        * \brief Sets a currently loaded scene as the active scene
        * \param name The name of the scene to activate
        */
        void SetActiveScene(const std::string& name);

        /**
        * \brief Retrieves a scene by its name
        * \param name The name of the scene
        * \return A shared pointer to the scene, or nullptr if not found
        */
        std::shared_ptr<Scene> GetScene(const std::string& name);
        /*!
        \brief Retrieves the currently active scene.
        \return A shared pointer to the active scene, or nullptr if no scene is active.
        */
        std::shared_ptr<Scene> GetActiveScene() { return m_ActiveScene; }

        /*!
        \brief Gets a mutable reference to the editor camera.
        \return A reference to the EditorCamera instance.
        */
        EditorCamera& GetEditorCamera() { return m_EditorCamera; }

        /*!
        \brief Gets a const reference to the editor camera.
        \return A const reference to the EditorCamera instance.
        */
        const EditorCamera& GetEditorCamera() const { return m_EditorCamera; }

        /*!
        \brief Checks whether the editor camera is currently in use.
        \return True if the editor camera is active, false otherwise.
        */
        bool IsUsingEditorCamera() const { return m_UseEditorCamera; }

        /*!
        \brief Configures whether the SceneManager operates in editor mode.
        \param isEditor True to enable editor mode, false for runtime mode.
        */
        void SetEditorMode(bool isEditor);

        /*!
        \brief Registers a script factory so instances can be created by name at runtime.
        \param scriptName The unique string name to associate with the script type T.
        */
        template<typename T>
        void RegisterScript(const std::string& scriptName)
        {
            static_assert(std::is_base_of<SceneScript, T>::value, "T must inherit from SceneScript");

            m_ScriptFactories[scriptName] = []() -> std::shared_ptr<SceneScript> {
                return std::make_shared<T>();
                };

            std::cout << "Registered script: " << scriptName << std::endl;
        }

        /**
        * \brief Creates an instance of a registered script by its string name
        * \param scriptName The name of the script to create
        * \return A shared pointer to the new script instance, or nullptr if the name is not registered
        */
        std::shared_ptr<SceneScript> CreateScript(const std::string& scriptName);

        /**
        * \brief Attaches a new instance of a script to a scene
        * \param sceneName The name of the scene to attach to
        * \param scriptName The name of the registered script to create and attach
        */
        void AttachScriptToScene(const std::string& sceneName, const std::string& scriptName);

        /**
        * \brief Detaches and destroys a script instance from a scene
        * \param sceneName The name of the scene
        * \param scriptName The name of the script to detach
        */
        void DetachScriptFromScene(const std::string& sceneName, const std::string& scriptName);

        /**
        * \brief Gets a list of all registered script names
        * \return A vector of strings containing the names of all scripts
        */
        std::vector<std::string> GetRegisteredScriptNames() const;

        // SCENE STUFF
        /**
        * \brief Checks if a scene with the given name is known to the manager
        * \param name The name of the scene
        * \return true if the scene exists
        */
        bool HasScene(const std::string& name) const;

        /**
        * \brief Checks if a scene is currently loaded in memory
        * \param name The name of the scene
        * \return true if the scene's state is Loaded or Running
        */
        bool IsSceneLoaded(const std::string& name) const;

        /**
        * \brief Checks if a scene is currently being loaded
        * \param name The name of the scene
        * \return true if the scene's state is Loading
        */
        bool IsSceneLoading(const std::string& name) const;

        /**
        * \brief Gets the current load state of a scene
        * \param name The name of the scene
        * \return The SceneState enum
        */
        SceneState GetSceneState(const std::string& name) const;

        /*!
        \brief Gets the current loading progress of a scene.
        \param name The name of the scene.
        \return A float from 0.0 to 1.0 representing the load progress.
        */
        float GetSceneLoadProgress(const std::string& name) const;

        /*!
        \brief Gets the names of all scenes known to the manager.
        \return A vector of scene name strings.
        */
        std::vector<std::string> GetAllSceneNames() const;

        /*!
        \brief Gets the names of all currently loaded scenes.
        \return A vector of loaded scene name strings.
        */
        std::vector<std::string> GetLoadedSceneNames() const;

        /*!
        \brief Gets the name of the currently active scene.
        \return The active scene name, or an empty string if no scene is active.
        */
        std::string GetActiveSceneName() const;

        /*!
        \brief Gets the ECS Coordinator associated with the active scene.
        \return A pointer to the active scene's Coordinator, or nullptr if no scene is active.
        */
        Uma_ECS::Coordinator* GetActiveSceneCoordinator() const;

    private:
        /*!
        \brief Processes scenes that are currently loading and advances their load state.
        */
        void UpdateLoadingScenes();

        /*!
        \brief Removes scenes that have been fully unloaded from the loaded scenes list.
        */
        void RemoveUnloadedScenes();

        /*!
        \brief Renders the SceneManager ImGui debug and editor window.
        */
        void UpdateIMGUIWindow();

        // Script factory type
        using ScriptFactory = std::function<std::shared_ptr<SceneScript>()>;
        std::unordered_map<std::string, ScriptFactory> m_ScriptFactories;

        std::unordered_map<std::string, std::shared_ptr<Scene>> m_Scenes;
        std::vector<std::shared_ptr<Scene>> m_LoadedScenes;
        std::vector<std::shared_ptr<Scene>> m_LoadingScenes;
        std::shared_ptr<Scene> m_ActiveScene;

        bool m_IsEditorMode = false;
        PLAYMODE playMode = PLAYMODE::PM_STOP;
        bool mGamePause = false;

        bool isUnloading = false;

        // Event system
        EventSystem* pEventSystem;

        // Editor camera
        EditorCamera m_EditorCamera;
        bool m_UseEditorCamera = false;
        bool m_isMouseOverUI = false;
    };
}