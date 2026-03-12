/*!
\file   SceneType.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Shahir Rasid (100%)
\par    E-mail: b.muhammadshahir@digipen.edu
\par    DigiPen login: b.muhammadshahir

\brief
This file implements the declarations for a base class of scene in that
anything that wants to be a scene should inherit from this class.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/
#pragma once

// ECS Core
#include "ECS/Core/Coordinator.hpp"

// ECS Systems
#include "ECS/Systems/PhysicsSystem.hpp"
#include "ECS/Systems/PlayerControllerSystem.hpp"
#include "ECS/Systems/RenderingSystem.hpp"
#include "ECS/Systems/CollisionSystem.hpp"
#include "ECS/Systems/LuaScriptingSystem.hpp"
#include "../Engine/UI/Systems/UISystem.h"
#include "ECS/Systems/TransformSystem.hpp"
#include "ECS/Systems/AudioSystem.hpp"
#include "ECS/Systems/AnimatorSystem.hpp"
#include "ECS/Systems/PathFindingSystem.hpp"
#include "ECS/Systems/ParticleSystem.hpp"
#include "ECS/Systems/ProjectileSystem.hpp"
#include "ECS/Systems/TilemapSystem.hpp"
#include "ECS/Systems/FSMSystem.hpp"

// ECS Components
#include "ECS/Components/Transform.h"
#include "ECS/Components/RigidBody.h"
#include "ECS/Components/Player.h"
#include "ECS/Components/Sprite.h"
#include "ECS/Components/Collider.h"
#include "ECS/Components/Camera.h"
#include "ECS/Components/Enemy.h"
#include "ECS/Components/Projectile.h"
#include "ECS/Components/LuaScript.h"
#include "ECS/Components/Animator.h"
#include "ECS/Components/AudioListener.h"
#include "ECS/Components/AudioComponent.h"
#include "ECS/Components/PathFinding.h"
#include "ECS/Components/ParticleEmitter.h"
#include "ECS/Components/Tilemap.h"
#include "ECS/Components/FSM.h"


// UI Components
#include "../UI/Components/RectTransform.h"
#include "../UI/Components/Canvas.h"
#include "../UI/Components/Image.h"
#include "../UI/Components/Button.h"
#include "../UI/Components/Text.h"

// Engine Systems
//#include "Systems/InputSystem.h"
#include "Systems/HybridInputSystem.h"
#include "Systems/Graphics.hpp"
#include "Systems/SoundManager.hpp"
#include "Systems/ResourcesManager.hpp"
#include "Systems/CameraSystem.hpp"
#include "../EditorApp/Editor/Core/EditorSystem.h"
#include "../Core/SystemManager.h"
#include "../Core/EventSystem.h"
#include "../Events/ECSEvents.h"
#include "../Events/IMGUIEvents.h"
#include "../PlayFab/Core/PlayFabManager.h"

// Serializer
#include "Core/GameSerializer.h"

// Engine Settings
#include "Core/EngineConfig.h"
#include "Core/FilePaths.h"

// debug
#include "Debugging/Debugger.hpp"

#include <future>

namespace Uma_Engine
{
    // forward decalaration
    class SceneScript;

    // enum fro asyn loading tracking
    enum class SceneState
    {
        SCENE_UNLOADED,
        SCENE_LOADING,
        SCENE_RUNNING,
        SCENE_UNLOADING
    };

    class Scene
    {
        public:
            /*!
            \brief Constructs a Scene with a name, file path, and system manager reference.
            \param name Display name of the scene.
            \param filepath File path used for serialization and deserialization.
            \param sm Pointer to the engine SystemManager for accessing engine systems.
            */
            Scene(const std::string& name, const std::string& filepath, SystemManager* sm);

            /*!
            \brief Destroys the scene and releases all associated resources.
            */
            ~Scene();

            // Lifecycle Stuff (called by SceneManager)
             /**
             * \brief Loads the Scene.
             */
            void Load();

            /**
            * \brief Loads the Scene asynchronously.
            */
            void LoadAsync();

            /**
            * \brief Unloads the Scene.
            */
            void Unload();

            /**
            * \brief Updates the Scene.
            * \param dt delta time
            */
            void Update(float dt);

            /**
            * \brief Updates the only selected parts of the Scene.
            * \param dt delta time
            */
            void UpdateSelective(float dt);

            // Script management
            /**
            * \brief Attaches a SceneScript object to a Scene.
            */
            void AttachScript(std::shared_ptr<SceneScript> script);

            /**
            * \brief Detaches a SceneScript object from a Scene.
            * \parama script name name of script
            */
            void DetachScript(const std::string& scriptName);

            /**
            * \brief Detaches all SceneScript objects from a Scene.
            */
            void DetachAllScripts();

            /**
            * \brief Checks if a Scene has a certain SceneScript.
            * \param scriptName name of script
            * \return bool true if found
            */
            bool HasScript(const std::string& scriptName) const;

            /**
            * \brief Returns string of attached SceneScript names.
            * \return vector<string> sting of names
            */
            std::vector<std::string> GetAttachedScriptNames() const;

            // Entity stuff
            /**
            * \brief Creates an entity.
            * \return Entity
            */
            Uma_ECS::Entity CreateEntity();

            /**
            * \brief Destroys a specified entity.
            * \param entity entity to be destroyed
            */
            void DestroyEntity(Uma_ECS::Entity entity);

            // Serialization
            /**
            * \brief Saves the scene to a specified filepath.
            * \param filepath file path to save to
            */
            void Serialize(const std::string& filepath = "");

            /**
            * \brief Loads the scene to a specified filepath.
            * \param filepath file path to load from
            */
            void Deserialize(const std::string& filepath = "");

            /**
            * \brief Getters for the scene object
            */
            /*!
            \brief Gets the current lifecycle state of the scene.
            \return Current SceneState value.
            */
            SceneState GetState() const { return m_State; }

            /*!
            \brief Gets the current async loading progress.
            \return Load progress as a float from 0.0 to 1.0.
            */
            float GetLoadProgress() const { return m_LoadProgress; }

            /*!
            \brief Checks if the scene is fully loaded and running.
            \return True if the scene state is SCENE_RUNNING.
            */
            bool IsLoaded() const { return m_State == SceneState::SCENE_RUNNING; }

            /*!
            \brief Gets the display name of the scene.
            \return Const reference to the scene name string.
            */
            const std::string& GetName() const { return m_Name; }

            /*!
            \brief Gets the file path used for serialization.
            \return Const reference to the file path string.
            */
            const std::string& GetFilePath() const { return m_FilePath; }

            /**
            * \brief Getters for ECS systems
            */
            /*!
            \brief Gets the ECS Coordinator for this scene.
            \return Reference to the scene's Coordinator.
            */
            Uma_ECS::Coordinator& GetCoordinator() { return m_Coordinator; }

            /*!
            \brief Gets the input system.
            \return Pointer to the HybridInputSystem.
            */
            Uma_Engine::HybridInputSystem* GetInputSystem() { return m_HybridInputSystem; }

            /*!
            \brief Gets the graphics system.
            \return Pointer to the Graphics system.
            */
            Uma_Engine::Graphics* GetGraphics() { return m_Graphics; }

            /*!
            \brief Gets the sound manager.
            \return Pointer to the SoundManager.
            */
            Uma_Engine::SoundManager* GetSound() { return m_Sound; }

            /*!
            \brief Gets the resources manager.
            \return Pointer to the ResourcesManager.
            */
            Uma_Engine::ResourcesManager* GetResourcesManager() { return m_ResourcesManager; }

            /*!
            \brief Gets the event system.
            \return Pointer to the EventSystem.
            */
            Uma_Engine::EventSystem* GetEventSystem() { return m_EventSystem; }

            /*!
            \brief Gets the PlayFab manager.
            \return Pointer to the PlayFabManager.
            */
            Uma_Engine::PlayFabManager* GetPlayFabManager() { return m_PlayFabManager; }

        //protected:
            // Engine Systems
            Uma_Engine::SystemManager* m_SystemManager;
            Uma_Engine::HybridInputSystem* m_HybridInputSystem;
            Uma_Engine::Graphics* m_Graphics;
            Uma_Engine::SoundManager* m_Sound;
            Uma_Engine::ResourcesManager* m_ResourcesManager;
            Uma_Engine::EventSystem* m_EventSystem;
            Uma_Engine::EditorSystem* m_EditorSystem;
            Uma_Engine::PlayFabManager* m_PlayFabManager;
            Uma_Engine::EngineConfig g_EngineConfig;

            // ECS related
            using Coordinator = Uma_ECS::Coordinator;
            Coordinator m_Coordinator;
            std::shared_ptr<Uma_ECS::TransformSystem> m_TransformSystem;
            std::shared_ptr<Uma_ECS::PhysicsSystem> m_PhysicsSystem;
            std::shared_ptr<Uma_ECS::CollisionSystem> m_CollisionSystem;
            std::shared_ptr<Uma_ECS::PlayerControllerSystem> m_PlayerController;
            std::shared_ptr<Uma_ECS::RenderingSystem> m_RenderingSystem;
            std::shared_ptr<Uma_ECS::CameraSystem> m_CameraSystem;
            std::shared_ptr<Uma_ECS::AnimatorSystem> m_AnimatorSystem;
            std::shared_ptr<Uma_ECS::LuaScriptingSystem> m_LuaScriptingSystem;
            std::shared_ptr<Uma_ECS::AudioSystem> m_AudioSystem;
            std::shared_ptr<Uma_ECS::PathFindingSystem> m_PathFindingSystem;
            std::shared_ptr<Uma_ECS::ParticleSystem> m_ParticleSystem;
            std::shared_ptr<Uma_ECS::ProjectileSystem> m_ProjectileSystem;
            std::shared_ptr<Uma_ECS::TilemapSystem> m_TilemapSystem;
            std::shared_ptr<Uma_ECS::FSMSystem> m_FSMSystem;

            // ECS UI related
            std::shared_ptr<Uma_UI::UISystem> m_UISystem;

            // temp need to remove oneday
            Uma_ECS::Entity m_player;
            Uma_ECS::Entity m_cam;

            // Scene Specific
            Uma_Engine::GameSerializer gGameSerializer;
            // Attached scripts
            std::vector<std::shared_ptr<SceneScript>> m_AttachedScripts;

        private:
            /**
            * \brief Initializes ECS systems
            */
            void InitializeECS();

            /**
            * \brief Initializes UI systems
            */
            void InitializeUISystem();

            /**
            * \brief Updates ECS systems
            * \param dt delta time
            */
            void UpdateECSSystems(float dt);

            /**
            * \brief Updates ECS systems using fixed dt
            */
            void FixedUpdateECSSystems();

            /**
            * \brief Loads internal variables
            */
            void LoadInternal();

            // Scene metadata
            std::string m_Name;
            std::string m_FilePath;
            SceneState m_State = SceneState::SCENE_UNLOADED;
            float m_LoadProgress = 0.0f;

            // fixed timestamp accumulator
            float m_Accumulator = 0.0f;
            // Will be set from config
            float m_FixedTimeStep = 1.0f / 60.0f;

            // for async loading
            std::future<void> m_LoadFuture;
    };

    class SceneScript
    {
        public:
            /*!
            \brief Constructs a SceneScript with the given name.
            \param name Identifier name for this script.
            */
            SceneScript(const std::string& name) : m_Name(name) {}
            virtual ~SceneScript() = default;

            // Lifecycle hooks - override these in derived scripts
            /**
            * \brief An initialize for SceneScript object
            * \param scene reference to the scene to be attached to
            */
            virtual void OnAttach(Scene* scene) { m_Scene = scene; }

            /**
            * \brief What the script will do when detached.
            */
            virtual void OnDetach() {}

            /**
            * \brief What to do when the script is loaded.
            */
            virtual void OnLoad() {}

            /**
            * \brief What to do when the script is unloaded.
            */
            virtual void OnUnload() {}

            /**
            * \brief What to do when the script updating.
            */
            virtual void OnUpdate(float dt) { UNREFERENCED_PARAMETER(dt); }

            /**
            * \brief Getter for name of the script
            * \return string name of script
            */
            const std::string& GetName() const { return m_Name; }

        protected:
            std::string m_Name;
            Scene* m_Scene = nullptr;

            /**
            * \brief Getters for scene ECS systems
            */
            /*!
            \brief Gets the ECS Coordinator from the attached scene.
            \return Reference to the scene's Coordinator.
            */
            Uma_ECS::Coordinator& GetCoordinator() { return m_Scene->GetCoordinator(); }

            /*!
            \brief Gets the Lua scripting system from the attached scene.
            \return Reference to the LuaScriptingSystem.
            */
            Uma_ECS::LuaScriptingSystem& GetLuascriptingSystem() { return *m_Scene->m_LuaScriptingSystem; }

            /*!
            \brief Gets the pathfinding system from the attached scene.
            \return Reference to the PathFindingSystem.
            */
            Uma_ECS::PathFindingSystem& GetPathFindingSystem() { return *m_Scene->m_PathFindingSystem; }

            /*!
            \brief Gets the input system from the attached scene.
            \return Pointer to the HybridInputSystem.
            */
            Uma_Engine::HybridInputSystem* GetInput() { return m_Scene->GetInputSystem(); }

            /*!
            \brief Gets the graphics system from the attached scene.
            \return Pointer to the Graphics system.
            */
            Uma_Engine::Graphics* GetGraphics() { return m_Scene->GetGraphics(); }

            /*!
            \brief Gets the sound manager from the attached scene.
            \return Pointer to the SoundManager.
            */
            Uma_Engine::SoundManager* GetSound() { return m_Scene->GetSound(); }

            /*!
            \brief Gets the resources manager from the attached scene.
            \return Pointer to the ResourcesManager.
            */
            Uma_Engine::ResourcesManager* GetResources() { return m_Scene->GetResourcesManager(); }

            /*!
            \brief Gets the event system from the attached scene.
            \return Pointer to the EventSystem.
            */
            Uma_Engine::EventSystem* GetEventSystem() { return m_Scene->GetEventSystem(); }
            //Uma_Engine::PlayFabManager* GetPlayFabManager() { return m_Scene->GetPlayFabManager(); }
    };
}