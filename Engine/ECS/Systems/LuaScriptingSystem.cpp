#include "LuaScriptingSystem.hpp"

#include "../Components/Transform.h"
#include "../Components/RigidBody.h"
#include "../Components/Sprite.h"
#include "../Components/Collider.h"
#include "../Components/Camera.h"
#include "../Components/Player.h"
#include "../Components/Enemy.h"

#include "Events/CollisionEvent.h"

#include <functional>

// --------------------------------------------------
// |          Component Getter Macro                |
// --------------------------------------------------

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

// SCRAPED METHOD NOT QUITE PRACTICAL
//// --------------------------------------------------
//// |            Input System Macro                  |
//// --------------------------------------------------
//
//// InputSystem Functions
//#define BIND_INPUT_FUNCTION(FuncName) \
//    sharedLua->set_function(#FuncName, [this](int param) -> bool \
//    { \
//        if (!pInputSystem) \
//        { \
//            Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eWarning, \
//                "Lua: Input System not available for " #FuncName); \
//            return false; \
//        } \
//        return pInputSystem->FuncName(param); \
//    } \
//    );

namespace Uma_ECS
{
    
    void LuaScriptingSystem::Init(Coordinator* c, Uma_Engine::EventSystem* e, Uma_Engine::HybridInputSystem* i)
    {
        // linking the Engine systems 
        pCoordinator = c;
        pEventSystem = e;
        pInputSystem = i;

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
           /* if (!scriptComponent.lua || !scriptComponent.lua->lua_state())
            {
                InitializeScripts(entity, scriptComponent);
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
                        InitializeScript(entity, script);
                    }

                    CallLuaFunction(script, "OnEnable");
                }

                if (!script.isInitialized)
                {
                    InitializeScript(entity, script);
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
        Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo,
            "LuaScriptingSystem::Shutdown() starting");

        if (!pCoordinator)
        {
            sharedLua = nullptr;
            return;
        }

        // Check if Lua is valid
        bool luaValid = sharedLua && sharedLua->lua_state();

        if (luaValid)
        {
            auto& scriptArray = pCoordinator->GetComponentArray<LuaScript>();

            // Call OnDestroy while Lua is alive
            for (auto const& entity : aEntities)
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
                        catch (...)
                        {
                            // Ignore errors during shutdown
                        }
                    }
                }
            }

            // Clear ALL environments while Lua is still alive
            Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo,
                "Clearing all environments");

            for (auto const& entity : aEntities)
            {
                if (!scriptArray.Has(entity)) continue;

                auto& scriptComponent = scriptArray.GetData(entity);

                for (auto& script : scriptComponent.scripts)
                {
                    try
                    {
                        script.scriptEnv = sol::nil;
                    }
                    catch (...)
                    {
                        // Ignore
                    }
                }
            }

            // Run GC
            try
            {
                sharedLua->collect_garbage();
                sharedLua->collect_garbage();
            }
            catch (...)
            {
                // Ignore
            }
        }

        // Destroy Lua state
        sharedLua = nullptr;

        // Clear C++ structures (environments already nil)
        auto& scriptArray = pCoordinator->GetComponentArray<LuaScript>();

        for (auto const& entity : aEntities)
        {
            if (!scriptArray.Has(entity)) continue;

            auto& scriptComponent = scriptArray.GetData(entity);

            for (auto& script : scriptComponent.scripts)
            {
                //script.callbacks = LuaScriptInstance::CallbackCache{};
                script.isInitialized = false;
                script.hasError = false;
            }

            scriptComponent.scripts.clear();
        }

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
    }

    void LuaScriptingSystem::RegisterComponentTypes()
    {
        // Register Transform
        sharedLua->new_usertype<Transform>("Transform",
            "position", &Transform::position,
            "rotation", &Transform::rotation,
            "scale", &Transform::scale
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
            "textureName", &Sprite::textureName,
            "renderLayer", &Sprite::renderLayer,
            "flipX", &Sprite::flipX,
            "flipY", &Sprite::flipY
        );

        // Register Player component - ADD THIS IF MISSING
        sharedLua->new_usertype<Player>("Player",
            "mSpeed", &Player::mSpeed
        );

        // Register Enemy component - ADD THIS IF MISSING
        sharedLua->new_usertype<Enemy>("Enemy",
            "mSpeed", &Enemy::mSpeed
        );

        // Register Camera
        sharedLua->new_usertype<Camera>("Camera",
            "zoom", &Camera::mZoom,
            "followPlayer", &Camera::followPlayer
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

       // Component list macro
#define COMPONENT_LIST \
        X(Transform)   \
        X(RigidBody)   \
        X(Sprite)      \
        X(Collider)    \
        X(Player)      \
        X(Enemy)       \
        X(Camera)

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
    }

    void LuaScriptingSystem::InitializeScripts(Entity entity, LuaScript& scriptComponent)
    {
        for (auto& script : scriptComponent.scripts)
        {
            if (!script.isInitialized)
            {
                InitializeScript(entity, script);
            }
        }
    }

    void LuaScriptingSystem::InitializeScript(Entity entity, LuaScriptInstance& script)
    {
        if (!sharedLua || !sharedLua->lua_state())
        {
            script.hasError = true;
            script.errorMessage = "Shared Lua state is invalid";
            Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eError,
                "Cannot initialize script: Shared Lua state is invalid");
            return;
        }

        script.scriptEnv = sol::environment(*sharedLua, sol::create, sharedLua->globals());

            // Bind entity-specific functions to this environment
        BindEntityAPI(entity, script.scriptEnv);

       // load and run the script
        try
        {
            auto result = sharedLua->script_file(script.scriptPath, script.scriptEnv);

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

        // Sync variables to Lua BEFORE calling Start()
        SyncVariablesToLua(script);

        // cache the function to the callback
        CacheCallbacks(script);

        script.isVariableDirty = false;
        script.isInitialized = true;

        Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo,
           "Lua script loaded: " + script.scriptPath);
    }

    void LuaScriptingSystem::CacheCallbacks(LuaScriptInstance& script)
    {
        /*sol::optional<sol::protected_function> onCollision = script.scriptEnv["OnCollision"];
        if (onCollision)
        {
            script.callbacks.hasOnCollision = true;
            script.callbacks.onCollisionFunc = *onCollision;
        }*/
    }

    template <typename... Args>
    void LuaScriptingSystem::CallCachedFunction(LuaScriptInstance& script, 
                                                sol::protected_function& func, 
                                                Args&&... args)
    {
        if (!func.valid() || func == sol::nil)
        {
            return;
        }

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
        /*if (!scriptArray.Has(owner)) return;

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
        }*/
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
        BIND_COMPONENT_GETTER(Collider);
        BIND_COMPONENT_GETTER(Player);
        BIND_COMPONENT_GETTER(Enemy);
        BIND_COMPONENT_GETTER(Camera);


        // debugging


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

        InitializeScript(entity, script);

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

    template<typename Func>
    void LuaScriptingSystem::BindInputFunction(const char* name, Func func)
    {
        sharedLua->set_function(name, [this, func, name](int param) -> bool {
            if (!pInputSystem) {
                Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eWarning,
                    std::string("Lua: InputSystem not available for ") + name);
                return false;
            }
            return std::invoke(func, param);
            });
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
            return pInputSystem ? pInputSystem->GetMousePosition() : Vec2{ 0, 0 };
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

    void LuaScriptingSystem::OnEntityDestroyed(Entity entity)
    {
        Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo,
            "=== OnEntityDestroyed START for entity: " + std::to_string(entity));

        if (!pCoordinator)
        {
            Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo,
                "pCoordinator is null - early exit");
            return;
        }

        auto& scriptArray = pCoordinator->GetComponentArray<LuaScript>();
        if (!scriptArray.Has(entity))
        {
            Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo,
                "Entity has no LuaScript component");
            return;
        }

        auto& scriptComponent = scriptArray.GetData(entity);

        if (scriptComponent.scripts.empty())
        {
            Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo,
                "Scripts already cleared");
            return;
        }

        // Check if Lua is still valid
        bool luaValid = sharedLua && sharedLua->lua_state();

        Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo,
            "Lua valid: " + std::string(luaValid ? "yes" : "no") +
            ", cleaning " + std::to_string(scriptComponent.scripts.size()) + " scripts");

        for (auto& script : scriptComponent.scripts)
        {
            // Call OnDestroy if Lua is valid
            if (luaValid && script.isEnabled && script.isInitialized)
            {
                try
                {
                    Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo,
                        "Calling OnDestroy for: " + script.scriptPath);

                    CallLuaFunction(script, "OnDestroy");
                }
                catch (const std::exception& e)
                {
                    Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eWarning,
                        "OnDestroy error: " + std::string(e.what()));
                }
                catch (...)
                {
                    Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eWarning,
                        "OnDestroy unknown error");
                }
            }

            // CRITICAL: Clear environment if Lua is valid
            if (luaValid)
            {
                try
                {
                    Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo,
                        "Clearing environment for: " + script.scriptPath);

                    script.scriptEnv = sol::nil;
                }
                catch (const std::exception& e)
                {
                    Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eWarning,
                        "Environment clear error: " + std::string(e.what()));
                }
                catch (...)
                {
                    Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eWarning,
                        "Environment clear unknown error");
                }
            }

            // Always clear C++ structures
            //script.callbacks = LuaScriptInstance::CallbackCache{};
            script.isInitialized = false;
            script.hasError = false;
        }

        scriptComponent.scripts.clear();

        Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo,
            "=== OnEntityDestroyed COMPLETE for entity: " + std::to_string(entity));
    }
}