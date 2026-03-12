/*!
\file   LuaScriptingSystem.cpp
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

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
#include "SoundManager.hpp"
#include "ResourcesTypes.hpp"

#include "../Components/Transform.h"
#include "../Components/RigidBody.h"
#include "../Components/Sprite.h"
#include "../Components/Collider.h"
#include "../Components/Camera.h"
#include "../Components/Player.h"
#include "../Components/Enemy.h"
#include "../Components/ParticleEmitter.h"
#include "../Components/FSM.h"
#include "../UI/Components/Text.h"
#include "../UI/Components/Dialogue.h"
#include "../UI/Components/Slider.h"
#include "../UI/Components/Checkbox.h"
#include "../UI/Components/Button.h"
#include "../Components/AudioComponent.h"
#include "../Components/SpriteMaterial.h"
#include "../Components/Cutscene.h"

#include "Events/ApplicationEvents.h"

// Feedback system
#include "../UI/Core/FeedbackTypes.h"

#include <functional>

// temp
#include <rapidjson/istreamwrapper.h>
#include "Core/FilePaths.h"
#include "Application.h"

namespace Uma_ECS
{
    
    void LuaScriptingSystem::Init(Coordinator* c, 
        Uma_Engine::EventSystem* e, 
        Uma_Engine::HybridInputSystem* i, 
        Uma_Engine::ResourcesManager* r, 
        Uma_Engine::Graphics* g,
        Uma_Engine::SoundManager* s)
    {
        // linking the Engine systems 
        pCoordinator = c;
        pEventSystem = e;
        pInputSystem = i;
        pResourcesManager = r;
        pGraphics = g;
        pSoundManager = s;

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

        // Register Vec3
        sharedLua->new_usertype<Vec3>("Vec3",
            sol::constructors<Vec3(), Vec3(float), Vec3(float, float, float)>(),

            // Fields
            "x", &Vec3::x,
            "y", &Vec3::y,
            "z", &Vec3::z,

            // Operators (if you want them)
            sol::meta_function::addition, [](const Vec3& a, const Vec3& b) { return a + b; },
            sol::meta_function::subtraction, [](const Vec3& a, const Vec3& b) { return a - b; },
            sol::meta_function::multiplication, sol::overload(
                [](const Vec3& v, float s) { return v * s; },
                [](float s, const Vec3& v) { return s * v; }
            ),
            sol::meta_function::to_string, [](const Vec3& v) {
                return "Vec3(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ")";
            }
        );

        sharedLua->set_function("Vec3", sol::overload(
            []() { return Vec3(); },
            [](float x, float y, float z) { return Vec3(x, y, z); }
        ));

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

        RegisterUtilityFunctions();
        RegisterCrossEntityAccess();
        RegisterEntityQueries();
        RegisterEntityManipulation();
        RegisterFeedbackAPI();

        RegisterPlayFabAPI();
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

        sharedLua->set_function("GetChildrenList", [&](Entity entity) -> std::vector<Entity> {
            return pCoordinator->GetChildrenList(entity);
            });

        sharedLua->set_function("GetChildren", [&](Entity entity, int index) -> Entity {
            return pCoordinator->GetChildren(entity, index);
            });

        sharedLua->set_function("HasChildren", [&](Entity entity, int index) -> Entity {

            if (pCoordinator->GetChildren(entity, index) == static_cast<Entity>(-1))
                return false;

            return true;
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

    void LuaScriptingSystem::RegisterFeedbackAPI()
    {
        sharedLua->new_enum<Uma_UI::FeedbackType>("FeedbackType",
            {
                { "Normal",    Uma_UI::FeedbackType::Normal    },
                { "Affinity",  Uma_UI::FeedbackType::Affinity  },
                { "Critical",  Uma_UI::FeedbackType::Critical  },
                { "Heal",      Uma_UI::FeedbackType::Heal      },
                { "PlayerHit", Uma_UI::FeedbackType::PlayerHit },
                { "ManaSpend", Uma_UI::FeedbackType::ManaSpend },
                { "ManaGain",  Uma_UI::FeedbackType::ManaGain  },
                { "Warning",   Uma_UI::FeedbackType::Warning   },
            });

        sharedLua->set_function("SpawnFeedback",
            [this](float worldX, float worldY, const std::string& value,
                sol::optional<std::string> typeStr)
            {
                Uma_UI::FeedbackType type = Uma_UI::FeedbackType::Normal;

                if (typeStr.has_value())
                {
                    const std::string& s = typeStr.value();
                    if (s == "affinity" || s == "Affinity" || s == "AFFINITY")
                        type = Uma_UI::FeedbackType::Affinity;

                    else if (s == "crit" || s == "Crit" || s == "CRIT"
                        || s == "critical" || s == "Critical" || s == "CRITICAL")
                        type = Uma_UI::FeedbackType::Critical;

                    else if (s == "heal" || s == "Heal" || s == "HEAL")
                        type = Uma_UI::FeedbackType::Heal;

                    else if (s == "playerhit" || s == "PlayerHit" || s == "PLAYERHIT"
                        || s == "player" || s == "Player" || s == "PLAYER")
                        type = Uma_UI::FeedbackType::PlayerHit;

                    else if (s == "manaspend" || s == "ManaSpend" || s == "MANASPEND"
                        || s == "mana" || s == "Mana" || s == "MANA")
                        type = Uma_UI::FeedbackType::ManaSpend;

                    else if (s == "managain" || s == "ManaGain" || s == "MANAGAIN")
                        type = Uma_UI::FeedbackType::ManaGain;

                    else if (s == "warn" || s == "Warn" || s == "WARN"
                        || s == "warning" || s == "Warning" || s == "WARNING")
                        type = Uma_UI::FeedbackType::Warning;
                    // else: anything else → Normal
                }

                pEventSystem->Emit<Uma_UI::SpawnFeedbackEvent>(
                    worldX, worldY, value, type);
            });
    }

    void LuaScriptingSystem::SetPlayFabManager(Uma_Engine::PlayFabManager* pfb)
    {
        pPlayFabManager = pfb;
       /* if (pPlayFabManager && sharedLua)
        {
            RegisterPlayFabAPI();
        }*/
    }

    void LuaScriptingSystem::RegisterPlayFabAPI()
    {
        if (!pPlayFabManager) return;

        // Register LeaderboardPlayerEntry type
        sharedLua->new_usertype<Uma_Engine::LeaderboardPlayerEntry>("LeaderboardEntry",
            "entityId",    &Uma_Engine::LeaderboardPlayerEntry::entityId,
            "displayName", &Uma_Engine::LeaderboardPlayerEntry::displayName,
            "rank",        &Uma_Engine::LeaderboardPlayerEntry::rank,
            "score",       &Uma_Engine::LeaderboardPlayerEntry::score
        );

        // ── State queries ────────────────────────────────────────────────────

        sharedLua->set_function("PlayFab_IsReady", [this]() -> bool {
            return pPlayFabManager && pPlayFabManager->IsReady();
        });

        sharedLua->set_function("PlayFab_IsLoggedIn", [this]() -> bool {
            return pPlayFabManager && pPlayFabManager->IsReady() && pPlayFabManager->Player().IsLoggedIn();
        });

        // ── Authentication ───────────────────────────────────────────────────

        sharedLua->set_function("PlayFab_LoginWithCustomID",
            [this](const std::string& customId, bool createAccount,
                   sol::optional<sol::protected_function> onSuccess,
                   sol::optional<sol::protected_function> onFailure)
            {
                if (!pPlayFabManager || !pPlayFabManager->IsReady()) return;

                Uma_Engine::OnPlayerLoginSuccess successCb = nullptr;
                Uma_Engine::OnPlayerLoginFailure failureCb = nullptr;

                if (onSuccess.has_value())
                {
                    sol::protected_function fn = onSuccess.value();
                    successCb = [fn]() mutable {
                        auto result = fn();
                        if (!result.valid()) {
                            sol::error err = result;
                            Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eError,
                                std::string("PlayFab_LoginWithCustomID onSuccess error: ") + err.what());
                        }
                    };
                }

                if (onFailure.has_value())
                {
                    sol::protected_function fn = onFailure.value();
                    failureCb = [fn](HRESULT hr, const std::string& msg) mutable {
                        auto result = fn(msg);
                        if (!result.valid()) {
                            sol::error err = result;
                            Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eError,
                                std::string("PlayFab_LoginWithCustomID onFailure error: ") + err.what());
                        }
                    };
                }

                pPlayFabManager->LoginWithCustomID(customId, createAccount, successCb, failureCb);
            });

        sharedLua->set_function("PlayFab_Logout", [this]() {
            if (pPlayFabManager) pPlayFabManager->Logout();
        });

        // ── Account ──────────────────────────────────────────────────────────

        sharedLua->set_function("PlayFab_GetAccountInfo",
            [this](sol::optional<sol::protected_function> onSuccess,
                   sol::optional<sol::protected_function> onFailure)
            {
                if (!pPlayFabManager || !pPlayFabManager->Player().IsLoggedIn()) return;

                Uma_Engine::OnKVResult successCb = nullptr;
                Uma_Engine::OnPlayerFailure failureCb = nullptr;

                if (onSuccess.has_value())
                {
                    sol::protected_function fn = onSuccess.value();
                    sol::state_view lua(sharedLua->lua_state());
                    successCb = [fn, lua](const std::unordered_map<std::string, std::string>& data) mutable {
                        sol::table t = lua.create_table();
                        for (auto& [k, v] : data) t[k] = v;
                        fn(t);
                    };
                }

                if (onFailure.has_value())
                {
                    sol::protected_function fn = onFailure.value();
                    failureCb = [fn](HRESULT, const std::string& msg) mutable { fn(msg); };
                }

                pPlayFabManager->Player().GetAccountInfo(successCb, failureCb);
            });

        sharedLua->set_function("PlayFab_SetDisplayName",
            [this](const std::string& name,
                   sol::optional<sol::protected_function> onSuccess,
                   sol::optional<sol::protected_function> onFailure)
            {
                if (!pPlayFabManager || !pPlayFabManager->Player().IsLoggedIn()) return;

                Uma_Engine::OnPlayerSuccess successCb = nullptr;
                Uma_Engine::OnPlayerFailure failureCb = nullptr;

                if (onSuccess.has_value())
                {
                    sol::protected_function fn = onSuccess.value();
                    successCb = [fn]() mutable { fn(); };
                }
                if (onFailure.has_value())
                {
                    sol::protected_function fn = onFailure.value();
                    failureCb = [fn](HRESULT, const std::string& msg) mutable { fn(msg); };
                }

                pPlayFabManager->Player().SetDisplayName(name, successCb, failureCb);
            });

        // ── Player Data ──────────────────────────────────────────────────────

        sharedLua->set_function("PlayFab_ReadData",
            [this](const std::string& key,
                   sol::optional<sol::protected_function> onSuccess,
                   sol::optional<sol::protected_function> onFailure)
            {
                if (!pPlayFabManager || !pPlayFabManager->Player().IsLoggedIn()) return;

                Uma_Engine::OnStringResult successCb = nullptr;
                Uma_Engine::OnPlayerFailure failureCb = nullptr;

                if (onSuccess.has_value())
                {
                    sol::protected_function fn = onSuccess.value();
                    successCb = [fn](const std::string& value) mutable { fn(value); };
                }
                if (onFailure.has_value())
                {
                    sol::protected_function fn = onFailure.value();
                    failureCb = [fn](HRESULT, const std::string& msg) mutable { fn(msg); };
                }

                pPlayFabManager->Player().ReadData(key, successCb, failureCb);
            });

        sharedLua->set_function("PlayFab_ReadMultipleData",
            [this](sol::table keysTable,
                   sol::optional<sol::protected_function> onSuccess,
                   sol::optional<sol::protected_function> onFailure)
            {
                if (!pPlayFabManager || !pPlayFabManager->Player().IsLoggedIn()) return;

                std::vector<std::string> keys;
                for (auto& pair : keysTable)
                    keys.push_back(pair.second.as<std::string>());

                Uma_Engine::OnKVResult successCb = nullptr;
                Uma_Engine::OnPlayerFailure failureCb = nullptr;

                if (onSuccess.has_value())
                {
                    sol::protected_function fn = onSuccess.value();
                    sol::state_view lua(sharedLua->lua_state());
                    successCb = [fn, lua](const std::unordered_map<std::string, std::string>& data) mutable {
                        sol::table t = lua.create_table();
                        for (auto& [k, v] : data) t[k] = v;
                        fn(t);
                    };
                }
                if (onFailure.has_value())
                {
                    sol::protected_function fn = onFailure.value();
                    failureCb = [fn](HRESULT, const std::string& msg) mutable { fn(msg); };
                }

                pPlayFabManager->Player().ReadMultipleData(keys, successCb, failureCb);
            });

        sharedLua->set_function("PlayFab_WriteData",
            [this](const std::string& key, const std::string& value,
                   sol::optional<sol::protected_function> onSuccess,
                   sol::optional<sol::protected_function> onFailure)
            {
                if (!pPlayFabManager || !pPlayFabManager->Player().IsLoggedIn()) return;

                Uma_Engine::OnPlayerSuccess successCb = nullptr;
                Uma_Engine::OnPlayerFailure failureCb = nullptr;

                if (onSuccess.has_value())
                {
                    sol::protected_function fn = onSuccess.value();
                    successCb = [fn]() mutable { fn(); };
                }
                if (onFailure.has_value())
                {
                    sol::protected_function fn = onFailure.value();
                    failureCb = [fn](HRESULT, const std::string& msg) mutable { fn(msg); };
                }

                pPlayFabManager->Player().WriteData(key, value, successCb, failureCb);
            });

        // ── Title Data ───────────────────────────────────────────────────────

        sharedLua->set_function("PlayFab_GetTitleData",
            [this](const std::string& key,
                   sol::optional<sol::protected_function> onSuccess,
                   sol::optional<sol::protected_function> onFailure)
            {
                if (!pPlayFabManager || !pPlayFabManager->Player().IsLoggedIn()) return;

                Uma_Engine::OnStringResult successCb = nullptr;
                Uma_Engine::OnPlayerFailure failureCb = nullptr;

                if (onSuccess.has_value())
                {
                    sol::protected_function fn = onSuccess.value();
                    successCb = [fn](const std::string& value) mutable { fn(value); };
                }
                if (onFailure.has_value())
                {
                    sol::protected_function fn = onFailure.value();
                    failureCb = [fn](HRESULT, const std::string& msg) mutable { fn(msg); };
                }

                pPlayFabManager->Player().GetTitleData(key, successCb, failureCb);
            });

        // ── Statistics ───────────────────────────────────────────────────────

        sharedLua->set_function("PlayFab_SubmitScore",
            [this](const std::string& statName, double value,
                   sol::optional<sol::protected_function> onSuccess,
                   sol::optional<sol::protected_function> onFailure)
            {
                if (!pPlayFabManager || !pPlayFabManager->Player().IsLoggedIn()) return;

                Uma_Engine::OnPlayerSuccess successCb = nullptr;
                Uma_Engine::OnPlayerFailure failureCb = nullptr;

                if (onSuccess.has_value())
                {
                    sol::protected_function fn = onSuccess.value();
                    successCb = [fn]() mutable { fn(); };
                }
                if (onFailure.has_value())
                {
                    sol::protected_function fn = onFailure.value();
                    failureCb = [fn](HRESULT, const std::string& msg) mutable { fn(msg); };
                }

                pPlayFabManager->Player().SubmitScore(statName, value, successCb, failureCb);
            });

        sharedLua->set_function("PlayFab_GetMyStats",
            [this](const std::string& statName,
                   sol::optional<sol::protected_function> onSuccess,
                   sol::optional<sol::protected_function> onFailure)
            {
                if (!pPlayFabManager || !pPlayFabManager->Player().IsLoggedIn()) return;

                Uma_Engine::OnStatResult successCb = nullptr;
                Uma_Engine::OnPlayerFailure failureCb = nullptr;

                if (onSuccess.has_value())
                {
                    sol::protected_function fn = onSuccess.value();
                    successCb = [fn](const std::string& name, double val) mutable { fn(name, val); };
                }
                if (onFailure.has_value())
                {
                    sol::protected_function fn = onFailure.value();
                    failureCb = [fn](HRESULT, const std::string& msg) mutable { fn(msg); };
                }

                pPlayFabManager->Player().GetMyStats(statName, successCb, failureCb);
            });

        // ── Leaderboards ─────────────────────────────────────────────────────

        sharedLua->set_function("PlayFab_GetLeaderboard",
            [this](const std::string& name, int pageSize, int startPos,
                   sol::optional<sol::protected_function> onSuccess,
                   sol::optional<sol::protected_function> onFailure)
            {
                if (!pPlayFabManager || !pPlayFabManager->Player().IsLoggedIn()) return;

                Uma_Engine::OnLeaderboardPlayerResult successCb = nullptr;
                Uma_Engine::OnPlayerFailure failureCb = nullptr;

                if (onSuccess.has_value())
                {
                    sol::protected_function fn = onSuccess.value();
                    sol::state_view lua(sharedLua->lua_state());
                    successCb = [fn, lua](const std::vector<Uma_Engine::LeaderboardPlayerEntry>& entries) mutable {
                        sol::table t = lua.create_table(static_cast<int>(entries.size()), 0);
                        for (size_t i = 0; i < entries.size(); ++i)
                        {
                            sol::table entry = lua.create_table(0, 4);
                            entry["entityId"]    = entries[i].entityId;
                            entry["displayName"] = entries[i].displayName;
                            entry["rank"]        = entries[i].rank;
                            entry["score"]       = entries[i].score;
                            t[static_cast<int>(i) + 1] = entry;
                        }
                        fn(t);
                    };
                }
                if (onFailure.has_value())
                {
                    sol::protected_function fn = onFailure.value();
                    failureCb = [fn](HRESULT, const std::string& msg) mutable { fn(msg); };
                }

                pPlayFabManager->Player().GetLeaderboard(
                    name, static_cast<uint32_t>(pageSize), static_cast<uint32_t>(startPos),
                    successCb, failureCb);
            });

        sharedLua->set_function("PlayFab_GetLeaderboardAroundMe",
            [this](const std::string& name, int pageSize,
                   sol::optional<sol::protected_function> onSuccess,
                   sol::optional<sol::protected_function> onFailure)
            {
                if (!pPlayFabManager || !pPlayFabManager->Player().IsLoggedIn()) return;

                Uma_Engine::OnLeaderboardPlayerResult successCb = nullptr;
                Uma_Engine::OnPlayerFailure failureCb = nullptr;

                if (onSuccess.has_value())
                {
                    sol::protected_function fn = onSuccess.value();
                    sol::state_view lua(sharedLua->lua_state());
                    successCb = [fn, lua](const std::vector<Uma_Engine::LeaderboardPlayerEntry>& entries) mutable {
                        sol::table t = lua.create_table(static_cast<int>(entries.size()), 0);
                        for (size_t i = 0; i < entries.size(); ++i)
                        {
                            sol::table entry = lua.create_table(0, 4);
                            entry["entityId"]    = entries[i].entityId;
                            entry["displayName"] = entries[i].displayName;
                            entry["rank"]        = entries[i].rank;
                            entry["score"]       = entries[i].score;
                            t[static_cast<int>(i) + 1] = entry;
                        }
                        fn(t);
                    };
                }
                if (onFailure.has_value())
                {
                    sol::protected_function fn = onFailure.value();
                    failureCb = [fn](HRESULT, const std::string& msg) mutable { fn(msg); };
                }

                pPlayFabManager->Player().GetLeaderboardAroundMe(
                    name, static_cast<uint32_t>(pageSize), successCb, failureCb);
            });

        Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo,
            "PlayFab API registered in Lua");
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
        // Register Sprite
        sharedLua->new_usertype<Sprite>("Sprite",
            // Texture and rendering
            "texturePath", &Sprite::texturePath,
            "renderLayer", &Sprite::renderLayer,
            "renderOrder", &Sprite::renderOrder,

            // Flip flags
            "flipX", &Sprite::flipX,
            "flipY", &Sprite::flipY,
            "autoFlip", &Sprite::autoFlip,

            // Size behavior
            "UseNativeSize", &Sprite::UseNativeSize,

            // Color and transparency
            "tintColor", &Sprite::tintColor,
            "alpha", &Sprite::alpha,

            // Sprite sheet properties
            "spriteSheetGrid", &Sprite::spriteSheetGrid,
            "spriteCell", &Sprite::spriteCell,
            "spriteOffset", &Sprite::spriteOffset,

            // Methods
            "GetUVs", [](const Sprite& sprite) -> std::tuple<Vec2, Vec2> {
                Vec2 uvOffset, uvSize;
                sprite.GetUVs(uvOffset, uvSize);
                return std::make_tuple(uvOffset, uvSize);
            }
        );

        // Register SpriteMaterial
        sharedLua->new_usertype<SpriteMaterial>("SpriteMaterial",
            "effectName", &SpriteMaterial::effectName,
            "SetFloat", &SpriteMaterial::SetFloat,
            "SetVec2", &SpriteMaterial::SetVec2,
            "SetVec3", &SpriteMaterial::SetVec3,
            "SetVec4", &SpriteMaterial::SetVec4,
            "SetInt", &SpriteMaterial::SetInt
        );

        sharedLua->new_enum<ElementType>("ElementType",
            {
                { "None", ElementType::None },          // 1,2
                { "Fire", ElementType::Fire },          // 3
                { "Water", ElementType::Water },        // 4
                { "Wind", ElementType::Wind },          // 5
                { "Steam", ElementType::Steam },        // 6
                { "Pyronado", ElementType::Pyronado },  // 7
                { "Whirlpool", ElementType::Whirlpool } // 8
            }
        );

        sharedLua->new_usertype<AttackStats>("AttackStats",
            sol::constructors<AttackStats()>(),
            "attackName", &AttackStats::AttackName,
            "animationClipName", &AttackStats::animationClipName,
            "mDamageMultiplier", &AttackStats::mDamageMultiplier,
            "mAttackSpeedMultiplier", &AttackStats::mAttackSpeedMultiplier,
            "triggerColliderIndex", &AttackStats::triggerColliderIndex,
            "manaCost", &AttackStats::manaCost,
            "attackRange", &AttackStats::attackRange,
            "attackArc", &AttackStats::attackArc,
            "applyBurn", &AttackStats::applyBurn,
            "applyStun", &AttackStats::applyStun,
            "effectDuration", &AttackStats::effectDuration,
            "attackCd", &AttackStats::attackCd,
            "attackCdCurr", &AttackStats::attackCdCurr,
            "attackIsInCoolDown", &AttackStats::attackIsInCoolDown,
            "elementType", &AttackStats::elementType
        );

        sharedLua->set_function("CreateAttackStats", []() -> AttackStats* {
            return new AttackStats();
            });

        // Helper function to add AttackStats to a Player's attackStats vector
        sharedLua->set_function("AddAttackStats", [](Player& player, AttackStats* attack) {
            if (attack) {
                player.attackStats.push_back(*attack);
                delete attack;  // Clean up the temporary object after copying
            }
            });

        // Helper function to clear all attack stats
        sharedLua->set_function("ClearAttackStats", [](Player& player) {
            player.attackStats.clear();
            });

        // Helper function to get attack stats count
        sharedLua->set_function("GetAttackStatsCount", [](Player& player) -> int {
            return static_cast<int>(player.attackStats.size());
            });

        sharedLua->new_usertype<CheckpointData>("CheckpointData",
            "checkpointID", &CheckpointData::checkpointID,
            "checkpointX", &CheckpointData::checkpointX,
            "checkpointY", &CheckpointData::checkpointY,
            "hasCheckpoint", &CheckpointData::hasCheckpoint
        );

        sharedLua->new_usertype<Player>("Player",
            "mHealth", &Player::mHealth,
            "mMaxHealth", &Player::mMaxHealth,
            "mHealthRegenRate", &Player::mHealthRegenRate,
            "mHealthRegenDelay", &Player::mHealthRegenDelay,
            "mHealthRegenDelayTimer", &Player::mHealthRegenDelayTimer,
            "mCanRegenHealth", &Player::mCanRegenHealth,

            "mSpeed", &Player::mSpeed,
            "mDashSpeed", &Player::mDashSpeed,
            "mDashDuration", &Player::mDashDuration,
            "mDashCD", &Player::mDashCD,
            "mDashCDMax", & Player::mDashCDMax,

            "mAttackDamage", &Player::mAttackDamage,
            "mAttackSpeed", &Player::mAttackSpeed,
            "mAttackRange", &Player::mAttackRange,
            "mDefense", &Player::mDefense,
            "mCritDamage", &Player::mCritDamage,

            "mMana", &Player::mMana,
            "mMaxMana", &Player::mMaxMana,
            "mManaRegenRate", &Player::mManaRegenRate,
            "mNeutralAttackManaGain", &Player::mNeutralAttackManaGain,

            "isStunned", &Player::isStunned,
            "stunedTimer", &Player::stunedTimer,
            "isInvulnerable", &Player::isInvulnerable,

            "mInvulnerabilityDuration", &Player::mInvulnerabilityDuration,
            "mHitStunDuration", &Player::mHitStunDuration,

            "lastElementUsed", &Player::lastElementUsed,
            "elementComboTimer", &Player::elementComboTimer,
            "elementComboWindow", &Player::elementComboWindow,

            "hasShield", &Player::hasShield,
            "isShieldBroken", &Player::isShieldBroken,


            "currAttackIndex", &Player::currAttackIndex,
            // "animatorState", &Player::animatorState,  // Optional - enum would need registration

            "attackStats", sol::property(
                [](Player& c) -> std::vector<AttackStats>&{ return c.attackStats; }
            ),

            "checkpointData", &Player::checkpointData
        );


        // Register Enemy component
        sharedLua->new_usertype<Enemy>("Enemy",
            "mHealth", &Enemy::mHealth,
            "mMaxHealth", &Enemy::mMaxHealth,
            "mHealthRegenRate", &Enemy::mHealthRegenRate,
            "mSpeed", &Enemy::mSpeed,
            "mAttackDamage", &Enemy::mAttackDamage,
            "mAttackSpeed", &Enemy::mAttackSpeed,
            "mAttackRange", &Enemy::mAttackRange,
            "mDefense", &Enemy::mDefense
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

            "GetCurrentFrame", [](const Uma_Engine::SpriteAnimator& animator) -> int {
                return animator.GetCurrentFrame();
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

        using Text = Uma_UI::Text;
        using Image = Uma_UI::Image;

        //Register Text component
        sharedLua->new_usertype<Text>("Text",
            "text", &Text::text,
            "color", &Text::color,
            "visible", &Text::visible
        );

        // register ui color
        sharedLua->new_usertype<Uma_UI::Color>("Color",
            "r", &Uma_UI::Color::r,
            "g", &Uma_UI::Color::g,
            "b", &Uma_UI::Color::b,
            "a", &Uma_UI::Color::a
        );

        // Register Image
        sharedLua->new_usertype<Image>("Image",
            "textureName", &Image::texturePath,
            "sortingOrder", &Image::sortingOrder,
            "fillAmount", &Image::fillAmount,
            "color", sol::property(
                [](Image& img) -> Uma_UI::Color& {
                    return img.color;
                },
                [](Image& img, const Uma_UI::Color& c) {
                    img.color = c;
                }
            ),
            "change", &Image::change
        );

        sharedLua->new_enum<Uma_UI::SliderDirection>("SliderDirection",
            {
                { "LeftToRight", Uma_UI::SliderDirection::LeftToRight },
                { "RightToLeft", Uma_UI::SliderDirection::RightToLeft },
                { "BottomToTop", Uma_UI::SliderDirection::BottomToTop },
                { "TopToBottom", Uma_UI::SliderDirection::TopToBottom }
            });

        sharedLua->new_usertype<Uma_UI::Slider>("Slider",
            "minValue", &Uma_UI::Slider::minValue,
            "maxValue", &Uma_UI::Slider::maxValue,
            "value", &Uma_UI::Slider::value,
            "wholeNumbers", &Uma_UI::Slider::wholeNumbers,
            "direction", &Uma_UI::Slider::direction,
            "interactable", &Uma_UI::Slider::interactable,
            "background", &Uma_UI::Slider::background,
            "fill", &Uma_UI::Slider::fill,
            "handle", &Uma_UI::Slider::handle,
            "normalColour", &Uma_UI::Slider::normalColour,
            "highlightColour", &Uma_UI::Slider::highlightColour,
            "disabledColour", &Uma_UI::Slider::disabledColour,
            "scriptName", &Uma_UI::Slider::scriptName,
            "isDragging", sol::readonly(&Uma_UI::Slider::isDragging),
            "isHovered", sol::readonly(&Uma_UI::Slider::isHovered)
        );

        sharedLua->new_enum<Uma_UI::CheckboxState>("CheckboxState",
            {
                { "Normal",   Uma_UI::CheckboxState::Normal   },
                { "Hovered",  Uma_UI::CheckboxState::Hovered  },
                { "Pressed",  Uma_UI::CheckboxState::Pressed  },
                { "Disabled", Uma_UI::CheckboxState::Disabled }
            });

        sharedLua->new_usertype<Uma_UI::Checkbox>("Checkbox",
            "isChecked", &Uma_UI::Checkbox::isChecked,
            "interactable", &Uma_UI::Checkbox::interactable,
            "currentState", sol::readonly(&Uma_UI::Checkbox::currentState),
            "background", &Uma_UI::Checkbox::background,
            "checkmark", &Uma_UI::Checkbox::checkmark,
            "normalColour", &Uma_UI::Checkbox::normalColour,
            "hoverColour", &Uma_UI::Checkbox::hoverColour,
            "pressedColour", &Uma_UI::Checkbox::pressedColour,
            "disabledColour", &Uma_UI::Checkbox::disabledColour,
            "checkedColour", &Uma_UI::Checkbox::checkedColour,
            "checkmarkNormalColour", &Uma_UI::Checkbox::checkmarkNormalColour,
            "checkmarkDisabledColour", &Uma_UI::Checkbox::checkmarkDisabledColour,
            "scriptName", &Uma_UI::Checkbox::scriptName,
            "wasHoveredLastFrame", sol::readonly(&Uma_UI::Checkbox::wasHoveredLastFrame),
            "wasPressedWhileHovered", sol::readonly(&Uma_UI::Checkbox::wasPressedWhileHovered)
        );

        // Register EffectProperty enum
        sharedLua->new_enum<Uma_UI::EffectProperty>("EffectProperty", {
            {"Position", Uma_UI::EffectProperty::Position},
            {"Scale", Uma_UI::EffectProperty::Scale},
            {"ColorTint", Uma_UI::EffectProperty::ColorTint},
            {"Alpha", Uma_UI::EffectProperty::Alpha}
            });

        // Register EasingType enum
        sharedLua->new_enum<Uma_UI::EasingType>("EasingType", {
            {"Linear", Uma_UI::EasingType::Linear},
            {"EaseInQuad", Uma_UI::EasingType::EaseInQuad},
            {"EaseOutQuad", Uma_UI::EasingType::EaseOutQuad},
            {"EaseInOutQuad", Uma_UI::EasingType::EaseInOutQuad},
            {"EaseInCubic", Uma_UI::EasingType::EaseInCubic},
            {"EaseOutCubic", Uma_UI::EasingType::EaseOutCubic},
            {"EaseInOutCubic", Uma_UI::EasingType::EaseInOutCubic},
            {"EaseInQuart", Uma_UI::EasingType::EaseInQuart},
            {"EaseOutQuart", Uma_UI::EasingType::EaseOutQuart},
            {"EaseInOutQuart", Uma_UI::EasingType::EaseInOutQuart},
            {"EaseInElastic", Uma_UI::EasingType::EaseInElastic},
            {"EaseOutElastic", Uma_UI::EasingType::EaseOutElastic},
            {"EaseInOutElastic", Uma_UI::EasingType::EaseInOutElastic},
            {"EaseInBounce", Uma_UI::EasingType::EaseInBounce},
            {"EaseOutBounce", Uma_UI::EasingType::EaseOutBounce},
            {"EaseInOutBounce", Uma_UI::EasingType::EaseInOutBounce}
            });

        // Register EffectClip
        sharedLua->new_usertype<Uma_UI::EffectClip>("EffectClip",
            "name", &Uma_UI::EffectClip::name,
            "property", &Uma_UI::EffectClip::property,
            "easing", &Uma_UI::EffectClip::easing,
            "duration", &Uma_UI::EffectClip::duration,
            "delay", &Uma_UI::EffectClip::delay,
            "loop", &Uma_UI::EffectClip::loop,
            "applyToChildren", &Uma_UI::EffectClip::applyToChildren,
            "startVec2", &Uma_UI::EffectClip::startVec2,
            "endVec2", &Uma_UI::EffectClip::endVec2,
            "startColor", &Uma_UI::EffectClip::startColor,
            "endColor", &Uma_UI::EffectClip::endColor,
            "startFloat", &Uma_UI::EffectClip::startFloat,
            "endFloat", &Uma_UI::EffectClip::endFloat,
            "currentTime", &Uma_UI::EffectClip::currentTime,
            "isPlaying", &Uma_UI::EffectClip::isPlaying,
            "hasStarted", &Uma_UI::EffectClip::hasStarted,
            "Play", &Uma_UI::EffectClip::Play,
            "Pause", &Uma_UI::EffectClip::Pause,
            "Stop", &Uma_UI::EffectClip::Stop,
            "Reset", &Uma_UI::EffectClip::Reset,
            "GetProgress", &Uma_UI::EffectClip::GetProgress,
            "IsComplete", &Uma_UI::EffectClip::IsComplete
        );

        // Register Effects - sol2 auto-handles std::vector<EffectClip>
        sharedLua->new_usertype<Uma_UI::Effects>("Effects",
            "clips", &Uma_UI::Effects::clips,  // Direct member access
            "playOnEnable", &Uma_UI::Effects::playOnEnable,
            "Play", sol::overload(
                [](Uma_UI::Effects& effects, const std::string& name) { effects.PlayClipByName(name); },
                [](Uma_UI::Effects& effects, int index) { if (index >= 0) effects.PlayClip(static_cast<size_t>(index)); }
            ),
            "PlayAll", &Uma_UI::Effects::PlayAll,
            "StopAll", &Uma_UI::Effects::StopAll,
            "ResetAll", &Uma_UI::Effects::ResetAll,
            "PauseClip", [](Uma_UI::Effects& effects, int index) { if (index >= 0) effects.PauseClip(static_cast<size_t>(index)); },
            "StopClip", [](Uma_UI::Effects& effects, int index) { if (index >= 0) effects.StopClip(static_cast<size_t>(index)); },
            "ResetClip", [](Uma_UI::Effects& effects, int index) { if (index >= 0) effects.ResetClip(static_cast<size_t>(index)); },
            "PauseClipByName", &Uma_UI::Effects::PauseClipByName,
            "StopClipByName", &Uma_UI::Effects::StopClipByName,
            "ResetClipByName", &Uma_UI::Effects::ResetClipByName,
            "IsClipPlaying", sol::overload(
                [](Uma_UI::Effects& effects, const std::string& name) -> bool {
                    int index = effects.FindClipIndexByName(name);
                    return index >= 0 && effects.IsClipPlaying(static_cast<size_t>(index));
                },
                [](Uma_UI::Effects& effects, int index) -> bool {
                    return index >= 0 && effects.IsClipPlaying(static_cast<size_t>(index));
                }
            ),
            "IsClipComplete", sol::overload(
                [](Uma_UI::Effects& effects, const std::string& name) -> bool {
                    int index = effects.FindClipIndexByName(name);
                    return index >= 0 && effects.IsClipComplete(static_cast<size_t>(index));
                },
                [](Uma_UI::Effects& effects, int index) -> bool {
                    return index >= 0 && effects.IsClipComplete(static_cast<size_t>(index));
                }
            ),
            "GetCurrentClip", [](Uma_UI::Effects& effects) -> std::string {
                for (size_t i = 0; i < effects.clips.size(); ++i) {
                    if (effects.IsClipPlaying(i)) return effects.clips[i].name;
                }
                return "";
            },
            "GetClipCount", &Uma_UI::Effects::GetClipCount,
            "FindClipIndexByName", &Uma_UI::Effects::FindClipIndexByName,
            "AddClip", &Uma_UI::Effects::AddClip,
            "RemoveClip", [](Uma_UI::Effects& effects, const std::string& name) -> bool {
                int index = effects.FindClipIndexByName(name);
                if (index >= 0) {
                    effects.clips.erase(effects.clips.begin() + index);
                    return true;
                }
                return false;
            }
        );

        sharedLua->new_usertype<Uma_UI::Button>("Button",
            "interactable", &Uma_UI::Button::interactable,
            "currentState", &Uma_UI::Button::currentState,
            "normalColour", &Uma_UI::Button::normalColour,
            "hoverColour", &Uma_UI::Button::hoverColour,
            "pressedColour", &Uma_UI::Button::pressedColour,
            "disabledColour", &Uma_UI::Button::disabledColour
        );

        // Register DialogueLine so Lua can read fields off returned line tables
        sharedLua->new_usertype<Uma_UI::DialogueLine>("DialogueLine",
            "speaker", &Uma_UI::DialogueLine::speaker,
            "text", &Uma_UI::DialogueLine::text,
            "portrait", &Uma_UI::DialogueLine::portrait
        );

        // Register Dialogue so GetDialogueFrom(entity) returns a usable object
        sharedLua->new_usertype<Uma_UI::Dialogue>("Dialogue",
            // GetSequence(id) → table of DialogueLine, or nil
            "GetSequence", [this](Uma_UI::Dialogue& data, const std::string& seqId) -> sol::object
            {
                const Uma_UI::DialogueSequence* seq = data.FindSequence(seqId);
                if (!seq)
                    return sol::nil;

                sol::state_view L(sharedLua->lua_state());
                sol::table lines = L.create_table(static_cast<int>(seq->lines.size()), 0);
                for (size_t i = 0; i < seq->lines.size(); ++i)
                {
                    sol::table entry = L.create_table(0, 3);
                    entry["speaker"] = seq->lines[i].speaker;
                    entry["text"] = seq->lines[i].text;
                    entry["portrait"] = seq->lines[i].portrait;
                    lines[static_cast<int>(i) + 1] = entry;
                }
                return lines;
            },

            // GetLineCount(id) → int (useful for Lua-side validation)
            "GetLineCount", [](Uma_UI::Dialogue& data, const std::string& seqId) -> int
            {
                const Uma_UI::DialogueSequence* seq = data.FindSequence(seqId);
                return seq ? static_cast<int>(seq->lines.size()) : 0;
            },

            // HasSequence(id) → bool
            "HasSequence", [](Uma_UI::Dialogue& data, const std::string& seqId) -> bool
            {
                return data.FindSequence(seqId) != nullptr;
            }
        );

        // Convenience free function: GetDialogueSequence(entity, id) → line table or nil
        // Matches the call style used in DialogueController.lua.
        sharedLua->set_function("GetDialogueSequence",
            [this](Entity entity, const std::string& seqId) -> sol::object
            {
                if (!pCoordinator->HasActiveEntity(entity))
                    return sol::nil;

                auto& arr = pCoordinator->GetComponentArray<Uma_UI::Dialogue>();
                if (!arr.Has(entity))
                    return sol::nil;

                const Uma_UI::DialogueSequence* seq = arr.GetData(entity).FindSequence(seqId);
                if (!seq)
                    return sol::nil;

                sol::state_view L(sharedLua->lua_state());
                sol::table lines = L.create_table(static_cast<int>(seq->lines.size()), 0);
                for (size_t i = 0; i < seq->lines.size(); ++i)
                {
                    sol::table entry = L.create_table(0, 3);
                    entry["speaker"] = seq->lines[i].speaker;
                    entry["text"] = seq->lines[i].text;
                    entry["portrait"] = seq->lines[i].portrait;
                    lines[static_cast<int>(i) + 1] = entry;
                }
                return lines;
            });

        // Register Camera
        sharedLua->new_usertype<Camera>("Camera",
            "zoom", &Camera::mZoom,
            "followPlayer", &Camera::followPlayer
        );

        // Register Cutscene
        sharedLua->new_usertype<Cutscene>("Cutscene",
            "playOnce", &Cutscene::playOnce,
            "hasPlayed", &Cutscene::hasPlayed
        );

        // GetCutsceneActions(entity) -> table of actions or nil
        sharedLua->set_function("GetCutsceneActions",
            [this](Entity entity) -> sol::object
            {
                if (!pCoordinator->HasActiveEntity(entity))
                    return sol::nil;

                auto& arr = pCoordinator->GetComponentArray<Cutscene>();
                if (!arr.Has(entity))
                    return sol::nil;

                auto& cutscene = arr.GetData(entity);
                sol::state_view L(sharedLua->lua_state());
                sol::table actions = L.create_table(static_cast<int>(cutscene.actions.size()), 0);
                for (size_t i = 0; i < cutscene.actions.size(); ++i)
                {
                    sol::table entry = L.create_table(0, 5);
                    entry["type"] = static_cast<int>(cutscene.actions[i].type);
                    entry["targetX"] = cutscene.actions[i].targetPosition.x;
                    entry["targetY"] = cutscene.actions[i].targetPosition.y;
                    entry["duration"] = cutscene.actions[i].duration;
                    entry["dialogueSequenceId"] = cutscene.actions[i].dialogueSequenceId;
                    entry["shakeIntensity"] = cutscene.actions[i].shakeIntensity;
                    entry["targetZoom"] = cutscene.actions[i].targetZoom;
                    actions[static_cast<int>(i) + 1] = entry;
                }
                return actions;
            });

        // SetCutscenePlayed(entity, played) - mark cutscene as played
        sharedLua->set_function("SetCutscenePlayed",
            [this](Entity entity, bool played)
            {
                if (!pCoordinator->HasActiveEntity(entity)) return;
                auto& arr = pCoordinator->GetComponentArray<Cutscene>();
                if (!arr.Has(entity)) return;
                arr.GetData(entity).hasPlayed = played;
            });

        // SetCameraFollow(cameraEntity, follow) - enable/disable player follow
        sharedLua->set_function("SetCameraFollow",
            [this](Entity cameraEntity, bool follow)
            {
                if (!pCoordinator->HasActiveEntity(cameraEntity)) return;
                auto& arr = pCoordinator->GetComponentArray<Camera>();
                if (!arr.Has(cameraEntity)) return;
                arr.GetData(cameraEntity).followPlayer = follow;
            });

        // SetCameraPosition(cameraEntity, x, y) - snap camera to position
        sharedLua->set_function("SetCameraPosition",
            [this](Entity cameraEntity, float x, float y)
            {
                if (!pCoordinator->HasActiveEntity(cameraEntity)) return;
                auto& tfArr = pCoordinator->GetComponentArray<Transform>();
                if (!tfArr.Has(cameraEntity)) return;
                tfArr.GetData(cameraEntity).position = Vec2(x, y);
            });

        // GetTransformPosition(entity) -> x, y (multi-return)
        sharedLua->set_function("GetTransformPosition",
            [this](Entity entity) -> std::tuple<float, float>
            {
                if (!pCoordinator->HasActiveEntity(entity))
                    return { 0.0f, 0.0f };
                auto& tfArr = pCoordinator->GetComponentArray<Transform>();
                if (!tfArr.Has(entity))
                    return { 0.0f, 0.0f };
                auto& pos = tfArr.GetData(entity).worldPosition;
                return { pos.x, pos.y };
            });

        // CameraShake(cameraEntity, intensity, duration) - trigger screen shake
        sharedLua->set_function("CameraShake",
            [this](Entity cameraEntity, float intensity, float duration)
            {
                if (!pCoordinator->HasActiveEntity(cameraEntity)) return;
                auto& arr = pCoordinator->GetComponentArray<Camera>();
                if (!arr.Has(cameraEntity)) return;
                auto& cam = arr.GetData(cameraEntity);
                cam.mShakeIntensity = intensity;
                cam.mShakeTimer = duration;
            });

        // SetCameraZoom(cameraEntity, zoom) - set camera zoom level
        sharedLua->set_function("SetCameraZoom",
            [this](Entity cameraEntity, float zoom)
            {
                if (!pCoordinator->HasActiveEntity(cameraEntity)) return;
                auto& arr = pCoordinator->GetComponentArray<Camera>();
                if (!arr.Has(cameraEntity)) return;
                arr.GetData(cameraEntity).mZoom = zoom;
            });

        // GetCameraZoom(cameraEntity) -> zoom value
        sharedLua->set_function("GetCameraZoom",
            [this](Entity cameraEntity) -> float
            {
                if (!pCoordinator->HasActiveEntity(cameraEntity)) return 10.0f;
                auto& arr = pCoordinator->GetComponentArray<Camera>();
                if (!arr.Has(cameraEntity)) return 10.0f;
                return arr.GetData(cameraEntity).mZoom;
            });

        // FindCameraEntity() -> entity or -1
        sharedLua->set_function("FindCameraEntity",
            [this]() -> Entity
            {
                auto& camArr = pCoordinator->GetComponentArray<Camera>();
                auto entities = camArr.GetAllEntities();
                if (entities.empty()) return static_cast<Entity>(-1);
                return entities[0];
            });

        // Cutscene active flag - freezes gameplay systems (player, enemies, pathfinding, projectiles)
        sharedLua->set_function("SetCutsceneActive", [](bool active) {
            Uma_Engine::Application::GetCutsceneActive() = active;
        });
        sharedLua->set_function("IsCutsceneActive", []() -> bool {
            return Uma_Engine::Application::GetCutsceneActive();
        });

        // CutsceneActionType constants for Lua
        sharedLua->set("CUTSCENE_SET_CAMERA", 0);
        sharedLua->set("CUTSCENE_LERP_CAMERA", 1);
        sharedLua->set("CUTSCENE_PLAY_DIALOGUE", 2);
        sharedLua->set("CUTSCENE_WAIT", 3);
        sharedLua->set("CUTSCENE_RETURN_CAMERA", 4);
        sharedLua->set("CUTSCENE_SHAKE_CAMERA", 5);

        sharedLua->new_usertype<PathFinding>("PathFinding",
            "goal", &PathFinding::goal,
            "reachedGoal", &PathFinding::reachedGoal,
            "enabled", &PathFinding::enabled
        );

        sharedLua->new_usertype<AudioComponent>("AudioComponent",
            "defaultVolume", &Uma_ECS::AudioComponent::defaultVolume, //to set the default volumes
            "hasLoadedSound", &Uma_ECS::AudioComponent::HasLoadedSound,

            "play", [this](Uma_ECS::AudioComponent& self, Uma_ECS::Entity entity, const std::string& name) {
                // We use the AudioSystem directly here for performance 
                // because these happen every frame/frequently
                (void)self;
                pCoordinator->GetSystem<AudioSystem>()->PlayEntitySound(entity, name);
            },
            "playOneShot", [this](Uma_ECS::AudioComponent& self, Uma_ECS::Entity entity, const std::string& name) {
                (void)self;
                pCoordinator->GetSystem<AudioSystem>()->PlayOneShotAtEntity(entity, name);
            },
            "stop", sol::overload(
                [this](Uma_ECS::AudioComponent&, Uma_ECS::Entity entity) {
                    pCoordinator->GetSystem<AudioSystem>()->StopEntitySound(entity);
                },
                [this](Uma_ECS::AudioComponent&, Uma_ECS::Entity entity, const std::string& name) {
                    // This maps to your StopEntitySound(Entity, string) overload
                    pCoordinator->GetSystem<AudioSystem>()->StopEntitySound(entity, name);
                }
            ),
            "playAtPos", [this](Uma_ECS::AudioComponent& self, Uma_ECS::Entity entity, float x, float y, const std::string& name) {
                // Uses the entity's component defaults but plays at specific coordinates
                (void)self;
                pCoordinator->GetSystem<AudioSystem>()->PlayOneShotAtPosition(entity, x, y, name, self.defaultVolume, self.default3D);
            },
            "playFaded", [this](Uma_ECS::AudioComponent& self, Uma_ECS::Entity entity, const std::string& name, float fadeTime) {
                (void)self;
                pCoordinator->GetSystem<AudioSystem>()->PlayEntitySoundFaded(entity, name, fadeTime);
            },
            "fadeOut", sol::overload(
                [this](Uma_ECS::AudioComponent&, Uma_ECS::Entity entity, const std::string& name, float fadeTime) {
                    pCoordinator->GetSystem<AudioSystem>()->FadeOutSound(entity, name, fadeTime);
                },
                [this](Uma_ECS::AudioComponent&, Uma_ECS::Entity entity, float fadeTime) {
                    pCoordinator->GetSystem<AudioSystem>()->FadeOutEntity(entity, fadeTime);
                },
                [this](Uma_ECS::AudioComponent&, Uma_ECS::Entity entity, const std::string& name) {
                    pCoordinator->GetSystem<AudioSystem>()->FadeOutSound(entity, name, 1.0f);
                },
                [this](Uma_ECS::AudioComponent&, Uma_ECS::Entity entity) {
                    pCoordinator->GetSystem<AudioSystem>()->FadeOutEntity(entity, 1.0f);
                }
            ),
            "toggleLowpass", [this](Uma_ECS::AudioComponent&, Uma_ECS::Entity entity, const std::string& name, bool dulled) {
                pCoordinator->GetSystem<AudioSystem>()->toggleLowpass(entity, name, dulled);
            }
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

        // particle emmitter
        // EmitterMode enum
        sharedLua->new_enum<EmitterMode>("EmitterMode",
            {
                {"Burst", EmitterMode::Burst},
                {"Continuous", EmitterMode::Continuous},
                {"ScreenFill", EmitterMode::ScreenFill}
            }
        );

        // Particle struct (read-only, mostly for debugging)
        sharedLua->new_usertype<Particle>("Particle",
            "position", &Particle::position,
            "velocity", &Particle::velocity,
            "color", &Particle::color,
            "scale", &Particle::scale,
            "rotation", &Particle::rotation,
            "rotationSpeed", &Particle::rotationSpeed,
            "lifetime", &Particle::lifetime,
            "maxLifetime", &Particle::maxLifetime,
            "age", &Particle::age,
            "opacity", &Particle::opacity,
            "baseOpacity", &Particle::baseOpacity,
            "active", &Particle::active
        );

        // ParticleAppearance struct
        sharedLua->new_usertype<ParticleAppearance>("ParticleAppearance",
            "scaleRange", &ParticleAppearance::scaleRange,
            "startColor", &ParticleAppearance::startColor,
            "endColor", &ParticleAppearance::endColor,
            "colorLerp", &ParticleAppearance::colorLerp,
            "randomOpacity", &ParticleAppearance::randomOpacity,
            "opacityRange", &ParticleAppearance::opacityRange,
            "rotateParticles", &ParticleAppearance::rotateParticles,
            "rotationSpeedRange", &ParticleAppearance::rotationSpeedRange
        );

        // FadeSettings struct
        sharedLua->new_usertype<FadeSettings>("FadeSettings",
            "fadeIn", &FadeSettings::fadeIn,
            "fadeInDuration", &FadeSettings::fadeInDuration,
            "fadeOut", &FadeSettings::fadeOut,
            "fadeOutDuration", &FadeSettings::fadeOutDuration,
            "fadeAtEdges", &FadeSettings::fadeAtEdges,
            "edgeFadeDistance", &FadeSettings::edgeFadeDistance
        );

        // ParticlePhysics struct
        sharedLua->new_usertype<ParticlePhysics>("ParticlePhysics",
            "speedRange", &ParticlePhysics::speedRange,
            "lifetimeRange", &ParticlePhysics::lifetimeRange,
            "gravity", &ParticlePhysics::gravity,
            "drag", &ParticlePhysics::drag
        );

        // SpawnSettings struct
        sharedLua->new_usertype<SpawnSettings>("SpawnSettings",
            "spawnOffset", &SpawnSettings::spawnOffset,
            "spawnRadius", &SpawnSettings::spawnRadius,
            "useEmissionCone", &SpawnSettings::useEmissionCone,
            "emissionAngle", &SpawnSettings::emissionAngle,
            "emissionSpread", &SpawnSettings::emissionSpread
        );

        // EmissionSettings struct
        sharedLua->new_usertype<EmissionSettings>("EmissionSettings",
            "emissionRate", &EmissionSettings::emissionRate,
            "loop", &EmissionSettings::loop,
            "loopDelay", &EmissionSettings::loopDelay
        );

        // ScreenFillSettings struct
        sharedLua->new_usertype<ScreenFillSettings>("ScreenFillSettings",
            "velocityXRange", &ScreenFillSettings::velocityXRange,
            "velocityYRange", &ScreenFillSettings::velocityYRange,
            "spawnAtTop", &ScreenFillSettings::spawnAtTop,
            "spawnMargin", &ScreenFillSettings::spawnMargin
        );

        // EmitterInstance struct
        sharedLua->new_usertype<EmitterInstance>("EmitterInstance",
            // Core settings
            "name", &EmitterInstance::name,
            "mode", &EmitterInstance::mode,
            "maxParticles", &EmitterInstance::maxParticles,
            "texturePath", &EmitterInstance::texturePath,
            "isActive", &EmitterInstance::isActive,
            "renderLayer", &EmitterInstance::renderLayer,
            "renderOrder", &EmitterInstance::renderOrder,

            // Configuration structs
            "appearance", &EmitterInstance::appearance,
            "fade", &EmitterInstance::fade,
            "physics", &EmitterInstance::physics,
            "spawn", &EmitterInstance::spawn,
            "emission", &EmitterInstance::emission,
            "screenFill", &EmitterInstance::screenFill,

            // Runtime state (read-only via property)
            "particles", sol::property(
                [](EmitterInstance& e) -> std::vector<Particle>&{ return e.particles; }
            ),
            "initialized", sol::readonly(&EmitterInstance::initialized),
            "emissionTimer", sol::readonly(&EmitterInstance::emissionTimer),
            "burstTimer", sol::readonly(&EmitterInstance::burstTimer),

            // Control methods
            "Play", &EmitterInstance::Play,
            "Stop", &EmitterInstance::Stop,
            "StopAndClear", &EmitterInstance::StopAndClear,
            "Pause", &EmitterInstance::Pause,
            "Resume", &EmitterInstance::Resume,
            "IsPlaying", &EmitterInstance::IsPlaying,
            "HasActiveParticles", &EmitterInstance::HasActiveParticles,
            "GetActiveParticleCount", &EmitterInstance::GetActiveParticleCount
        );

        // ParticleEmitter component
        sharedLua->new_usertype<ParticleEmitter>("ParticleEmitter",
            // Access to emitters vector
            "emitters", sol::property(
                [](ParticleEmitter& pe) -> std::vector<EmitterInstance>&{ return pe.emitters; }
            ),

            // Management methods
            "AddEmitter", sol::overload(
                [](ParticleEmitter& pe, const std::string& name) -> int {
                    return pe.AddEmitter(name);
                },
                [](ParticleEmitter& pe) -> int {
                    return pe.AddEmitter();
                }
            ),
            "RemoveEmitter", &ParticleEmitter::RemoveEmitter,

            // Getters - return pointers, Sol2 handles nullptr
            "GetEmitter", sol::overload(
                sol::resolve<EmitterInstance * (int)>(&ParticleEmitter::GetEmitter),
                sol::resolve<EmitterInstance * (const std::string&)>(&ParticleEmitter::GetEmitter)
            ),

            "GetEmitterCount", &ParticleEmitter::GetEmitterCount,
            "PlayAll", &ParticleEmitter::PlayAll,
            "StopAll", &ParticleEmitter::StopAll
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

        sharedLua->set_function("CountEntitiesWithComponent",
            [this](const std::string& componentName) -> int {
                if (!pCoordinator) {
                    return 0;
                }
                return (int)pCoordinator
                    ->FindEntitiesWithComponentByName(componentName).size();
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
        using Image = Uma_UI::Image;
        using Effects = Uma_UI::Effects;
        using Cutscene = Uma_ECS::Cutscene;
        using Button = Uma_UI::Button;
        using Dialogue = Uma_UI::Dialogue;
        using Slider = Uma_UI::Slider;
        using Checkbox = Uma_UI::Checkbox;

       // Component list macro
#define COMPONENT_LIST      \
        X(Transform)        \
        X(RigidBody)        \
        X(Sprite)           \
        X(Collider)         \
        X(Player)           \
        X(Enemy)            \
        X(Camera)           \
        X(PathFinding)      \
        X(Projectile)       \
        X(Animator)         \
        X(Text)             \
        X(Image)            \
        X(Effects)          \
        X(Button)         \
        X(Dialogue)         \
        X(ParticleEmitter)  \
        X(AudioComponent)   \
        X(Slider)           \
        X(Checkbox)         \
        X(SpriteMaterial)   \
        X(Cutscene)         \
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

    void LuaScriptingSystem::RegisterUtilityFunctions()
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

        sharedLua->set_function("PlayOneShotAtPosition", [this](Uma_Engine::Entity ent, float x, float y, const std::string& audioName, float vol) {
            pEventSystem->Emit<Uma_Engine::PlayOneShotAtPositionEvent>(ent, x, y, audioName, vol);
            });

        // scene management
        sharedLua->set_function("LoadScene", [this](const std::string& sceneName)
            {
                pEventSystem->Emit<Uma_Engine::LoadSceneRequestEvent>(sceneName, true);
            });

        sharedLua->set_function("ReloadScene", [this]()
            {
                pEventSystem->Emit<Uma_Engine::ReLoadSceneRequestEvent>();
            });

        sharedLua->set_function("RestartScene", [this]()
            {
                // temp solution
                pEventSystem->Emit<Uma_Engine::ResetSceneRequest>(true);
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
        //global audio functions
        sharedLua->set_function("setMasterVolume", [this](float volume) {
            pSoundManager->setChannelGroupVolume(volume, Uma_Engine::SoundType::MASTER);
            });

        sharedLua->set_function("setSFXVolume", [this](float volume) {
            pSoundManager->setChannelGroupVolume(volume, Uma_Engine::SoundType::SFX);
            });

        sharedLua->set_function("setBGMVolume", [this](float volume) {
            pSoundManager->setChannelGroupVolume(volume, Uma_Engine::SoundType::BGM);
            });

        sharedLua->set_function("setGroupVolume", [this](float volume, Uma_Engine::SoundType type) {
            pSoundManager->setChannelGroupVolume(volume, type);
            });

        //============ get volume==========================================================
        sharedLua->set_function("getSFXVolume", [this]() {
            return pSoundManager->getChannelGroupVolume(Uma_Engine::SoundType::SFX);
            });

        sharedLua->set_function("getBGMVolume", [this]() {
            return pSoundManager->getChannelGroupVolume(Uma_Engine::SoundType::BGM);
            });

        sharedLua->set_function("getGroupVolume", [this](Uma_Engine::SoundType type) {
            return pSoundManager->getChannelGroupVolume(type);
            });

        sharedLua->set_function("getMasterVolume", [this]() {
            return pSoundManager->getChannelGroupVolume(Uma_Engine::SoundType::MASTER);
            });

        sharedLua->set_function("toggleGroupLowpass", [this](const std::string& groupName, bool enable)
            {
                SoundType type;
                if (groupName == "SFX") type = SoundType::SFX;
                else if (groupName == "BGM") type = SoundType::BGM;
                else if (groupName == "MASTER") type = SoundType::MASTER;
                else {
                    Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eError, "Audio Group doesnt exist");
                    return;
                }

                pCoordinator->GetSystem<AudioSystem>()->toggleLowpass(type, enable);
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
                OnTriggerEnterEvent(e.entityA, e.entityB, e.triggerOwner);
            }
        );

        pEventSystem->Subscribe<Uma_Engine::OnTriggerEvent, LuaScriptingSystem>(
            [this](const Uma_Engine::OnTriggerEvent& e)
            {
                OnTriggerEvent(e.entityA, e.entityB, e.triggerOwner);
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

    void LuaScriptingSystem::OnTriggerEvent(Entity entityA, Entity entityB, Entity triggerOwner)
    {
        auto& scriptArray = pCoordinator->GetComponentArray<LuaScript>();

        // Notify both entities, passing other entity and trigger owner
        NotifyTriggerScripts(scriptArray, entityA, entityB, triggerOwner, "OnTrigger");
        NotifyTriggerScripts(scriptArray, entityB, entityA, triggerOwner, "OnTrigger");
    }

    void LuaScriptingSystem::OnTriggerEnterEvent(Entity entityA, Entity entityB, Entity triggerOwner)
    {
        auto& scriptArray = pCoordinator->GetComponentArray<LuaScript>();

        // Notify both entities, passing other entity and trigger owner
        NotifyTriggerScripts(scriptArray, entityA, entityB, triggerOwner, "OnTriggerEnter");
        NotifyTriggerScripts(scriptArray, entityB, entityA, triggerOwner, "OnTriggerEnter");
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

    void LuaScriptingSystem::NotifyTriggerScripts(
        ComponentArray<LuaScript>& scriptArray,
        Entity owner,
        Entity other,
        Entity triggerOwner,
        const char* callbackName)
    {
        if (!scriptArray.Has(owner)) return;

        auto& scriptComponent = scriptArray.GetData(owner);

        for (auto& script : scriptComponent.scripts)
        {
            if (!script.isEnabled || script.hasError) continue;

            CallLuaFunction(script, callbackName, other, triggerOwner);
        }
    }

    // this function is to allow the luascript to be able to get other component 
    // of this entity
    // key difference from register LUA API is that this is only for the entity 
    // that owns the script (non global)
    void LuaScriptingSystem::BindEntityAPI(Entity entity, sol::environment& env)
    {
        // USING MACRO TO DO THE JOB 
        // More efficient

        using Text = Uma_UI::Text;
        using Image = Uma_UI::Image;
        using Effects = Uma_UI::Effects;
        using Button = Uma_UI::Button;
        using Slider = Uma_UI::Slider;
        using Checkbox = Uma_UI::Checkbox;
        using Dialogue = Uma_UI::Dialogue;
        using Cutscene = Uma_ECS::Cutscene;

#define COMPONENT_LIST \
        BIND_COMPONENT_GETTER(Transform)        \
        BIND_COMPONENT_GETTER(RigidBody)        \
        BIND_COMPONENT_GETTER(Sprite)           \
        BIND_COMPONENT_GETTER(Collider)         \
        BIND_COMPONENT_GETTER(Player)           \
        BIND_COMPONENT_GETTER(Enemy)            \
        BIND_COMPONENT_GETTER(Camera)           \
        BIND_COMPONENT_GETTER(Text)             \
        BIND_COMPONENT_GETTER(PathFinding)      \
        BIND_COMPONENT_GETTER(Animator)         \
        BIND_COMPONENT_GETTER(Image)            \
        BIND_COMPONENT_GETTER(ParticleEmitter)  \
        BIND_COMPONENT_GETTER(Effects)          \
        BIND_COMPONENT_GETTER(Button)           \
        BIND_COMPONENT_GETTER(Dialogue)         \
        BIND_COMPONENT_GETTER(AudioComponent)   \
        BIND_COMPONENT_GETTER(SpriteMaterial)   \
        BIND_COMPONENT_GETTER(Cutscene)         \
        BIND_COMPONENT_GETTER(Slider)           \
        BIND_COMPONENT_GETTER(Checkbox)         \
        //BIND_COMPONENT_GETTER(Projectile)     \

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

        sharedLua->set_function("GetMouseWorldPosition", [this]() -> Vec2 {

            return pInputSystem ? pGraphics->ScreenToWorld(pInputSystem->GetSceneMousePosition()) : Vec2{ 0, 0 };
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