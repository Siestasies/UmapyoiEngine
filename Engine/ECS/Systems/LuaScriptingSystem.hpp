/*!
\file   LuaScriptingSystem.hpp
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

#pragma once

#include "../Core/System.hpp"
#include "../Components/LuaScript.h"

// Engine systems
#include "Systems/HybridInputSystem.h"
#include "Core/EventSystem.h"
#include "Debugging/Debugger.hpp"
#include "../Core/Coordinator.hpp"
#include "Systems/ResourcesManager.hpp"
#include "Systems/Graphics.hpp"

// PlayFab
#include "PlayFab/Core/PlayFabManager.h"

// Events
#include "Events/CollisionEvent.h"
#include "Events/AudioEvents.h"
#include "Events/IMGUIEvents.h"
#include "Events/LuaScriptingEvents.h"
#include "Events/FeedbackEvents.h"

//#define SOL_ALL_SAFETIES_ON 1
//#define SOL_PRINT_ERRORS 1
#pragma warning(push)
#pragma warning(disable: 4244 6287 26498 5321)
#include <sol/sol.hpp>
#pragma warning(pop)
#include <memory>

namespace Uma_ECS
{
    class LuaScriptingSystem : public ECSSystem
    {
    public:

        /**
         * \brief Initializes Lua scripting system with required engine dependencies
         * \param c Pointer to ECS coordinator
         * \param e Pointer to event system
         * \param i Pointer to input system
         */
        void Init(Coordinator* c, 
            Uma_Engine::EventSystem* e, 
            Uma_Engine::HybridInputSystem* i, 
            Uma_Engine::ResourcesManager* r,
            Uma_Engine::Graphics* g,
            Uma_Engine::SoundManager* s);

        /**
         * \brief Updates all active Lua scripts with delta time
         * \param dt Delta time in seconds
         */
        void Update(float dt);

        /**
         * \brief Shuts down Lua state and unsubscribes from all events
         */
        void Shutdown();

        /**
         * \brief Restarts all Lua scripts by reinitializing environments
         */
        void Restart();

        /*!
         * \brief Initializes all Lua scripts on all entities that have a LuaScript component.
         */
        void InitializeAllScripts();

        /*!
         * \brief Calls the Start function on all initialized Lua scripts to begin execution.
         */
        void StartScripts();

        /**
         * \brief Sets the PlayFab manager and registers PlayFab Lua API.
         *        Call after both LuaScriptingSystem::Init() and PlayFabManager::Init().
         * \param pfb Pointer to the PlayFabManager (may be nullptr to skip)
         */
        void SetPlayFabManager(Uma_Engine::PlayFabManager* pfb);

        /*!
         * \brief Calls a named function on a specific script attached to an entity.
         * \tparam Args Variadic argument types to forward to the Lua function.
         * \param entity Entity owning the script.
         * \param scriptName Name of the script instance to target.
         * \param functionName Name of the Lua function to invoke.
         * \param args Arguments to pass to the Lua function.
         */
        template<typename... Args>
        void CallScriptFunction(Entity entity, std::string scriptName, std::string functionName, Args&&... args)
        {
            auto& luaScript = pCoordinator->GetComponent<LuaScript>(entity);

            CallLuaFunction(*luaScript.GetScriptByName(scriptName), functionName.c_str(), std::forward<Args>(args)...);
        }


    private:
        /**
         * \brief Initializes all scripts attached to an entity
         * \param entity Entity owning the scripts
         * \param scriptComponent LuaScript component containing script instances
         */
        void InitializeEntityScripts(Entity entity, LuaScript& scriptComponent);

        /**
         * \brief Initializes a single script instance with isolated environment
         * \param entity Entity owning the script
         * \param script Script instance to initialize
         */
        void InitializeEntityScript(Entity entity, LuaScriptInstance& script);

        /**
         * \brief Registers complete Lua API including components, entities, and utilities
         */
        void RegisterLuaAPI();

        /**
         * \brief Binds entity-specific API to script environment
         * \param entity Entity to bind
         * \param env Sol2 environment for binding
         */
        void BindEntityAPI(Entity entity, sol::environment& env);

        /**
         * \brief Discovers exposed variables from Lua script's ExposedVariables table
         * \param script Script instance to scan for variables
         */
        void DiscoverExposedVariables(LuaScriptInstance& script);

        /**
         * \brief Reloads a script by reinitializing its environment
         * \param entity Entity owning the script
         * \param scriptIndex Index of script in LuaScript component
         */
        void RefreshScript(Entity entity, size_t scriptIndex);

        /**
         * \brief Synchronizes C++ exposed variables to Lua environment
         * \param script Script instance to sync
         */
        void SyncVariablesToLua(LuaScriptInstance& script);

        /**
         * \brief Synchronizes Lua environment variables back to C++ storage
         * \param script Script instance to sync from
         */
        void SyncVariablesFromLua(LuaScriptInstance& script);

        /**
         * \brief Safely calls Lua function with error handling
         * \tparam Args Variadic argument types
         * \param script Script instance containing the function
         * \param funcName Name of Lua function to call
         * \param args Arguments to pass to function
         */
        template<typename... Args>
        void CallLuaFunction(LuaScriptInstance& script, const char* funcName, Args&&... args)
        {
            try
            {
                sol::optional<sol::protected_function> func = (*script.scriptEnv)[funcName];

                if (!func)
                {
                    /*Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eError,
                        "function " + std::string(funcName) + "(): " + "is invalid / doesn't exists");*/

                    // if function is invalid just dont do anything

                    return;
                }

                auto result = (*func)(std::forward<Args>(args)...);

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

        /**
         * \brief Subscribes to collision and trigger events
         */
        void SubscribeToEvents();

        /**
         * \brief Unsubscribes from all registered events
         */
        void UnsubscribeEvents();

        /**
         * \brief Notifies scripts of events by calling specified callback function
         * \param scriptArray Component array containing scripts
         * \param owner Entity that owns the scripts
         * \param other Other entity involved in event
         * \param callbackName Name of Lua callback function to invoke
         */
        void NotifyScripts(
            ComponentArray<LuaScript>& scriptArray,
            Entity owner,
            Entity other,
            const char* callbackName);

        /*!
         * \brief Notifies scripts of a trigger event by calling a specified callback function.
         * \param scriptArray Component array containing LuaScript components.
         * \param owner Entity that owns the scripts to notify.
         * \param other Other entity involved in the trigger event.
         * \param triggerOwner Entity that owns the trigger collider.
         * \param callbackName Name of the Lua callback function to invoke.
         */
        void NotifyTriggerScripts(
            ComponentArray<LuaScript>& scriptArray,
            Entity owner,
            Entity other,
            Entity triggerOwner,
            const char* callbackName);

        /**
         * \brief Collision enter event callback
         * \param entityA First entity in collision
         * \param entityB Second entity in collision
         */
        void OnCollisionEnterEvent(Entity entityA, Entity entityB);

        /**
         * \brief Collision exit event callback
         * \param entityA First entity in collision
         * \param entityB Second entity in collision
         */
        void OnCollisionExitEvent(Entity entityA, Entity entityB);

        /**
         * \brief Collision stay event callback
         * \param entityA First entity in collision
         * \param entityB Second entity in collision
         */
        void OnCollisionEvent(Entity entityA, Entity entityB);

        /**
         * \brief Trigger enter event callback
         * \param entityA First entity in trigger
         * \param entityB Second entity in trigger
         */
        void OnTriggerEnterEvent(Entity entityA, Entity entityB, Entity triggerOwner);

        /**
         * \brief Trigger exit event callback
         * \param entityA First entity in trigger
         * \param entityB Second entity in trigger
         */
        void OnTriggerExitEvent(Entity entityA, Entity entityB);

        /**
         * \brief Trigger stay event callback
         * \param entityA First entity in trigger
         * \param entityB Second entity in trigger
         */
        void OnTriggerEvent(Entity entityA, Entity entityB, Entity triggerOwner);

        /**
         * \brief Binds input system function to Lua API
         * \tparam Func Function type to bind
         * \param name Name to expose in Lua
         * \param func Function pointer to bind
         */
        template<typename Func>
        void BindInputFunction(const char* name, Func func)
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

        /**
         * \brief Registers input key/button bindings to Lua
         */
        void RegisterInputBindings();

        /**
         * \brief Registers input key constant values to Lua
         */
        void RegisterKeyConstants();

        /**
         * \brief Registers entity query functions to Lua API
         */
        void RegisterEntityQueries();

        /**
         * \brief Registers utility functions to Lua API
         */
        void RegisterUtilityFunctions();

        /**
         * \brief Registers cross-entity access functions to Lua API
         */
        void RegisterCrossEntityAccess();

        /**
         * \brief Registers component type bindings to Lua API
         */
        void RegisterComponentTypes();

        /**
         * \brief Registers entity creation/destruction functions to Lua API
         */
        void RegisterEntityManipulation();

        /**
         * \brief Registers feedback spawning function to Lua API
         */
        void RegisterFeedbackAPI();

        /**
         * \brief Registers PlayFab API functions to Lua
         */
        void RegisterPlayFabAPI();

        /**
         * \brief Cleanup handler for entity destruction
         * \param entity Entity being destroyed
         */
        void OnEntityDestroyed(Entity entity) override;

        std::shared_ptr<sol::state> sharedLua;

        std::vector<std::shared_ptr<Uma_Engine::IEventListener>> aEventListeners;

        // supporting systems
        Uma_Engine::EventSystem* pEventSystem = nullptr;
        Uma_Engine::HybridInputSystem* pInputSystem = nullptr;
        Coordinator* pCoordinator = nullptr;
        Uma_Engine::ResourcesManager* pResourcesManager = nullptr;
        Uma_Engine::Graphics* pGraphics = nullptr;
        Uma_Engine::SoundManager* pSoundManager = nullptr;
        Uma_Engine::PlayFabManager* pPlayFabManager = nullptr;

        // runtime variables
        float lastDeltaTime{};
    };
}