#include "SceneType.h"
#include "Core/GameSerializer.h"
#include <future>

namespace Uma_Engine
{
    Scene::Scene(const std::string& name, const std::string& filepath, SystemManager* sm) :
        m_Name(name), m_FilePath(filepath), m_SystemManager(sm)
    {
    }

    Scene::~Scene()
    {
        if (m_State == SceneState::SCENE_RUNNING)
            Unload();
    }

    // ==================== PUBLIC INTERFACE ====================
    // These are called by SceneManager

    void Scene::Load()
    {
        if (m_State != SceneState::SCENE_UNLOADED)
            return;

        m_State = SceneState::SCENE_LOADING;
        LoadInternal();
        m_State = SceneState::SCENE_RUNNING;
        m_LoadProgress = 1.0f;
    }

    void Scene::LoadAsync()
    {
        if (m_State != SceneState::SCENE_UNLOADED)
            return;

        m_State = SceneState::SCENE_LOADING;
        m_LoadProgress = 0.0f;

        // Launch async task
        m_LoadFuture = std::async(std::launch::async, [this]() {
            LoadInternal();
            m_State = SceneState::SCENE_RUNNING;
            m_LoadProgress = 1.0f;
            });
    }

    void Scene::Unload()
    {
        if (m_State != SceneState::SCENE_RUNNING)
            return;

        m_State = SceneState::SCENE_UNLOADING;

        // Wait for async load if still loading
        if (m_LoadFuture.valid())
            m_LoadFuture.wait();

        // Call derived class cleanup BEFORE destroying entities
        OnUnload();

        // Destroy all entities
        m_Coordinator.DestroyAllEntities();

        // Unload resources
        if (m_ResourcesManager)
        {
            m_ResourcesManager->UnloadAllTextures();
            m_ResourcesManager->UnloadAllSound();
        }

        m_State = SceneState::SCENE_UNLOADED;
        m_LoadProgress = 0.0f;
        m_FirstFrame = true;
    }

    void Scene::Update(float dt)
    {
        if (m_State != SceneState::SCENE_RUNNING)
            return;

        // Smooth delta time
        if (m_FirstFrame)
        {
            m_SmoothedDt = dt;
            m_FirstFrame = false;
        }
        else
        {
            m_SmoothedDt = 0.9f * m_SmoothedDt + 0.1f * dt;
        }

        // Update ECS systems first
        UpdateECSSystems(dt);

        // Call derived class update logic
        OnUpdate(dt);
    }

    // ==================== INTERNAL LOADING ====================

    void Scene::LoadInternal()
    {
        // Get system handles
        m_HybridInputSystem = m_SystemManager->GetSystem<HybridInputSystem>();
        m_Graphics = m_SystemManager->GetSystem<Graphics>();
        m_Sound = m_SystemManager->GetSystem<Sound>();
        m_EventSystem = m_SystemManager->GetSystem<EventSystem>();
        m_ResourcesManager = m_SystemManager->GetSystem<ResourcesManager>();

        m_LoadProgress = 0.1f;

        // Initialize ECS systems
        InitializeECS();

        m_LoadProgress = 0.5f;

        // Call derived class initialization BEFORE deserialization
        // This allows the derived class to set up event listeners, etc.
        OnLoad();

        m_LoadProgress = 0.7f;

        //Deserialize scene from file (if it exists)
        if (!m_FilePath.empty())
        {
            Deserialize(m_FilePath);
        }

        m_LoadProgress = 1.0f;
    }

    // ==================== PROTECTED HELPERS ====================

    void Scene::InitializeECS()
    {
        // Initialize coordinator with event system
        m_Coordinator.Init(m_EventSystem);

        // Register all components
        m_Coordinator.RegisterComponent<Uma_ECS::Transform>();
        m_Coordinator.RegisterComponent<Uma_ECS::RigidBody>();
        m_Coordinator.RegisterComponent<Uma_ECS::Collider>();
        m_Coordinator.RegisterComponent<Uma_ECS::Sprite>();
        m_Coordinator.RegisterComponent<Uma_ECS::Camera>();
        m_Coordinator.RegisterComponent<Uma_ECS::Player>();
        m_Coordinator.RegisterComponent<Uma_ECS::Enemy>();

        // Player Controller System
        m_PlayerController = m_Coordinator.RegisterSystem<Uma_ECS::PlayerControllerSystem>();
        {
            Uma_ECS::Signature sign;
            sign.set(m_Coordinator.GetComponentType<Uma_ECS::RigidBody>());
            sign.set(m_Coordinator.GetComponentType<Uma_ECS::Transform>());
            sign.set(m_Coordinator.GetComponentType<Uma_ECS::Player>());
            m_Coordinator.SetSystemSignature<Uma_ECS::PlayerControllerSystem>(sign);
        }
        m_PlayerController->Init(m_EventSystem, m_HybridInputSystem, &m_Coordinator);

        // Physics System
        m_PhysicsSystem = m_Coordinator.RegisterSystem<Uma_ECS::PhysicsSystem>();
        {
            Uma_ECS::Signature sign;
            sign.set(m_Coordinator.GetComponentType<Uma_ECS::RigidBody>());
            sign.set(m_Coordinator.GetComponentType<Uma_ECS::Transform>());
            m_Coordinator.SetSystemSignature<Uma_ECS::PhysicsSystem>(sign);
        }
        m_PhysicsSystem->Init(&m_Coordinator);

        // Collision System
        m_CollisionSystem = m_Coordinator.RegisterSystem<Uma_ECS::CollisionSystem>();
        {
            Uma_ECS::Signature sign;
            sign.set(m_Coordinator.GetComponentType<Uma_ECS::RigidBody>());
            sign.set(m_Coordinator.GetComponentType<Uma_ECS::Transform>());
            sign.set(m_Coordinator.GetComponentType<Uma_ECS::Collider>());
            m_Coordinator.SetSystemSignature<Uma_ECS::CollisionSystem>(sign);
        }
        m_CollisionSystem->Init(&m_Coordinator);

        // Rendering System
        m_RenderingSystem = m_Coordinator.RegisterSystem<Uma_ECS::RenderingSystem>();
        {
            Uma_ECS::Signature sign;
            sign.set(m_Coordinator.GetComponentType<Uma_ECS::Sprite>());
            sign.set(m_Coordinator.GetComponentType<Uma_ECS::Transform>());
            m_Coordinator.SetSystemSignature<Uma_ECS::RenderingSystem>(sign);
        }
        m_RenderingSystem->Init(m_Graphics, m_ResourcesManager, &m_Coordinator);

        // Camera System
        m_CameraSystem = m_Coordinator.RegisterSystem<Uma_ECS::CameraSystem>();
        {
            Uma_ECS::Signature sign;
            sign.set(m_Coordinator.GetComponentType<Uma_ECS::Camera>());
            sign.set(m_Coordinator.GetComponentType<Uma_ECS::Transform>());
            m_Coordinator.SetSystemSignature<Uma_ECS::CameraSystem>(sign);
        }
        m_CameraSystem->Init(&m_Coordinator);
    }

    void Scene::UpdateECSSystems(float dt)
    {
        // Update all ECS systems
        if (m_PlayerController)
            m_PlayerController->Update(dt);

        if (m_PhysicsSystem)
            m_PhysicsSystem->Update(m_SmoothedDt);

        if (m_CollisionSystem)
            m_CollisionSystem->Update(dt);

        if (m_CameraSystem)
            m_CameraSystem->Update(dt);

        // Render
        if (m_Graphics)
        {
            m_Graphics->ClearBackground(0.2f, 0.3f, 0.3f);
        }

        if (m_RenderingSystem)
            m_RenderingSystem->Update(dt);
    }


    // ==================== SERIALIZATION ====================

    void Scene::Serialize(const std::string& filepath)
    {
        GameSerializer serializer;
        serializer.Register(m_ResourcesManager);
        serializer.Register(&m_Coordinator);
        serializer.save(filepath.empty() ? m_FilePath : filepath);
    }

    void Scene::Deserialize(const std::string& filepath)
    {
        GameSerializer serializer;
        serializer.Register(m_ResourcesManager);
        serializer.Register(&m_Coordinator);
        serializer.load(filepath.empty() ? m_FilePath : filepath);
    }
}