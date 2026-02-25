/*!
\file   SceneType.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Shahir Rasid (everything else)
\par    E-mail: b.muhammadshahir@digipen.edu
\par    DigiPen login: b.muhammadshahir

\author Leong Wai Men (all ECS related)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
This file implements the definitions for a base class of scene in that
anything that wants to be a scene should inherit from this class.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/
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

        gGameSerializer.ShutDown();

        // shut down Coodinator
        m_Coordinator.ShutDown();

        m_PlayerController->Shutdown();

        m_TilemapSystem->Shutdown();

        m_ProjectileSystem->Shutdown();

        m_LuaScriptingSystem->Shutdown();
        
        m_UISystem->Shutdown();

        m_AudioSystem->Shutdown();

        m_PathFindingSystem->Shutdown();

        // Unload resources
        if (m_ResourcesManager)
        {
            m_ResourcesManager->UnloadAllTextures();
            m_ResourcesManager->UnloadAllSound();
            m_ResourcesManager->UnloadAllFonts();
        }

        // Unlaod scripts

        m_State = SceneState::SCENE_UNLOADED;
        m_LoadProgress = 0.0f;
    }

    void Scene::Update(float dt)
    {
        if (m_State != SceneState::SCENE_RUNNING)
            return;

        // Cap dt to prevent spiral of death
        if (dt > g_EngineConfig.maxFrameTime)
            dt = g_EngineConfig.maxFrameTime;

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

        // Update ECS systems
        UpdateECSSystems(dt);

        // Update all attached scripts
        for (auto& script : m_AttachedScripts)
        {
            script->OnUpdate(dt);
        }

        m_Coordinator.ProcessDeletionQueue();
    }

    void Scene::UpdateSelective(float dt)
    {
        if (m_TransformSystem)
            m_TransformSystem->UpdateWorldTransform();

        if (m_RenderingSystem)
            m_RenderingSystem->Update(dt, false);

        if (m_ParticleSystem)
            m_ParticleSystem->Update(dt);

        if (m_AnimatorSystem)
            m_AnimatorSystem->Update(dt);

        if (m_UISystem)
            m_UISystem->Update(dt);

        if (m_CollisionSystem)
            m_CollisionSystem->DebugRender();

        if (m_CameraSystem)
            m_CameraSystem->Update(dt);

        m_Coordinator.ProcessDeletionQueue();
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
        m_Sound = m_SystemManager->GetSystem<SoundManager>();
        m_EventSystem = m_SystemManager->GetSystem<EventSystem>();
        m_ResourcesManager = m_SystemManager->GetSystem<ResourcesManager>();
        m_EditorSystem = m_SystemManager->GetSystem<EditorSystem>();

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

            // 1/2/2026
            // wm commented this code
            // bc loading a scene is not supposed to trigger the script function
            //m_LuaScriptingSystem->CallStart();
        }

        // cache the scene before starting the scene
        m_Coordinator.CacheState();

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
        m_Coordinator.RegisterComponent<Uma_ECS::AudioListener>();
        m_Coordinator.RegisterComponent<Uma_ECS::AudioComponent>();
        m_Coordinator.RegisterComponent<Uma_ECS::PathFinding>();
        m_Coordinator.RegisterComponent<Uma_ECS::Prefab>();
        m_Coordinator.RegisterComponent<Uma_ECS::Projectile>();
        m_Coordinator.RegisterComponent<Uma_UI::RectTransform>();
        m_Coordinator.RegisterComponent<Uma_UI::Canvas>();
        m_Coordinator.RegisterComponent<Uma_UI::Image>();
        m_Coordinator.RegisterComponent<Uma_UI::Button>();
        m_Coordinator.RegisterComponent<Uma_UI::Slider>();
        m_Coordinator.RegisterComponent<Uma_UI::Checkbox>();
        m_Coordinator.RegisterComponent<Uma_UI::Text>();
        m_Coordinator.RegisterComponent<Uma_UI::Effects>();
        m_Coordinator.RegisterComponent<Uma_ECS::ParticleEmitter>();
        m_Coordinator.RegisterComponent<Uma_ECS::Tilemap>();
        m_Coordinator.RegisterComponent<Uma_ECS::FSM>();
        
        // Player Controller System
        m_PlayerController = m_Coordinator.RegisterSystem<Uma_ECS::PlayerControllerSystem>();
        {
            Uma_ECS::Signature sign;
            sign.set(m_Coordinator.GetComponentType<Uma_ECS::RigidBody>());
            sign.set(m_Coordinator.GetComponentType<Uma_ECS::Transform>());
            sign.set(m_Coordinator.GetComponentType<Uma_ECS::Player>());
            sign.set(m_Coordinator.GetComponentType<Uma_ECS::PathFinding>());
            m_Coordinator.SetSystemSignature<Uma_ECS::PlayerControllerSystem>(sign);
        }
        m_PlayerController->Init(m_EventSystem, m_HybridInputSystem, &m_Coordinator, m_Graphics);

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

        // Projectile System
        m_ProjectileSystem = m_Coordinator.RegisterSystem<Uma_ECS::ProjectileSystem>();
        {
            Uma_ECS::Signature sign;
            sign.set(m_Coordinator.GetComponentType<Uma_ECS::RigidBody>());
            sign.set(m_Coordinator.GetComponentType<Uma_ECS::Transform>());
            sign.set(m_Coordinator.GetComponentType<Uma_ECS::Projectile>());
            m_Coordinator.SetSystemSignature<Uma_ECS::ProjectileSystem>(sign);
        }
        m_ProjectileSystem->Init(&m_Coordinator, m_EventSystem);

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

        // Animator System
        m_AnimatorSystem = m_Coordinator.RegisterSystem<Uma_ECS::AnimatorSystem>();
        {
            Uma_ECS::Signature sign;
            sign.set(m_Coordinator.GetComponentType<Uma_ECS::Animator>());
            m_Coordinator.SetSystemSignature<Uma_ECS::AnimatorSystem>(sign);
        }
        m_AnimatorSystem->Init(&m_Coordinator, m_ResourcesManager);

        m_LuaScriptingSystem = m_Coordinator.RegisterSystem<Uma_ECS::LuaScriptingSystem>();
        {
            Uma_ECS::Signature sign;
            sign.set(m_Coordinator.GetComponentType<Uma_ECS::LuaScript>());
            m_Coordinator.SetSystemSignature<Uma_ECS::LuaScriptingSystem>(sign);
        }
        m_LuaScriptingSystem->Init(&m_Coordinator, m_EventSystem, m_HybridInputSystem, m_ResourcesManager, m_Graphics);

        //Audio system
        m_AudioSystem = m_Coordinator.RegisterSystem<Uma_ECS::AudioSystem>();
        {
            Uma_ECS::Signature sign;
            sign.set(m_Coordinator.GetComponentType<Uma_ECS::RigidBody>());
            sign.set(m_Coordinator.GetComponentType<Uma_ECS::Transform>());
            //sign.set(m_Coordinator.GetComponentType<Uma_ECS::AudioListener>());
            sign.set(m_Coordinator.GetComponentType<Uma_ECS::AudioComponent>());
            m_Coordinator.SetSystemSignature<Uma_ECS::AudioSystem>(sign);
        }
        m_AudioSystem->Init(m_Sound, &m_Coordinator, m_EventSystem, m_ResourcesManager);

        //path finding system
        m_PathFindingSystem = m_Coordinator.RegisterSystem<Uma_ECS::PathFindingSystem>(); 
        {
            Uma_ECS::Signature sign;
            sign.set(m_Coordinator.GetComponentType<Uma_ECS::RigidBody>());
            sign.set(m_Coordinator.GetComponentType<Uma_ECS::Transform>());
            sign.set(m_Coordinator.GetComponentType<Uma_ECS::PathFinding>());

            m_Coordinator.SetSystemSignature<Uma_ECS::PathFindingSystem>(sign);
        }
        m_PathFindingSystem->Init(&m_Coordinator, m_EventSystem, m_Graphics);

        // Particle System
        m_ParticleSystem = m_Coordinator.RegisterSystem<Uma_ECS::ParticleSystem>();
        {
            Uma_ECS::Signature sign;
            sign.set(m_Coordinator.GetComponentType<Uma_ECS::ParticleEmitter>());
            sign.set(m_Coordinator.GetComponentType<Uma_ECS::Transform>());
            m_Coordinator.SetSystemSignature<Uma_ECS::ParticleSystem>(sign);
        }
        m_ParticleSystem->Init(m_Graphics, m_ResourcesManager, &m_Coordinator);

        // tilemap 
        m_TilemapSystem = m_Coordinator.RegisterSystem<Uma_ECS::TilemapSystem>();
        {
            Uma_ECS::Signature sign;
            sign.set(m_Coordinator.GetComponentType<Uma_ECS::Transform>());
            sign.set(m_Coordinator.GetComponentType<Uma_ECS::Tilemap>());
            m_Coordinator.SetSystemSignature<Uma_ECS::TilemapSystem>(sign);
        }
        m_TilemapSystem->Init(&m_Coordinator, m_Graphics, m_ResourcesManager);
        
        //FSM
        m_FSMSystem = m_Coordinator.RegisterSystem<Uma_ECS::FSMSystem>();
        {
            Uma_ECS::Signature sign;
            sign.set(m_Coordinator.GetComponentType<Uma_ECS::LuaScript>());
            sign.set(m_Coordinator.GetComponentType<Uma_ECS::FSM>());

            m_Coordinator.SetSystemSignature<Uma_ECS::FSMSystem>(sign);
        }
        m_FSMSystem->Init(&m_Coordinator);

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
        //if (m_PlayerController)
        //    m_PlayerController->Update(m_FixedTimeStep);
        if (m_AnimatorSystem)
            m_AnimatorSystem->Update(dt);

        if (m_LuaScriptingSystem)
            m_LuaScriptingSystem->Update(dt);

        if (m_CameraSystem)
            m_CameraSystem->Update(dt);

       /* if (m_Graphics)
            m_Graphics->ClearBackground(0.2f, 0.3f, 0.3f);*/

        if (m_RenderingSystem)
            m_RenderingSystem->Update(dt, true);

        if (m_TilemapSystem)
            m_TilemapSystem->Update(dt);


        if (m_ParticleSystem)
            m_ParticleSystem->Update(dt);

        if (m_CollisionSystem)
            m_CollisionSystem->DebugRender();

        if (m_UISystem)
        {
            m_UISystem->Update(dt);
            m_UISystem->InputPass();
        }

        if (m_AudioSystem)
            m_AudioSystem->Update(dt);

        if (m_FSMSystem)
            m_FSMSystem->Update(dt);
    }

    void Scene::FixedUpdateECSSystems()
    {
        // Physics runs at FIXED timestep

        if (m_PlayerController)
            m_PlayerController->Update(m_FixedTimeStep);

        if (m_PathFindingSystem)
            m_PathFindingSystem->Update(m_FixedTimeStep);

        if (m_PhysicsSystem)
            m_PhysicsSystem->Update(m_FixedTimeStep);

        if (m_ProjectileSystem)
            m_ProjectileSystem->Update(m_FixedTimeStep);

        if (m_TransformSystem)
            m_TransformSystem->UpdateWorldTransform();

        if (m_PhysicsSystem)
            m_PhysicsSystem->ApplyVelocity(m_FixedTimeStep);

        // Collision detection runs at FIXED timestep
        if (m_CollisionSystem)
            m_CollisionSystem->Update(m_FixedTimeStep);
    }
}