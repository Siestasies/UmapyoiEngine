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
This file implements the definition for a base class of scene in that
anything that wants to be a system should inherit from this class.

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

// ECS Components
#include "ECS/Components/Transform.h"
#include "ECS/Components/RigidBody.h"
#include "ECS/Components/Player.h"
#include "ECS/Components/Sprite.h"
#include "ECS/Components/Collider.h"
#include "ECS/Components/Camera.h"
#include "ECS/Components/Enemy.h"

// Engine Systems
#include "Systems/InputSystem.h"
#include "Systems/Graphics.hpp"
#include "Systems/Sound.hpp"
#include "Systems/ResourcesManager.hpp"
#include "Systems/CameraSystem.hpp"
#include "../Core/SystemManager.h"
#include "../Core/EventSystem.h"
#include "../Core/ECSEvents.h"
#include "../Core/IMGUIEvents.h"

#include "Core/FilePaths.h"

#include <string>
#include <memory>
#include <future>

namespace Uma_Engine
{
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
            Scene(const std::string& name, const std::string& filepath, SystemManager* sm);
            ~Scene();

            void OnLoad();
            void OnLoadAsync();
            void OnUnload();
            void OnUpdate(float dt);
            void OnRender();

            void Serialize(const std::string& filepath);
            void Deserialize(const std::string& filepath);

            Uma_ECS::Entity CreateEntity();
            void DestroyEntity(Uma_ECS::Entity entity);

            SceneState GetState() const { return m_State; }
            float GetLoadProgress() const { return m_LoadProgress; };
            Uma_ECS::Coordinator& GetCoordinator() { return m_Coordinator; };
            const std::string& GetName() const { return m_Name; };
            const std::string& GetFilePath() const { return m_FilePath; };
            bool isLoaded() const { return m_State == SceneState::SCENE_RUNNING; };

        private:
            friend class SceneManager;
            SystemManager* m_SystemManager = nullptr;

            void SpawnDefaultEntities();

            std::string m_Name;
            std::string m_FilePath;
            SceneState m_State = SceneState::SCENE_UNLOADED;
            float m_LoadProgress = 0.0f;

            // Engine Systems
            Uma_Engine::HybridInputSystem* m_HybridInputSystem;
            Uma_Engine::Graphics* m_Graphics;
            Uma_Engine::Sound* m_Sound;
            Uma_Engine::ResourcesManager* m_ResourcesManager;
            Uma_Engine::EventSystem* m_EventSystem;

            // ECS related
            Uma_ECS::Coordinator m_Coordinator;
            std::shared_ptr<Uma_ECS::PhysicsSystem> m_PhysicsSystem;
            std::shared_ptr<Uma_ECS::CollisionSystem> m_CollisionSystem;
            std::shared_ptr<Uma_ECS::PlayerControllerSystem> m_PlayerController;
            std::shared_ptr<Uma_ECS::RenderingSystem> m_RenderingSystem;
            std::shared_ptr<Uma_ECS::CameraSystem> m_CameraSystem;
            Uma_ECS::Entity m_Player;
            Uma_ECS::Entity m_Cam;

            std::future<void> m_LoadFuture;
            void LoadInternal();
    };
}