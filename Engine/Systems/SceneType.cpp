#include "SceneType.h"
#include "Core/GameSerializer.h"
#include <algorithm>

namespace Uma_Engine
{
    Scene::Scene(const std::string& name, const std::string& filepath, SystemManager* sm)
        : m_Name(name), m_FilePath(filepath), m_SystemManager(sm)
    {
    }

    Scene::~Scene()
    {
        if (m_State == SceneState::SCENE_RUNNING)
            Unload();
    }

    // LIFECYCLE STUFF
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

        // Wait for async load
        if (m_LoadFuture.valid())
            m_LoadFuture.wait();

        // Call OnUnload for all attached scripts
        for (auto& script : m_AttachedScripts)
        {
            script->OnUnload();
        }

        m_LuaScriptingSystem->Shutdown();

        m_UISystem->Shutdown();

        // Destroy entities
        m_Coordinator.DestroyAllEntities();

        
        // Unload resources
        if (m_ResourcesManager)
        {
            m_ResourcesManager->UnloadAllTextures();
            m_ResourcesManager->UnloadAllSound();
        }

        // Unlaod scripts

        m_State = SceneState::SCENE_UNLOADED;
        m_LoadProgress = 0.0f;
        m_FirstFrame = true;
    }

    void Scene::Update(float dt)
    {
        if (m_State != SceneState::SCENE_RUNNING)
            return;

        // Smooth delta time
        /*if (m_FirstFrame)
        {
            m_SmoothedDt = dt;
            m_FirstFrame = false;
        }
        else
        {
            m_SmoothedDt = 0.9f * m_SmoothedDt + 0.1f * dt;
        }*/

        // Cap dt to prevent spiral of death
        if (dt > g_EngineConfig.maxFrameTime)
            dt = g_EngineConfig.maxFrameTime;

        // Smooth delta time for rendering
        if (m_FirstFrame)
        {
            m_SmoothedDt = dt;
            m_FirstFrame = false;
        }
        else
        {
            m_SmoothedDt = 0.9f * m_SmoothedDt + 0.1f * dt;
        }

        // save prev pos
        m_PhysicsSystem->SavePrevPos();

        // Fixed timestep physics loop
        m_Accumulator += dt;

        int physicsSteps = 0;
        while (m_Accumulator >= m_FixedTimeStep && physicsSteps < g_EngineConfig.maxPhysicsSteps)
        {
            // Update physics and collision at fixed rate
            FixedUpdateECSSystems();

            m_Accumulator -= m_FixedTimeStep;
            physicsSteps++;
        }

        // Calculate interpolation alpha
        //float alpha = m_Accumulator / m_FixedTimeStep;

        // Update render positions for smooth interpolation
       /* auto& tfArray = m_Coordinator.GetComponentArray<Uma_ECS::Transform>();
        for (size_t i = 0; i < tfArray.Size(); ++i)
        {
            auto& tf = tfArray.GetComponentAt(i);
            tf.UpdateRenderPosition(alpha);
        }*/

        // Update ECS systems
        UpdateECSSystems(dt);

        // Update all attached scripts
        for (auto& script : m_AttachedScripts)
        {
            script->OnUpdate(dt);
        }
    }

    void Scene::UpdateSelective(float dt)
    {
        if (m_Graphics)
            m_Graphics->ClearBackground(0.2f, 0.3f, 0.3f);

        if (m_RenderingSystem)
            m_RenderingSystem->Update(dt);

        if (m_TransformSystem)
            m_TransformSystem->UpdateWorldTransform();
    }

    // SCRIPT STUFF

    void Scene::AttachScript(std::shared_ptr<SceneScript> script)
    {
        if (!script)
            return;

        // Check if script already attached
        if (HasScript(script->GetName()))
        {
            std::cout << "Script '" << script->GetName() << "' already attached to scene '" << m_Name << "'" << std::endl;
            return;
        }

        m_AttachedScripts.push_back(script);
        script->OnAttach(this);

        // If scene is already loaded, call OnLoad for the script
        if (m_State == SceneState::SCENE_RUNNING)
        {
            script->OnLoad();
        }

        std::cout << "Script '" << script->GetName() << "' attached to scene '" << m_Name << "'" << std::endl;
    }

    void Scene::DetachScript(const std::string& scriptName)
    {
        auto it = std::remove_if(m_AttachedScripts.begin(), m_AttachedScripts.end(),
            [&scriptName](const std::shared_ptr<SceneScript>& script) {
                if (script->GetName() == scriptName)
                {
                    script->OnDetach();
                    return true;
                }
                return false;
            });

        if (it != m_AttachedScripts.end())
        {
            m_AttachedScripts.erase(it, m_AttachedScripts.end());
            std::cout << "Script '" << scriptName << "' detached from scene '" << m_Name << "'" << std::endl;
        }
    }

    void Scene::DetachAllScripts()
    {
        for (auto& script : m_AttachedScripts)
        {
            script->OnDetach();
        }
        m_AttachedScripts.clear();
    }

    bool Scene::HasScript(const std::string& scriptName) const
    {
        return std::any_of(m_AttachedScripts.begin(), m_AttachedScripts.end(),
            [&scriptName](const std::shared_ptr<SceneScript>& script) {
                return script->GetName() == scriptName;
            });
    }

    std::vector<std::string> Scene::GetAttachedScriptNames() const
    {
        std::vector<std::string> names;
        for (const auto& script : m_AttachedScripts)
        {
            names.push_back(script->GetName());
        }
        return names;
    }

    // ENTITY STUFF
    Uma_ECS::Entity Scene::CreateEntity()
    {
        return m_Coordinator.CreateEntity();
    }

    void Scene::DestroyEntity(Uma_ECS::Entity entity)
    {
        m_Coordinator.DestroyEntity(entity);
    }

    // SERIALIZATION
    void Scene::Serialize(const std::string& filepath)
    {
        if (filepath.empty())
            gGameSerializer.load(m_FilePath);
    }

    void Scene::Deserialize(const std::string& filepath)
    {
        if (filepath.empty())
        {
            gGameSerializer.load(m_FilePath);
        }
    }

    // INTERNALS

    void Scene::LoadInternal()
    {
        // Get system handles
        m_HybridInputSystem = m_SystemManager->GetSystem<HybridInputSystem>();
        m_Graphics = m_SystemManager->GetSystem<Graphics>();
        m_Sound = m_SystemManager->GetSystem<Sound>();
        m_EventSystem = m_SystemManager->GetSystem<EventSystem>();
        m_ResourcesManager = m_SystemManager->GetSystem<ResourcesManager>();

        m_LoadProgress = 0.2f;

        // Initialize ECS
        InitializeECS();

        m_LoadProgress = 0.6f;

        // Call OnLoad for all attached scripts
        for (auto& script : m_AttachedScripts)
        {
            script->OnLoad();
        }

        m_LoadProgress = 0.8f;

        // Deserialize if file exists
        if (!m_FilePath.empty())
        {
            Deserialize();

            m_LuaScriptingSystem->CallStart();
        }

        m_FixedTimeStep = g_EngineConfig.fixedTimeStep;

        m_LoadProgress = 1.0f;
    }

    void Scene::InitializeECS()
    {
        m_Coordinator.Init(m_EventSystem);

        // Register components
        m_Coordinator.RegisterComponent<Uma_ECS::Transform>();
        m_Coordinator.RegisterComponent<Uma_ECS::RigidBody>();
        m_Coordinator.RegisterComponent<Uma_ECS::Collider>();
        m_Coordinator.RegisterComponent<Uma_ECS::Sprite>();
        m_Coordinator.RegisterComponent<Uma_ECS::Camera>();
        m_Coordinator.RegisterComponent<Uma_ECS::Player>();
        m_Coordinator.RegisterComponent<Uma_ECS::Enemy>();
        m_Coordinator.RegisterComponent<Uma_ECS::LuaScript>();
        m_Coordinator.RegisterComponent<Uma_ECS::Animator>();
        m_Coordinator.RegisterComponent<Uma_UI::RectTransform>();
        m_Coordinator.RegisterComponent<Uma_UI::Canvas>();
        m_Coordinator.RegisterComponent<Uma_UI::Image>();
        m_Coordinator.RegisterComponent<Uma_UI::Button>();
        m_Coordinator.RegisterComponent<Uma_UI::Text>();

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

        // Transform System
        m_TransformSystem = m_Coordinator.RegisterSystem<Uma_ECS::TransformSystem>();
        {
            Uma_ECS::Signature sign;
            sign.set(m_Coordinator.GetComponentType<Uma_ECS::Transform>());
            m_Coordinator.SetSystemSignature<Uma_ECS::TransformSystem>(sign);
        }
        m_TransformSystem->Init(&m_Coordinator);

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
        m_CollisionSystem->Init(&m_Coordinator, m_EventSystem, m_Graphics);

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

        m_LuaScriptingSystem = m_Coordinator.RegisterSystem<Uma_ECS::LuaScriptingSystem>();
        {
            Uma_ECS::Signature sign;
            sign.set(m_Coordinator.GetComponentType<Uma_ECS::LuaScript>());
            m_Coordinator.SetSystemSignature<Uma_ECS::LuaScriptingSystem>(sign);
        }
        m_LuaScriptingSystem->Init(&m_Coordinator, m_EventSystem, m_HybridInputSystem);

        InitializeUISystem();

        gGameSerializer.Register(m_ResourcesManager);
        gGameSerializer.Register(&m_Coordinator);
    }

    void Scene::InitializeUISystem()
    {
        m_UISystem = m_Coordinator.RegisterSystem<Uma_UI::UISystem>();
        {
            // 1. RectTransform  (mandatory for layout)
            // 2. Image OR Text  (at least one drawable)
            // 3. Button         (optional interactable)
            Uma_ECS::Signature sign;
            sign.set(m_Coordinator.GetComponentType<Uma_UI::RectTransform>());
            m_Coordinator.SetSystemSignature<Uma_UI::UISystem>(sign);
        }

        m_UISystem->SetCoordinator(&m_Coordinator);
        m_UISystem->SetEventSystem(m_EventSystem);
        m_UISystem->SetGraphics(m_Graphics);
        m_UISystem->SetResourcesManager(m_ResourcesManager);
        m_UISystem->Init();
    }

    void Scene::UpdateECSSystems(float dt)
    {
        if (m_PlayerController)
            m_PlayerController->Update(dt);

        if (m_LuaScriptingSystem)
            m_LuaScriptingSystem->Update(dt);

        if (m_CameraSystem)
            m_CameraSystem->Update(dt);

        if (m_Graphics)
            m_Graphics->ClearBackground(0.2f, 0.3f, 0.3f);

        if (m_RenderingSystem)
            m_RenderingSystem->Update(dt);

        if (m_CollisionSystem)
            m_CollisionSystem->DebugRender();
    }

    void Scene::FixedUpdateECSSystems()
    {
        // Physics runs at FIXED timestep
        if (m_PhysicsSystem)
            m_PhysicsSystem->Update(m_FixedTimeStep);

        if (m_TransformSystem)
            m_TransformSystem->UpdateWorldTransform();

        if (m_PhysicsSystem)
            m_PhysicsSystem->ApplyVelocity(m_FixedTimeStep);

        // Collision detection runs at FIXED timestep
        if (m_CollisionSystem)
            m_CollisionSystem->Update(m_FixedTimeStep);
    }
}