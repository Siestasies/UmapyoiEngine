#include "LuaScriptingSystem.hpp"

#include "../Components/Transform.h"
#include "../Components/RigidBody.h"
#include "../Components/Sprite.h"
#include "../Components/Collider.h"
#include "../Components/Camera.h"
#include "../Components/Player.h"
#include "../Components/Enemy.h"

#include "Events/CollisionEvent.h"

// the macro to get components
#define BIND_COMPONENT_GETTER(ComponentType) \
    env.set_function("Get" #ComponentType, [this, entity]() -> sol::optional<std::reference_wrapper<ComponentType>> { \
        if (!pCoordinator->HasActiveEntity(entity)) { \
            Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eError, \
                "Lua: Tried to access " #ComponentType " on destroyed entity"); \
            return sol::nullopt; \
        } \
        auto& arr = pCoordinator->GetComponentArray<ComponentType>(); \
        if (!arr.Has(entity)) { \
            return sol::nullopt; \
        } \
        return std::ref(arr.GetData(entity)); \
    }); \
    env.set_function("Has" #ComponentType, [this, entity]() -> bool { \
        return pCoordinator->GetComponentArray<ComponentType>().Has(entity); \
    });

namespace Uma_ECS
{
    void LuaScriptingSystem::Init(Coordinator* c, Uma_Engine::EventSystem* e)
    {
        pCoordinator = c;
        pEventSystem = e;

        // create shared Lua state with all standard libraries
        sharedLua = std::make_shared<sol::state>();
        sharedLua->open_libraries(
            sol::lib::base,
            sol::lib::math,
            sol::lib::string,
            sol::lib::table
        );

        RegisterLuaAPI();

        RegisterEventListeners();
    }

    void LuaScriptingSystem::Update(float dt)
    {
        auto& scriptArray = pCoordinator->GetComponentArray<LuaScript>();

        for (auto const& entity : aEntities)
        {
            auto& scriptComponent = scriptArray.GetData(entity);

            // Initialize scripts if needed
            if (!scriptComponent.lua || !scriptComponent.lua->lua_state())
            {
                InitializeScripts(entity, scriptComponent);
            }

            // update each script instance
            for (auto& script : scriptComponent.scripts)
            {
                if (script.hasError)
                    continue;

                if (!script.isEnabled)
                {
                    if (script.wasEnabledLastFrame)
                    {
                        CallLuaFunction(script, "OnDisable");
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
                        InitializeScript(entity, script, scriptComponent.lua);
                    }

                    CallLuaFunction(script, "OnEnable");
                }

                if (!script.isInitialized)
                {
                    InitializeScript(entity, script, scriptComponent.lua);
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

    void LuaScriptingSystem::CallStart() 
    {
        auto& scriptArray = pCoordinator->GetComponentArray<LuaScript>();

        for (auto const& entity : aEntities)
        {
            auto& scriptComponent = scriptArray.GetData(entity);

            InitializeScripts(entity, scriptComponent);

            for (auto& script : scriptComponent.scripts)
            {
                if (!script.isEnabled || !script.isInitialized || script.hasError)
                {
                    continue;
                }

                CallLuaFunction(script, "Start");
            }
        }
    }

    void LuaScriptingSystem::Shutdown()
    {
        auto& scriptArray = pCoordinator->GetComponentArray<LuaScript>();

        for (auto const& entity : aEntities)
        {
            auto& scriptComponent = scriptArray.GetData(entity);

            for (auto& script : scriptComponent.scripts)
            {
                if (script.isEnabled && script.isInitialized)
                {
                    CallLuaFunction(script, "OnDestroy");
                }

                // CRITICAL: Clear cached callbacks first
                script.callbacks = LuaScriptInstance::CallbackCache{};

                // Clear the environment (this releases references)
                script.scriptEnv = sol::nil;
                script.isInitialized = false;
                script.hasError = false;
            }

            // Then clear the Lua state
            if (scriptComponent.lua)
            {
                scriptComponent.lua.reset();
            }
        }

        // Finally clear the shared state
        if (sharedLua)
        {
            sharedLua->collect_garbage(); // force garbage collect before shutdown
            sharedLua.reset();
        }
    }

    void LuaScriptingSystem::RegisterLuaAPI()
    {
        // this function is basically registering all the type
        // eg. transform, rigidbody, Vec2 and any helper type
        // the lua script needs

        // u need to register any class struct yall need 

        // register Vec2
        sharedLua->new_usertype<Vec2>("Vec2",
            sol::constructors<Vec2(), Vec2(float, float)>(),                                        // constructor
            "x", &Vec2::x,
            "y", &Vec2::y,
            sol::meta_function::addition, [](const Vec2& a, const Vec2& b) {return a + b; },        // operator + overload
            sol::meta_function::subtraction, [](const Vec2& a, const Vec2& b) {return a - b; },     // operator - overload
            sol::meta_function::multiplication, sol::overload(                                      // operator * overload (MULTIPLE OVERLOAD)
                [](const Vec2& v, float s) {return v * s; },
                [](float s, const Vec2& v) {return v * s; }
            )
        );

        // Register Transform component
        sharedLua->new_usertype<Transform>("Transform",
            "position", &Transform::position,
            "rotation", &Transform::rotation,
            "scale", &Transform::scale
        );

        // Register RigidBody component
        sharedLua->new_usertype<RigidBody>("RigidBody",
            "velocity", &RigidBody::velocity,
            "acceleration", &RigidBody::acceleration,
            "accel_strength", &RigidBody::accel_strength,
            "fric_coeff", &RigidBody::fric_coeff
        );

        // Register Sprite component
        sharedLua->new_usertype<Sprite>("Sprite",
            "textureName", &Sprite::textureName,
            "renderLayer", &Sprite::renderLayer,
            "flipX", &Sprite::flipX,
            "flipY", &Sprite::flipY
        );

        // need to add more (tf rb for testing now)
        // more...
    }

    void LuaScriptingSystem::InitializeScripts(Entity entity, LuaScript& scriptComponent)
    {
        scriptComponent.lua = sharedLua;

        for (auto& script : scriptComponent.scripts)
        {
            if (!script.isInitialized)
            {
                InitializeScript(entity, script, scriptComponent.lua);
            }
        }
    }

    void LuaScriptingSystem::InitializeScript(Entity entity, LuaScriptInstance& script, std::shared_ptr<sol::state> lua)
    {
        script.scriptEnv = sol::environment(*lua, sol::create, lua->globals());

            // Bind entity-specific functions to this environment
       BindEntityAPI(entity, script.scriptEnv);

       // load and run the script
       try
       {
           auto result = lua->script_file(script.scriptPath, script.scriptEnv);

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
       script.scriptEnv["EntityID"] = entity;

       // Discover exposed variables
       DiscoverExposedVariables(script);

       // cache the function to the callback
       CacheCallbacks(script);

       script.isVariableDirty = true;
       script.isInitialized = true;

       Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo,
           "Lua script loaded: " + script.scriptPath);
    }

    void LuaScriptingSystem::CacheCallbacks(LuaScriptInstance& script)
    {
        sol::optional<sol::protected_function> onCollision = script.scriptEnv["OnCollision"];
        if (onCollision)
        {
            script.callbacks.hasOnCollision = true;
            script.callbacks.onCollisionFunc = *onCollision;
        }
    }

    template <typename... Args>
    void LuaScriptingSystem::CallCachedFunction(LuaScriptInstance& script, 
                                                sol::protected_function& func, 
                                                Args&&... args)
    {
        try
        {
            auto result = func(std::forward<Args>(args)...);

            if (!result.valid())
            {
                sol::error err = result;
                script.hasError = true;
                script.errorMessage = err.what();

                Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eError,
                    script.scriptPath + " callback error: " + script.errorMessage);
            }
        }
        catch (const sol::error& e)
        {
            script.hasError = true;
            script.errorMessage = e.what();

            Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eError,
                script.scriptPath + " callback exception: " + script.errorMessage);
        }
    }

    void LuaScriptingSystem::RegisterEventListeners()
    {
        pEventSystem->Subscribe<Uma_Engine::OnCollisionEnterEvent>(
            [this](const Uma_Engine::OnCollisionEnterEvent& e)
            {
                OnCollisionEnterEvent(e.entityA, e.entityB);
            });

        pEventSystem->Subscribe<Uma_Engine::OnCollisionEvent>(
            [this](const Uma_Engine::OnCollisionEvent& e)
            {
                OnCollisionEvent(e.entityA, e.entityB);
            });

        pEventSystem->Subscribe<Uma_Engine::OnCollisionExitEvent>(
            [this](const Uma_Engine::OnCollisionExitEvent& e)
            {
                OnCollisionExitEvent(e.entityA, e.entityB);
            });

        pEventSystem->Subscribe<Uma_Engine::OnTriggerEnterEvent>(
            [this](const Uma_Engine::OnTriggerEnterEvent& e)
            {
                OnTriggerEnterEvent(e.trigger, e.entity);
            });

        pEventSystem->Subscribe<Uma_Engine::OnTriggerEvent>(
            [this](const Uma_Engine::OnTriggerEvent& e)
            {
                OnTriggerEvent(e.trigger, e.entity);
            });

        pEventSystem->Subscribe<Uma_Engine::OnTriggerExitEvent>(
            [this](const Uma_Engine::OnTriggerExitEvent& e)
            {
                OnTriggerExitEvent(e.trigger, e.entity);
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

            sol::optional<sol::protected_function> callback =
                script.scriptEnv[callbackName];

            if (callback)
            {
                CallCachedFunction(script, *callback, other);
            }
        }
    }
     
    // basically setting up functions that lua script can use 
    // eg. get transform/rigidbody or other of entity
    void LuaScriptingSystem::BindEntityAPI(Entity entity, sol::environment& env)
    {
        // Get components THIS IS UGLY
        // CAN BE OPTIMISED
        /*env.set_function("GetTransform", [this, entity]() -> Transform& 
            {
                return pCoordinator->GetComponent<Transform>(entity);
            });

        env.set_function("GetRigidBody", [this, entity]() -> RigidBody& 
            {
                return pCoordinator->GetComponent<RigidBody>(entity);
            });

        env.set_function("GetSprite", [this, entity]() -> Sprite& 
            {
                return pCoordinator->GetComponent<Sprite>(entity);
            });*/

        BIND_COMPONENT_GETTER(Transform);
        BIND_COMPONENT_GETTER(RigidBody);
        BIND_COMPONENT_GETTER(Sprite);
        //BIND_COMPONENT_GETTER(Collider);
        //BIND_COMPONENT_GETTER(Player);
        //BIND_COMPONENT_GETTER(Enemy);
        //BIND_COMPONENT_GETTER(Camera);


        // debugging
        // Utility functions
        env.set_function("Log", [](const std::string& msg) {
            Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo, msg);
            });

        env.set_function("LogWarning", [](const std::string& msg) {
            Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eWarning, msg);
            });

        env.set_function("LogError", [](const std::string& msg) {
            Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eError, msg);
            });

        // other helpers

        // Add time access
        sharedLua->set_function("GetDeltaTime", [this]() {
            return lastDeltaTime; // Store in class
            });

    }

    void LuaScriptingSystem::DiscoverExposedVariables(LuaScriptInstance& script)
    {
        sol::optional<sol::table> exposedVars = script.scriptEnv["ExposedVars"];

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
                script.scriptEnv[var.name] = std::get<float>(var.value);
                break;
            case LuaVarType::T_INT:
                script.scriptEnv[var.name] = std::get<int>(var.value);
                break;
            case LuaVarType::T_BOOL:
                script.scriptEnv[var.name] = std::get<bool>(var.value);
                break;
            case LuaVarType::T_STRING:
                script.scriptEnv[var.name] = std::get<std::string>(var.value);
                break;
            }
        }
    }

    void LuaScriptingSystem::SyncVariablesFromLua(LuaScriptInstance& script)
    {
        for (auto& var : script.exposedVariables)
        {
            sol::object value = script.scriptEnv[var.name];

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
    void LuaScriptingSystem::ReloadScript(Entity entity, size_t scriptIndex)
    {
        auto& scriptArray = pCoordinator->GetComponentArray<LuaScript>();
        if (!scriptArray.Has(entity)) return;

        auto& scriptComponent = scriptArray.GetData(entity);
        if (scriptIndex >= scriptComponent.scripts.size()) return;

        auto& script = scriptComponent.scripts[scriptIndex];

        // Store current variable values
        std::vector<LuaVariable> oldVars = script.exposedVariables;

        // Clear and reinitialize
        script.scriptEnv = sol::nil;
        script.isInitialized = false;
        script.hasError = false;

        InitializeScript(entity, script, scriptComponent.lua);

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
        CallLuaFunction(script, "Start");
    }


    // Invoking the Lua code
    // BISMILLAH PLEASE WORK
    void LuaScriptingSystem::CallLuaFunction(LuaScriptInstance& script, const char* funcName, float dt)
    {
        try
        {
            sol::optional<sol::protected_function> func = script.scriptEnv[funcName];

            if (!func)
                return;

            sol::protected_function_result result;

            if (std::string(funcName) == "Update")
            {
                result = (*func)(dt);
            }
            else
            {
                result = (*func)();
            }

            if (!result.valid())
            {
                sol::error err = result;
                script.hasError = true;
                script.errorMessage = err.what();

                Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eError,
                    "Lua Error in " + std::string(funcName) + "(): " + script.errorMessage);
            }
        }
        catch (const sol::error& e)
        {
            script.hasError = true;
            script.errorMessage = e.what();

            Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eError,
                "Lua Exception in " + std::string(funcName) + "(): " + script.errorMessage);
        }
    }
}