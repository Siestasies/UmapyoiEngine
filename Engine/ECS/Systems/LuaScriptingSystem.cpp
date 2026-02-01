/*!
\file   LuaScriptingSystem.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines Lua scripting system for entity behavior execution and script lifecycle management.

Manages initialization, update, and shutdown of Lua scripts attached to entities. Provides
comprehensive Lua API bindings including entity queries, component access, input handling,
cross-entity manipulation, and utility functions. Handles collision/trigger callback events
(OnCollisionEnter/Exit, OnTriggerEnter/Exit) via EventSystem subscription. Maintains isolated
Sol2 environments per script instance with variable synchronization between C++ and Lua. Supports
script hot-reloading, exposed variable discovery, and entity destruction cleanup. Integrates with
HybridInputSystem for input bindings and EventSystem for game events.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "LuaScriptingSystem.hpp"
#include "AudioSystem.hpp"

#include "../Components/Transform.h"
#include "../Components/RigidBody.h"
#include "../Components/Sprite.h"
#include "../Components/Collider.h"
#include "../Components/Camera.h"
#include "../Components/Player.h"
#include "../Components/Enemy.h"
#include "../Components/FSM.h"
#include "../UI/Components/Text.h"

#include "Events/ApplicationEvents.h"

#include <functional>

// temp
#include <rapidjson/istreamwrapper.h>
#include "Core/FilePaths.h"
#include "Application.h"

namespace Uma_ECS
{
    
    void LuaScriptingSystem::Init(Coordinator* c, Uma_Engine::EventSystem* e, Uma_Engine::HybridInputSystem* i, Uma_Engine::ResourcesManager* r)
    {
        // linking the Engine systems 
        pCoordinator = c;
        pEventSystem = e;
        pInputSystem = i;
        pResourcesManager = r;

        // create shared Lua state with all standard libraries
        sharedLua = std::make_shared<sol::state>();
        sharedLua->open_libraries(
            sol::lib::base,
            sol::lib::math,
            sol::lib::string,
            sol::lib::table,
            sol::lib::package
        );

        //Set up package.path for require()
        std::string currentPath = (*sharedLua)["package"]["path"];
        (*sharedLua)["package"]["path"] = currentPath +
            ";./Assets/Scripts/?.lua" +
            ";./Assets/Scripts/States/?.lua";  // Add path for your state files

        RegisterLuaAPI();

        SubscribeToEvents();
    }

    void LuaScriptingSystem::Restart()
    {
        Shutdown();

        sharedLua = std::make_shared<sol::state>();
        sharedLua->open_libraries(
            sol::lib::base,
            sol::lib::math,
            sol::lib::string,
            sol::lib::table,
            sol::lib::package
        );

        //Set up package.path for require()
        std::string currentPath = (*sharedLua)["package"]["path"];
        (*sharedLua)["package"]["path"] = currentPath +
            ";./Assets/Scripts/?.lua" +
            ";./Assets/Scripts/States/?.lua";  // Add path for your state files

        RegisterLuaAPI();
        SubscribeToEvents();  // Re-subscribe to events after restart
    }

    void LuaScriptingSystem::Update(float dt)
    {
        auto& scriptArray = pCoordinator->GetComponentArray<LuaScript>();

        for (auto const& entity : aEntities)
        {
            if (!pCoordinator->IsActiveInHierarchy(entity))
                continue;

            auto& scriptComponent = scriptArray.GetData(entity);

            // Initialize scripts if needed
            /* if (!scriptComponent.lua || !scriptComponent.lua->lua_state())
            {
                InitializeEntityScripts(entity, scriptComponent);
            }*/

            // update each script instance
            for (auto& script : scriptComponent.scripts)
            {
                if (script.hasError)
                    continue;

                if (!script.isEnabled)
                {
                    if (script.wasEnabledLastFrame)
                    {
                        //CallLuaFunction(script, "OnDisable");
                        script.wasEnabledLastFrame = false;
                    }
                    continue;
                }

                if (!script.wasEnabledLastFrame)
                {
                    script.isVariableDirty = true;  // Force sync when enabled
                    script.wasEnabledLastFrame = true;

                    if (!script.isInitialized)
                    {
                        InitializeEntityScript(entity, script);
                    }

                    //CallLuaFunction(script, "OnEnable");
                }

                if (!script.isInitialized)
                {
                    InitializeEntityScript(entity, script);
                }

                if (script.isVariableDirty) // oni update when there is changes being made
                {
                    // sync c++ variables to lua
                    SyncVariablesToLua(script);
                    script.isVariableDirty = false;
                }

                // call update
                CallLuaFunction(script, "Update", dt);

                // sync lua variables back to c++
                SyncVariablesFromLua(script);
            }
        }
        lastDeltaTime = dt;
    }

    void LuaScriptingSystem::InitializeAllScripts()
    {
        auto& scriptArray = pCoordinator->GetComponentArray<LuaScript>();

        for (auto const& entity : aEntities)
        {
            auto& scriptComponent = scriptArray.GetData(entity);

            InitializeEntityScripts(entity, scriptComponent);
        }
    }

    void LuaScriptingSystem::StartScripts()
    {
        auto& scriptArray = pCoordinator->GetComponentArray<LuaScript>();

        for (auto const& entity : aEntities)
        {
            auto& scriptComponent = scriptArray.GetData(entity);

            InitializeEntityScripts(entity, scriptComponent);

            for (size_t i = 0; i < scriptComponent.scripts.size(); i++)
            {
                if (!scriptComponent.scripts[i].isEnabled)
                {
                    continue;
                }

                // refresh the varaibles of the script
                RefreshScript(entity, i);
                
                // call start on the lua script
                CallLuaFunction(scriptComponent.scripts[i], "Start");
            }
        }
    }

    void LuaScriptingSystem::Shutdown()
    {
        Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo,
            "LuaScriptingSystem::Shutdown() starting");

        if (!pCoordinator)
        {
            sharedLua = nullptr;
            return;
        }

        UnsubscribeEvents();

        bool luaValid = sharedLua && sharedLua->lua_state();

        auto& scriptArray = pCoordinator->GetComponentArray<LuaScript>();
        std::vector<Entity> allEntities;
        allEntities.reserve(scriptArray.Size());

        for (size_t i = 0; i < scriptArray.Size(); ++i)
            allEntities.push_back(scriptArray.GetEntity(i));

        Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo,
            "Found " + std::to_string(allEntities.size()) + " entities with scripts");

        if (luaValid)
        {
            for (auto const& entity : allEntities)
            {
                if (!scriptArray.Has(entity)) continue;
                auto& scriptComponent = scriptArray.GetData(entity);

                for (auto& script : scriptComponent.scripts)
                {
                    if (script.isEnabled && script.isInitialized)
                    {
                        try
                        {
                            CallLuaFunction(script, "OnDestroy");
                        }
                        catch (...) {}
                    }

                    // Clear and reset the environment safely
                    if (script.scriptEnv)
                    {
                        try
                        {
                            script.scriptEnv->clear();
                            script.scriptEnv.reset();
                        }
                        catch (...) {}
                    }

                    script.isInitialized = false;
                    script.hasError = false;
                }

                scriptComponent.scripts.clear();
            }

            try
            {
                sharedLua->collect_garbage();
                sharedLua->collect_garbage();
            }
            catch (...) {}
        }

        sharedLua = nullptr;

        Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo,
            "LuaScriptingSystem::Shutdown() completed");
    }


    void LuaScriptingSystem::RegisterLuaAPI()
    {
        // this function is basically registering all the type
        // eg. transform, rigidbody, Vec2 and any helper type
        // the lua script needs

        // u need to register any class struct yall need 

        // Register Vec2
        sharedLua->new_usertype<Vec2>("Vec2",
            // Constructors
            sol::constructors<Vec2(), Vec2(float, float)>(),

            // Properties
            "x", &Vec2::x,
            "y", &Vec2::y,

            // Operators
            sol::meta_function::addition, [](const Vec2& a, const Vec2& b) { return a + b; },
            sol::meta_function::subtraction, [](const Vec2& a, const Vec2& b) { return a - b; },
            sol::meta_function::multiplication, sol::overload(
                [](const Vec2& v, float s) { return v * s; },
                [](float s, const Vec2& v) { return v * s; }
            ),
            sol::meta_function::division, sol::overload(
                [](const Vec2& v, float s) { return Vec2(v.x / s, v.y / s); }
            ),

            // String representation for debugging
            sol::meta_function::to_string, [](const Vec2& v) {
                return "Vec2(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ")";
            }
        );

        // IMPORTANT: Create a global helper function for Vec2 construction
        sharedLua->set_function("Vec2", sol::overload(
            []() { return Vec2(); },
            [](float x, float y) { return Vec2(x, y); }
        ));

        sharedLua->new_usertype<std::vector<Entity>>("EntityVector",
            sol::meta_function::length, &std::vector<Entity>::size,
            sol::meta_function::index, [](std::vector<Entity>& vec, int index) -> Entity {
                if (index < 1 || index > static_cast<int>(vec.size())) {
                    return static_cast<Entity>(-1);
                }
                return vec[index - 1];
            }
        );


        // need to add more (tf rb for testing now)
        // more...

        // Register Component types
        RegisterComponentTypes();

        // Input System Functions
        RegisterInputBindings();    // register the key press functions
        RegisterKeyConstants();     // register the availables keys

        RegisterUtilityFUnctions();
        RegisterCrossEntityAccess();
        RegisterEntityQueries();
        RegisterEntityManipulation();
    }

    void LuaScriptingSystem::RegisterEntityManipulation()
    {
        sharedLua->set_function("CreateEntity", [&]() 
            {
                try
                {
                    Entity e = pCoordinator->CreateEntity();
                    std::string debug = "created entity : " + std::to_string(e);
                    Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo, debug);

                    return e;
                }
                catch (...)
                {
                    std::string debug = "failed to create entity : ";
                    Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo, debug);

                    return static_cast<Entity>(-1);
                }
                
            });

        sharedLua->set_function("DestroyEntity", [&](const Entity& entity)
            {
                try
                {
                    pCoordinator->DestroyEntity(entity);
                    std::string debug = "destroyed entity : " + std::to_string(entity);
                    Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo, debug);
                }
                catch (...)
                {
                    std::string debug = "failed to destroy entity : " + std::to_string(entity);
                    Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo, debug);
                }
            });

        sharedLua->set_function("SetParent", [&](Entity child, Entity parent) {
            try {
                pCoordinator->SetParent(child, parent);
            }
            catch (...) {
                Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eError,
                    "Failed to set parent relationship");
            }
            });

        sharedLua->set_function("RemoveParent", [&](Entity child) {
            try {
                pCoordinator->RemoveParent(child);
            }
            catch (...) {
                Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eError,
                    "Failed to remove parent");
            }
            });

        sharedLua->set_function("GetParent", [&](Entity entity) -> int {
            auto parent = pCoordinator->GetParent(entity);
            return parent.has_value() ? static_cast<int>(parent.value()) : -1;
            });

        sharedLua->set_function("HasParent", [&](Entity entity) -> bool {
            return pCoordinator->GetParent(entity).has_value();
            });

        sharedLua->set_function("GetChildren", [&](Entity entity) -> std::vector<Entity> {
            return pCoordinator->GetChildren(entity);
            });

        sharedLua->set_function("DestroyWithChildren", [&](Entity entity) {
            try {
                pCoordinator->DestroyEntityAndChildren(entity);
            }
            catch (...) {
                Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eError,
                    "Failed to destroy entity hierarchy");
            }
            });

        sharedLua->set_function("SetActiveEntity", [this](Uma_ECS::Entity entity, bool isActive)
            {
                if (pCoordinator->HasActiveEntity(entity))
                {
                    pCoordinator->SetActive(entity, isActive);
                }
            });

        sharedLua->set_function("GetActiveEntity", [this](Uma_ECS::Entity entity) -> bool
            {
               return  pCoordinator->IsActiveInHierarchy(entity);
            });

        // temp animator exposure
        sharedLua->set_function("PlayAnimation", [this](Entity entity, std::string name)
            {
                if (!pCoordinator->HasActiveEntity(entity)) return;
                if (!pCoordinator->IsActiveInHierarchy(entity)) return;

                auto& animator = pCoordinator->GetComponent<Animator>(entity);

                animator.animator.Play(name, true);
            
            });

        // Add Force
        sharedLua->set_function("AddForce", [this](Entity entity, Vec2 pos, Vec2 dir, float force, float rotation)
            {
                if (!pCoordinator->HasActiveEntity(entity)) return;
                if (!pCoordinator->IsActiveInHierarchy(entity)) return;

                auto& tf = pCoordinator->GetComponent<Transform>(entity);
                auto& rb = pCoordinator->GetComponent<RigidBody>(entity);


                tf.position = pos;
                tf.rotation.x = rotation;

                rb.velocity = dir;
                rb.velocity.normalize();

                rb.velocity *= force;
            });

        // path finding set goal
        sharedLua->set_function("SetPathFindingGoal", [this](Uma_ECS::Entity entity, float x, float y)
            {
                if (pCoordinator->HasComponent<PathFinding>(entity))
                {
                    auto& pf = pCoordinator->GetComponent<PathFinding>(entity);

                    pf.goal = Vec2(x, y);
                }
            });

        // add component remove component

        // Load prefab from file
        // this method of doing is very ugly 
        // but it requires the need of refactoring the Gameserializer and coordinator 
        // to provide a better way to handle serializing and deserializing
        // WIP
        // prefab name must include file extention
        sharedLua->set_function("SpawnPrefab", [&](const std::string& prefabName, Vec2 pos) -> Entity
            {
                // Safety check
                if (!pResourcesManager) {
                    Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eError, "SpawnPrefab Failed: ResourcesManager is null");
                    return static_cast<Entity>(-1);
                }

                // Load/Get cached Prefab JSON via ResourcesManager
                std::string prefabPath = Uma_FilePath::PREFAB_DIR + prefabName;
                auto docPtr = pResourcesManager->GetPrefab(prefabPath);

                if (!docPtr) {
                    Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eError, "Failed to load prefab: " + prefabPath);
                    return static_cast<Entity>(-1);
                }

                const rapidjson::Document& doc = *docPtr;

                if (doc.HasMember("resources")) {
                    pResourcesManager->Deserialize(doc["resources"]);
                }

                // Instantiate Entity
                if (!doc.HasMember("Prefab")) {
                    Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eError, "Invalid prefab format: " + prefabPath);
                    return static_cast<Entity>(-1);
                }

                try {
                    Entity rootEntity = pCoordinator->DeserializePrefab(doc["Prefab"]);

                    // Apply override position
                    if (rootEntity != static_cast<Entity>(-1)) {
                        if (pCoordinator->HasComponent<Transform>(rootEntity)) {
                            auto& tf = pCoordinator->GetComponent<Transform>(rootEntity);
                            tf.position = pos;
                        }

                        // Log success
                        Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo, "Spawned prefab: " + prefabName);
                    }
                    return rootEntity;
                }
                catch (const std::exception& e) {
                    Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eError, "Exception spawning prefab: " + std::string(e.what()));
                    return static_cast<Entity>(-1);
                }
            });
    }

    void LuaScriptingSystem::RegisterComponentTypes()
    {
        // Register Transform
        sharedLua->new_usertype<Transform>("Transform",
            "position", &Transform::position,
            "rotation", &Transform::rotation,
            "scale", &Transform::scale,
            "worldPosition", &Transform::worldPosition
        );

        // Register RigidBody
        sharedLua->new_usertype<RigidBody>("RigidBody",
            "velocity", &RigidBody::velocity,
            "acceleration", &RigidBody::acceleration,
            "accel_strength", &RigidBody::accel_strength,
            "fric_coeff", &RigidBody::fric_coeff
        );

        // Register Sprite
        sharedLua->new_usertype<Sprite>("Sprite",
            "texturePath", &Sprite::texturePath,
            "renderLayer", &Sprite::renderLayer,
            "flipX", &Sprite::flipX,
            "flipY", &Sprite::flipY,
            "autoFlip", &Sprite::autoFlip
        );

        // Register Player component
        sharedLua->new_usertype<Player>("Player",
            "mHealth"         ,&Player::mHealth,
            "mMaxHealth"      ,&Player::mMaxHealth,
            "mHealthRegenRate",&Player::mHealthRegenRate,
            "mSpeed"          ,&Player::mSpeed,
            "mDashSpeed"      ,&Player::mDashSpeed,
            "mDashCD"         ,&Player::mDashCD,
            "mAttackDamage"   ,&Player::mAttackDamage,
            "mAttackSpeed"    ,&Player::mAttackSpeed,
            "mAttackRange"    ,&Player::mAttackRange,
            "mDefense"        ,&Player::mDefense,
            "mMana"           ,&Player::mMana,
            "mMaxMana"        ,&Player::mMaxMana,
            "mManaRegenRate"  ,&Player::mManaRegenRate
        );


        // Register Enemy component
        sharedLua->new_usertype<Enemy>("Enemy",
            "mHealth"         ,&Enemy::mHealth,
            "mMaxHealth"      ,&Enemy::mMaxHealth,
            "mHealthRegenRate",&Enemy::mHealthRegenRate,
            "mSpeed"          ,&Enemy::mSpeed,
            "mAttackDamage"   ,&Enemy::mAttackDamage,
            "mAttackSpeed"    ,&Enemy::mAttackSpeed,
            "mAttackRange"    ,&Enemy::mAttackRange,
            "mDefense"        ,&Enemy::mDefense
        );

        // ProjectileType
        sharedLua->new_enum<ProjectileType>("ProjectileType",
            {
                {"AOE", P_AOE},
                {"SINGLE", P_SINGLE}
            }
        );

        // ProjectileStats
        sharedLua->new_usertype<ProjectileStats>("ProjectileStats",
            "type", &ProjectileStats::type,
            "damage", &ProjectileStats::damage,
            "speed", &ProjectileStats::speed,
            "fadeOVerTime", &ProjectileStats::fadeOVerTime,
            "lifeTime", &ProjectileStats::lifeTime
        );

        // Projectile
        sharedLua->new_usertype<Projectile>("Projectile",
            "mStats", &Projectile::mStats
        );

        // Register SpriteAnimator first (the engine class)
        sharedLua->new_usertype<Uma_Engine::SpriteAnimator>("SpriteAnimator",
            // Methods that take the animator instance as first parameter
            "Play", [](Uma_Engine::SpriteAnimator& animator, const std::string& name, bool restart) {
                animator.Play(name, restart);
            },

            "IsPlaying", [](Uma_Engine::SpriteAnimator& animator) -> bool {
                return animator.IsPlaying();
            },

            "HasFinished", [](Uma_Engine::SpriteAnimator& animator) -> bool {
                return animator.HasFinished();
            },

            "GetCurrentClip", [](const Uma_Engine::SpriteAnimator& animator) -> std::string {
                return animator.GetCurrentClip();
            },

            "Reset", [](Uma_Engine::SpriteAnimator& animator) {
                animator.Reset();
            },

            "AddClip", sol::overload(
                [](Uma_Engine::SpriteAnimator& animator, const std::string& name,
                    int framesX, int framesY, int startFrame, int frameCount,
                    float fps, bool loop) {
                        animator.AddClip(name, framesX, framesY, startFrame, frameCount, fps, loop);
                },
                [](Uma_Engine::SpriteAnimator& animator, const std::string& name,
                    int framesX, int framesY, int startFrame, int frameCount, float fps) {
                        animator.AddClip(name, framesX, framesY, startFrame, frameCount, fps);
                },
                [](Uma_Engine::SpriteAnimator& animator, const std::string& name,
                    int framesX, int framesY, int startFrame, int frameCount) {
                        animator.AddClip(name, framesX, framesY, startFrame, frameCount);
                }
            ),

            "RemoveClip", [](Uma_Engine::SpriteAnimator& animator, const std::string& name) -> bool {
                return animator.RemoveClip(name);
            }
        );

        // Register Animator component (the ECS component wrapper)
        sharedLua->new_usertype<Animator>("Animator",
            // Direct member access
            "autoPlay", &Animator::autoPlay,
            "initialClip", &Animator::initialClip,
            "isInitialized", &Animator::isInitialized,
            "uvOffset", &Animator::uvOffset,
            "uvSize", &Animator::uvSize,

            // Access to the SpriteAnimator instance
            "animator", &Animator::animator
        );

        //Register Text component
        sharedLua->new_usertype<Uma_UI::Text>("Text",
            "text", &Uma_UI::Text::text,
            "visible", &Uma_UI::Text::visible
        );

        // Register Camera
        sharedLua->new_usertype<Camera>("Camera",
            "zoom", &Camera::mZoom,
            "followPlayer", &Camera::followPlayer
        );

        sharedLua->new_usertype<PathFinding>("PathFinding",
            "goal", &PathFinding::goal,
            "reachedGoal", &PathFinding::reachedGoal
        );

        // ===================================================================
        // COLLISION SYSTEM TYPES
        // ===================================================================

        // Collision Layers
        sol::table collisionLayerTable = sharedLua->create_table("CollisionLayer");
        collisionLayerTable["NONE"] = CL_NONE;
        collisionLayerTable["DEFAULT"] = CL_DEFAULT;
        collisionLayerTable["PLAYER"] = CL_PLAYER;
        collisionLayerTable["ENEMY"] = CL_ENEMY;
        collisionLayerTable["WALL"] = CL_WALL;
        collisionLayerTable["PROJECTILE"] = CL_PROJECTILE;
        collisionLayerTable["PICKUP"] = CL_PICKUP;
        collisionLayerTable["ALL"] = CL_ALL;

        // Collider Purpose
        sol::table colliderPurposeTable = sharedLua->create_table("ColliderPurpose");
        colliderPurposeTable["Physics"] = static_cast<int>(ColliderPurpose::Physics);
        colliderPurposeTable["Environment"] = static_cast<int>(ColliderPurpose::Environment);
        colliderPurposeTable["Trigger"] = static_cast<int>(ColliderPurpose::Trigger);

        // BoundingBox
        sharedLua->new_usertype<BoundingBox>("BoundingBox",
            "min", &BoundingBox::min,
            "max", &BoundingBox::max
        );

        // ColliderShape - MUST be registered BEFORE the vector
        sharedLua->new_usertype<ColliderShape>("ColliderShape",
            sol::constructors<ColliderShape()>(),
            "size", &ColliderShape::size,
            "offset", &ColliderShape::offset,
            "purpose", &ColliderShape::purpose,
            "layer", &ColliderShape::layer,
            "colliderMask", &ColliderShape::colliderMask,
            "isActive", &ColliderShape::isActive,
            "autoFitToSprite", &ColliderShape::autoFitToSprite
        );

        // ===================================================================
        // VECTOR REGISTRATION - Use sol::as_container
        // ===================================================================

        // This tells Sol2 to treat std::vector<ColliderShape> as a container
        // No explicit usertype needed - Sol2 handles it automatically

        // ===================================================================
        // COLLIDER REGISTRATION - Use property accessors
        // ===================================================================

        sharedLua->new_usertype<Collider>("Collider",
            // Use sol::property for vectors to ensure proper handling
            "shapes", sol::property(
                [](Collider& c) -> std::vector<ColliderShape>&{ return c.shapes; }
            ),
            "bounds", sol::property(
                [](Collider& c) -> std::vector<BoundingBox>&{ return c.bounds; }
            ),

            // Regular members
            "defaultLayer", &Collider::defaultLayer,
            "defaultMask", &Collider::defaultMask,
            "showBBox", &Collider::showBBox,

            // Member functions
            "GetPrimaryShape", sol::resolve<ColliderShape & ()>(&Collider::GetPrimaryShape),
            "GetPrimaryBounds", sol::resolve<BoundingBox & ()>(&Collider::GetPrimaryBounds),
            "GetEffectiveLayer", &Collider::GetEffectiveLayer,
            "GetEffectiveMask", &Collider::GetEffectiveMask
        );
    }

    void LuaScriptingSystem::RegisterEntityQueries() 
    {
        // -----------------------------------------------------------
        // ENTITY QUERY FUNCTIONS (GLOBAL)
        // -----------------------------------------------------------

        // Find all entities with a component (returns array of entity IDs)
        sharedLua->set_function("FindEntitiesWithComponent",
            [this](const std::string& componentName) -> std::vector<Entity> {
                if (!pCoordinator) {
                    Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eWarning,
                        "Lua: Coordinator not available for FindEntitiesWithComponent");
                    return {};
                }
                return pCoordinator->FindEntitiesWithComponentByName(componentName);
            });

        // Find first entity with a component (returns entity ID or -1 if not found)
        sharedLua->set_function("FindEntityWithComponent",
            [this](const std::string& componentName) -> Entity {
                if (!pCoordinator) {
                    Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eWarning,
                        "Lua: Coordinator not available for FindEntityWithComponent");
                    return static_cast<Entity>(-1);
                }
                return pCoordinator->FindEntityWithComponentByName(componentName);
            });

        // Get total active entity count
        sharedLua->set_function("GetEntityCount", [this]() -> int {
            if (!pCoordinator) return 0;
            return pCoordinator->GetEntityCount();
            });

        // Check if an entity ID is valid and active
        sharedLua->set_function("IsEntityValid", [this](Entity entity) -> bool {
            if (!pCoordinator) return false;
            if (entity == static_cast<Entity>(-1)) return false; // Invalid entity marker
            return pCoordinator->HasActiveEntity(entity);
            });
    }

    void LuaScriptingSystem::RegisterCrossEntityAccess()
    {
        // ok so in this function righht
        // we are gg to provide 2 methods of accessing the component 
        // of the targeted entity

        // first is entity wrapper (like accessing a struct / class)
        // then the direct access method with the entity id
        using Text = Uma_UI::Text;
       // Component list macro
#define COMPONENT_LIST \
        X(Transform)   \
        X(RigidBody)   \
        X(Sprite)      \
        X(Collider)    \
        X(Player)      \
        X(Enemy)       \
        X(Camera)      \
        X(PathFinding) \
        X(Projectile)  \
        X(Animator)    \
        X(Text)        \


    // -----------------------------------------------------------
    // ENTITY WRAPPER
    // -----------------------------------------------------------

        sharedLua->set_function("GetEntity", [this](Entity targetEntity) -> sol::table {
            sol::state_view lua(sharedLua->lua_state());
            sol::table entityTable = lua.create_table();

            if (!pCoordinator->HasActiveEntity(targetEntity)) {
                entityTable["isValid"] = false;
                return entityTable;
            }

            entityTable["id"] = targetEntity;
            entityTable["isValid"] = true;

            // Add all component getters using X-macro pattern
#define X(ComponentType) \
        entityTable["Get" #ComponentType] = [this, targetEntity]() -> ComponentType* { \
            if (!pCoordinator->HasActiveEntity(targetEntity)) return nullptr; \
            auto& arr = pCoordinator->GetComponentArray<ComponentType>(); \
            if (!arr.Has(targetEntity)) return nullptr; \
            return &arr.GetData(targetEntity); \
        }; \
        entityTable["Has" #ComponentType] = [this, targetEntity]() -> bool { \
            if (!pCoordinator->HasActiveEntity(targetEntity)) return false; \
            return pCoordinator->GetComponentArray<ComponentType>().Has(targetEntity); \
        };

            COMPONENT_LIST

#undef X

                return entityTable;
            });

        // -----------------------------------------------------------
        // DIRECT FUNCTIONS (FUNCTIONAL STYLE)
        // -----------------------------------------------------------

#define X(ComponentType) \
    sharedLua->set_function("Get" #ComponentType "From", [this](Entity targetEntity) -> ComponentType* { \
        if (!pCoordinator->HasActiveEntity(targetEntity)) return nullptr; \
        auto& arr = pCoordinator->GetComponentArray<ComponentType>(); \
        if (!arr.Has(targetEntity)) return nullptr; \
        return &arr.GetData(targetEntity); \
    }); \
    sharedLua->set_function("Has" #ComponentType "On", [this](Entity targetEntity) -> bool { \
        if (!pCoordinator->HasActiveEntity(targetEntity)) return false; \
        return pCoordinator->GetComponentArray<ComponentType>().Has(targetEntity); \
    });

        COMPONENT_LIST

#undef X
#undef COMPONENT_LIST
    }

    void LuaScriptingSystem::RegisterUtilityFUnctions()
    {
        // Utility functions
        sharedLua->set_function("Log", [](const std::string& msg) {
            Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo, msg);
            });

        sharedLua->set_function("LogWarning", [](const std::string& msg) {
            Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eWarning, msg);
            });

        sharedLua->set_function("LogError", [](const std::string& msg) {
            Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eError, msg);
            });

        // other helpers

        // Add time access
        sharedLua->set_function("GetDeltaTime", [this]() {
            return lastDeltaTime; // Store in class
            });

        // play audio
        sharedLua->set_function("PlaySound", [this](const std::string& audioName, float vol, int loop) {
            pEventSystem->Emit<Uma_Engine::PlaySoundEvent>(audioName, vol, loop);
            });

        sharedLua->set_function("StopSound", [this](const std::string& audioName) {
            pEventSystem->Emit<Uma_Engine::StopSoundEvent>(audioName);
            });

        sharedLua->set_function("PlayMusic", [this](const std::string& audioName, float vol, int loop) {
            pEventSystem->Emit<Uma_Engine::PlayMusicEvent>(audioName, vol, loop);
            });

        sharedLua->set_function("StopMusic", [this](const std::string& audioName) {
            pEventSystem->Emit<Uma_Engine::StopMusicEvent>(audioName);
            });

        //new audio event
        sharedLua->set_function("PlayEntitySound", [this](Uma_ECS::Entity entity, const std::string& audioName, bool loop, float vol) {
            pEventSystem->Emit<Uma_Engine::PlayEntitySoundEvent>(entity, audioName, loop, vol);
            });

        sharedLua->set_function("StopEntitySound", [this](Uma_ECS::Entity entity) {
            pEventSystem->Emit<Uma_Engine::StopEntitySoundEvent>(entity);
            });

        sharedLua->set_function("StopEntitySoundByName", [this](Uma_ECS::Entity entity, const std::string& soundName) {
            pEventSystem->Emit<Uma_Engine::StopEntitySoundByNameEvent>(entity, soundName);
            });

        sharedLua->set_function("PlayOneShotAtEntity", [this](Uma_ECS::Entity entity, const std::string& audioName, float vol) {
            pEventSystem->Emit<Uma_Engine::PlayOneShotAtEntityEvent>(entity, audioName, vol);
            });

        sharedLua->set_function("PlayOneShotAtPosition", [this](float x, float y, const std::string& audioName, float vol) {
            pEventSystem->Emit<Uma_Engine::PlayOneShotAtPositionEvent>(x, y, audioName, vol);
            });

        // scene management
        sharedLua->set_function("LoadScene", [this](const std::string& sceneName)
            {
                pEventSystem->Emit<Uma_Engine::LoadSceneRequestEvent>(sceneName, true);
            });

        sharedLua->set_function("CloseApplication", [this]()
            {
                pEventSystem->Emit<Uma_Engine::ApplicationQuitRequest>();
            });

        sharedLua->set_function("PauseGame", [this](bool isPause)
            {
                Uma_Engine::Application::GetGamePause() = isPause;
                pEventSystem->Emit<Uma_Engine::ApplicationGamePauseRequest>(isPause);
            });

        sharedLua->set_function("IsGamePause", [this]() -> bool
            {
                return Uma_Engine::Application::GetGamePause();
            });

        sharedLua->set_function("GetFps", [this]() -> float
            {
                return Uma_Engine::Application::GetFps();
            });

        // FSM 
        sharedLua->set_function("ChangeState", [this](Uma_ECS::Entity e, std::string nextState)
            {
                auto& fsm = pCoordinator->GetComponent<FSM>(e);
                fsm.next = nextState;
            });
    }


    void LuaScriptingSystem::InitializeEntityScripts(Entity entity, LuaScript& scriptComponent)
    {
        for (auto& script : scriptComponent.scripts)
        {
            if (!script.isInitialized)
            {
                InitializeEntityScript(entity, script);
            }
        }
    }

    void LuaScriptingSystem::InitializeEntityScript(Entity entity, LuaScriptInstance& script)
    {
        if (!sharedLua || !sharedLua->lua_state())
        {
            script.hasError = true;
            script.errorMessage = "Shared Lua state is invalid";
            Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eError,
                "Cannot initialize script: Shared Lua state is invalid");
            return;
        }

        script.scriptEnv = std::make_shared<sol::environment>(*sharedLua, sol::create, sharedLua->globals());

            // Bind entity-specific functions to this environment
        BindEntityAPI(entity, (*script.scriptEnv));

       // load and run the script
        try
        {
            auto result = sharedLua->script_file(script.scriptPath, *script.scriptEnv);

            // it has problem running the script
            if (!result.valid())
            {
                sol::error err = result;
                script.hasError = true;
                script.errorMessage = err.what();

                Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eError,
                    "Lua Error loading " + script.scriptPath + ": " + script.errorMessage);

                return;
            }
        }
        catch (const sol::error& e)
        {    
            script.hasError = true;
            script.errorMessage = e.what();

            Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eError,
                "Lua Error loading " + script.scriptPath + ": " + script.errorMessage);

            return;
        }

        // store entity ID in environment 
        (*script.scriptEnv)["EntityID"] = entity;

        // Discover exposed variables
        DiscoverExposedVariables(script);

        // Sync variables to Lua BEFORE calling Start()
        SyncVariablesToLua(script);

        script.isVariableDirty = false;
        script.isInitialized = true;

        Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo,
           "Lua script loaded: " + script.scriptPath);
    }

    void LuaScriptingSystem::UnsubscribeEvents()
    {
        pEventSystem->UnsubscribeSystem<LuaScriptingSystem>();
    }

    void LuaScriptingSystem::SubscribeToEvents()
    {
        pEventSystem->Subscribe<Uma_Engine::OnCollisionEnterEvent, LuaScriptingSystem>(
            [this](const Uma_Engine::OnCollisionEnterEvent& e)
            {
                OnCollisionEnterEvent(e.entityA, e.entityB);
            }
        );

        pEventSystem->Subscribe<Uma_Engine::OnCollisionEvent, LuaScriptingSystem>(
            [this](const Uma_Engine::OnCollisionEvent& e)
            {
                OnCollisionEvent(e.entityA, e.entityB);
            }
        );

        pEventSystem->Subscribe<Uma_Engine::OnCollisionExitEvent, LuaScriptingSystem>(
            [this](const Uma_Engine::OnCollisionExitEvent& e)
            {
                OnCollisionExitEvent(e.entityA, e.entityB);
            }
        );

        pEventSystem->Subscribe<Uma_Engine::OnTriggerEnterEvent, LuaScriptingSystem>(
            [this](const Uma_Engine::OnTriggerEnterEvent& e)
            {
                OnTriggerEnterEvent(e.trigger, e.entity);
            }
        );

        pEventSystem->Subscribe<Uma_Engine::OnTriggerEvent, LuaScriptingSystem>(
            [this](const Uma_Engine::OnTriggerEvent& e)
            {
                OnTriggerEvent(e.trigger, e.entity);
            }
        );

        pEventSystem->Subscribe<Uma_Engine::OnTriggerExitEvent, LuaScriptingSystem>(
            [this](const Uma_Engine::OnTriggerExitEvent& e)
            {
                OnTriggerExitEvent(e.trigger, e.entity);
            });

        pEventSystem->Subscribe<Uma_Engine::CallLuaToInitScript, LuaScriptingSystem>(
            [this](const Uma_Engine::CallLuaToInitScript& e)
            {
                auto& lArray = pCoordinator->GetComponentArray<LuaScript>();
                if (lArray.Has(e.en))
                {
                    auto& lua = lArray.GetData(e.en);
                    InitializeEntityScripts(e.en, lua);
                    //CallStart();
                }
            }
        );

        pEventSystem->Subscribe<Uma_Engine::ButtonOnClickedEvent, LuaScriptingSystem>(
            [this](const Uma_Engine::ButtonOnClickedEvent& e)
            {
                if (pCoordinator->HasComponent<LuaScript>(e.entity))
                {
                    auto& luaComp = pCoordinator->GetComponent<LuaScript>(e.entity);
                    auto& script = *luaComp.GetScript(e.scriptIndex);
                    CallLuaFunction(script, "OnClicked");
                }
            }
        );

        pEventSystem->Subscribe<Uma_Engine::EntityScriptActiveStateChangedEvent, LuaScriptingSystem>(
            [this](const Uma_Engine::EntityScriptActiveStateChangedEvent& e)
            {
                if (!pCoordinator->HasComponent<LuaScript>(e.entityId)) return;

                auto& luaScriptComp = pCoordinator->GetComponent<LuaScript>(e.entityId);
                auto& script = *luaScriptComp.GetScript(e.scriptIndex);

                if (!script.scriptEnv)
                {
                    InitializeEntityScript(e.entityId, script);
                }

                if (script.isEnabled != e.isActive)
                {
                    if (e.isActive)
                    {
                        CallLuaFunction(script, "OnEnable");
                    }
                    else
                    {
                        CallLuaFunction(script, "OnDisable");
                    }
                    script.isEnabled = e.isActive;
                }
            }
        );

        pEventSystem->Subscribe<Uma_Engine::CallLuaFunction, LuaScriptingSystem>(
            [this](const Uma_Engine::CallLuaFunction& e)
            {
                auto& luaComp = pCoordinator->GetComponent<LuaScript>(e.entity);
                auto& script = *luaComp.GetScriptByName(e.scriptName);
                CallLuaFunction(script, e.functionName.c_str());
            });
    }

    void LuaScriptingSystem::OnCollisionEvent(Entity entityA, Entity entityB)
    {
        auto& scriptArray = pCoordinator->GetComponentArray<LuaScript>();

        // Notify both entities
        NotifyScripts(scriptArray, entityA, entityB, "OnCollision");
        NotifyScripts(scriptArray, entityB, entityA, "OnCollision");
    }

    void LuaScriptingSystem::OnCollisionEnterEvent(Entity entityA, Entity entityB)
    {
        pCoordinator->GetSerializerName();
        auto& scriptArray = pCoordinator->GetComponentArray<LuaScript>();

        // Notify both entities
        NotifyScripts(scriptArray, entityA, entityB, "OnCollisionEnter");
        NotifyScripts(scriptArray, entityB, entityA, "OnCollisionEnter");
    }

    void LuaScriptingSystem::OnCollisionExitEvent(Entity entityA, Entity entityB)
    {
        auto& scriptArray = pCoordinator->GetComponentArray<LuaScript>();

        NotifyScripts(scriptArray, entityA, entityB, "OnCollisionExit");
        NotifyScripts(scriptArray, entityB, entityA, "OnCollisionExit");
    }

    void LuaScriptingSystem::OnTriggerEvent(Entity entityA, Entity entityB)
    {
        auto& scriptArray = pCoordinator->GetComponentArray<LuaScript>();

        // Notify both entities
        NotifyScripts(scriptArray, entityA, entityB, "OnTrigger");
        NotifyScripts(scriptArray, entityB, entityA, "OnTrigger");
    }

    void LuaScriptingSystem::OnTriggerEnterEvent(Entity entityA, Entity entityB)
    {
        auto& scriptArray = pCoordinator->GetComponentArray<LuaScript>();

        // Notify both entities
        NotifyScripts(scriptArray, entityA, entityB, "OnTriggerEnter");
        NotifyScripts(scriptArray, entityB, entityA, "OnTriggerEnter");
    }

    void LuaScriptingSystem::OnTriggerExitEvent(Entity entityA, Entity entityB)
    {
        auto& scriptArray = pCoordinator->GetComponentArray<LuaScript>();

        NotifyScripts(scriptArray, entityA, entityB, "OnTriggerExit");
        NotifyScripts(scriptArray, entityB, entityA, "OnTriggerExit");
    }

    // Helper function to reduce code duplication
    void LuaScriptingSystem::NotifyScripts(
        ComponentArray<LuaScript>& scriptArray,
        Entity owner,
        Entity other,
        const char* callbackName)
    {
        if (!scriptArray.Has(owner)) return;

        auto& scriptComponent = scriptArray.GetData(owner);

        for (auto& script : scriptComponent.scripts)
        {
            if (!script.isEnabled || script.hasError) continue;

            CallLuaFunction(script, callbackName, other);
        }
    }
     
    // this function is to allow the luascript to be able to get other component 
    // of this entity
    // key difference from register LUA API is that this is only for the entity 
    // that owns the script (non global)
    void LuaScriptingSystem::BindEntityAPI(Entity entity, sol::environment& env)
    {
       // USING MARCO TO DO THE JOB 
       // More effecient

        using Text = Uma_UI::Text;

#define COMPONENT_LIST \
        BIND_COMPONENT_GETTER(Transform)   \
        BIND_COMPONENT_GETTER(RigidBody)   \
        BIND_COMPONENT_GETTER(Sprite)      \
        BIND_COMPONENT_GETTER(Collider)    \
        BIND_COMPONENT_GETTER(Player)      \
        BIND_COMPONENT_GETTER(Enemy)       \
        BIND_COMPONENT_GETTER(Camera)      \
        BIND_COMPONENT_GETTER(Text)        \
        BIND_COMPONENT_GETTER(PathFinding) \
        BIND_COMPONENT_GETTER(Animator)    \
        //BIND_COMPONENT_GETTER(Projectile)\

#define BIND_COMPONENT_GETTER(ComponentType) \
    env.set_function("Get" #ComponentType, [this, entity]() -> ComponentType* { \
        if (!pCoordinator->HasActiveEntity(entity)) { \
            Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eError, \
                "Lua: Tried to access " #ComponentType " on destroyed entity"); \
            return nullptr; \
        } \
        auto& arr = pCoordinator->GetComponentArray<ComponentType>(); \
        if (!arr.Has(entity)) { \
            return nullptr; \
        } \
        return &arr.GetData(entity); \
    }); \
    env.set_function("Has" #ComponentType, [this, entity]() -> bool { \
        if (!pCoordinator->HasActiveEntity(entity)) return false; \
        return pCoordinator->GetComponentArray<ComponentType>().Has(entity); \
    });

        COMPONENT_LIST

#undef BIND_COMPONENT_GETTER
#undef COMPONENT_LIST
    }

    void LuaScriptingSystem::DiscoverExposedVariables(LuaScriptInstance& script)
    {
        sol::optional<sol::table> exposedVars = (*script.scriptEnv)["ExposedVars"];

        if (!exposedVars) return;

        for (const auto& pair : *exposedVars)
        {
            std::string varName = pair.first.as<std::string>();

            auto it = std::find_if(script.exposedVariables.begin(),
                script.exposedVariables.end(),
                [&varName](const LuaVariable& v) {
                    return v.name == varName;
                });

            if (it == script.exposedVariables.end())
            {
                LuaVariable var;
                var.name = varName;

                sol::object value = pair.second;

                if (value.is<float>())
                {
                    var.type = LuaVarType::T_FLOAT;
                    var.value = value.as<float>();
                }
                else if (value.is<int>())
                {
                    var.type = LuaVarType::T_INT;
                    var.value = value.as<int>();
                }
                else if (value.is<bool>())
                {
                    var.type = LuaVarType::T_BOOL;
                    var.value = value.as<bool>();
                }
                else if (value.is<std::string>())
                {
                    var.type = LuaVarType::T_STRING;
                    var.value = value.as<std::string>();
                }

                script.exposedVariables.push_back(var);
            }
        }
    }

    void LuaScriptingSystem::SyncVariablesToLua(LuaScriptInstance& script)
    {
        for (const auto& var : script.exposedVariables)
        {
            switch (var.type)
            {
            case LuaVarType::T_FLOAT:
                (*script.scriptEnv)[var.name] = std::get<float>(var.value);
                break;
            case LuaVarType::T_INT:
                (*script.scriptEnv)[var.name] = std::get<int>(var.value);
                break;
            case LuaVarType::T_BOOL:
                (*script.scriptEnv)[var.name] = std::get<bool>(var.value);
                break;
            case LuaVarType::T_STRING:
                (*script.scriptEnv)[var.name] = std::get<std::string>(var.value);
                break;
            }
        }
    }

    void LuaScriptingSystem::SyncVariablesFromLua(LuaScriptInstance& script)
    {
        for (auto& var : script.exposedVariables)
        {
            sol::object value = (*script.scriptEnv)[var.name];

            switch (var.type)
            {
            case LuaVarType::T_FLOAT:
                if (value.is<float>())
                    var.value = value.as<float>();
                break;
            case LuaVarType::T_INT:
                if (value.is<int>())
                    var.value = value.as<int>();
                break;
            case LuaVarType::T_BOOL:
                if (value.is<bool>())
                    var.value = value.as<bool>();
                break;
            case LuaVarType::T_STRING:
                if (value.is<std::string>())
                    var.value = value.as<std::string>();
                break;
            }
        }
    }

    // for hot reload 
    void LuaScriptingSystem::RefreshScript(Entity entity, size_t scriptIndex)
    {
        auto& scriptArray = pCoordinator->GetComponentArray<LuaScript>();
        if (!scriptArray.Has(entity)) return;
        
        auto& scriptComponent = scriptArray.GetData(entity);
        if (scriptIndex >= scriptComponent.scripts.size()) return;

        auto& script = scriptComponent.scripts[scriptIndex];

        // Store current variable values
        std::vector<LuaVariable> oldVars = script.exposedVariables;

        // Clear and reinitialize
        (*script.scriptEnv) = sol::nil;
        script.isInitialized = false;
        script.hasError = false;

        InitializeEntityScript(entity, script);

        // Restore variable values where names match
        for (auto& newVar : script.exposedVariables)
        {
            for (const auto& oldVar : oldVars)
            {
                if (newVar.name == oldVar.name && newVar.type == oldVar.type)
                {
                    newVar.value = oldVar.value;
                    break;
                }
            }
        }

        // Re-sync to Lua
        SyncVariablesToLua(script);
    }

    void LuaScriptingSystem::RegisterInputBindings()
    {
        // Bind all input functions
        BindInputFunction("KeyDown", &Uma_Engine::HybridInputSystem::KeyDown);
        BindInputFunction("KeyPressed", &Uma_Engine::HybridInputSystem::KeyPressed);
        BindInputFunction("KeyReleased", &Uma_Engine::HybridInputSystem::KeyReleased);
        BindInputFunction("MouseButtonDown", &Uma_Engine::HybridInputSystem::MouseButtonDown);
        BindInputFunction("MouseButtonPressed", &Uma_Engine::HybridInputSystem::MouseButtonPressed);
        BindInputFunction("MouseButtonReleased", &Uma_Engine::HybridInputSystem::MouseButtonReleased);

        // Mouse position (special case)
        sharedLua->set_function("GetMousePosition", [this]() -> Vec2 {
            return pInputSystem ? pInputSystem->GetSceneMousePosition() : Vec2{ 0, 0 };
            });
    }

    void LuaScriptingSystem::RegisterKeyConstants()
    {
        struct KeyBinding { const char* name; int code; };

        // Movement keys
        const KeyBinding keys[] = {
            {"KEY_W", GLFW_KEY_W},
            {"KEY_A", GLFW_KEY_A},
            {"KEY_S", GLFW_KEY_S},
            {"KEY_D", GLFW_KEY_D},
            {"KEY_U",GLFW_KEY_U},
            {"KEY_SPACE", GLFW_KEY_SPACE},
            {"KEY_SHIFT", GLFW_KEY_LEFT_SHIFT},
            {"KEY_CTRL", GLFW_KEY_LEFT_CONTROL},
            {"KEY_E", GLFW_KEY_E},
            {"MOUSE_LEFT", GLFW_MOUSE_BUTTON_LEFT},
            {"MOUSE_RIGHT", GLFW_MOUSE_BUTTON_RIGHT},
            {"MOUSE_MIDDLE", GLFW_MOUSE_BUTTON_MIDDLE}
        };

        for (const auto& key : keys) {
            sharedLua->set(key.name, key.code);
        }

        // Register all letters A-Z
        for (char c = 'A'; c <= 'Z'; ++c) {
            std::string keyName = std::string("KEY_") + c;
            sharedLua->set(keyName, GLFW_KEY_A + (c - 'A'));
        }

        // Register all numbers 0-9
        for (int i = 0; i <= 9; ++i) {
            std::string keyName = std::string("KEY_") + std::to_string(i);
            sharedLua->set(keyName, GLFW_KEY_0 + i);
        }

        // Register function keys F1-F12
        for (int i = 1; i <= 12; ++i) {
            std::string keyName = std::string("KEY_F") + std::to_string(i);
            sharedLua->set(keyName, GLFW_KEY_F1 + (i - 1));
        }

        // Other common keys
        sharedLua->set("KEY_ESCAPE", GLFW_KEY_ESCAPE);
        sharedLua->set("KEY_ENTER", GLFW_KEY_ENTER);
        sharedLua->set("KEY_TAB", GLFW_KEY_TAB);
        sharedLua->set("KEY_BACKSPACE", GLFW_KEY_BACKSPACE);
        sharedLua->set("KEY_DELETE", GLFW_KEY_DELETE);
    }

    void LuaScriptingSystem::OnEntityDestroyed(Entity entity)
    {
        /*Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo,
            "=== OnEntityDestroyed START for entity: " + std::to_string(entity));*/

        if (!pCoordinator)
            return;

        auto& scriptArray = pCoordinator->GetComponentArray<LuaScript>();
        if (!scriptArray.Has(entity))
            return;

        auto& scriptComponent = scriptArray.GetData(entity);

        if (scriptComponent.scripts.empty())
            return;

        bool luaValid = (sharedLua && sharedLua->lua_state());

        for (auto& script : scriptComponent.scripts)
        {
            // Safe OnDestroy
            if (luaValid && script.isEnabled && script.isInitialized)
            {
                try
                {
                    CallLuaFunction(script, "OnDestroy");
                }
                catch (...) { /* ignore */ }
            }

            // Safe environment cleanup
            if (script.scriptEnv)
            {
                try
                {
                    // Only clear if Lua still valid
                    if (luaValid)
                        (*script.scriptEnv) = sol::nil;

                    // Release the shared_ptr entirely
                    script.scriptEnv.reset();
                }
                catch (...) { /* ignore */ }
            }

            // Clear C++ state
            script.isInitialized = false;
            script.hasError = false;
        }

        // Let vector destructor run � shared_ptr handles Lua env memory safely
        scriptComponent.scripts.clear();

        Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo,
            "=== OnEntityDestroyed COMPLETE for entity: " + std::to_string(entity));
    }

}