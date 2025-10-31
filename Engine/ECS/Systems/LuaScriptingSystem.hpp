#pragma once

#include "../Core/System.hpp"
#include "../Components/LuaScript.h"

// Engine systems
#include "WIP_Scripts/Test_Input_Events.h"
#include "Core/EventSystem.h"
#include "Debugging/Debugger.hpp"
#include "../Core/Coordinator.hpp"

#define SOL_ALL_SAFETIES_ON 1
#define SOL_PRINT_ERRORS 1
#include <sol/sol.hpp>
#include <memory>

namespace Uma_ECS
{
    class LuaScriptingSystem : public ECSSystem
    {
    public:

        void Init(Coordinator* c, Uma_Engine::EventSystem* e, Uma_Engine::HybridInputSystem* i);
        void Update(float dt);
        void Shutdown();

        void Restart();

        void CallStart();

    private:

        // Intialize all scripts for an entity
        void InitializeScripts(Entity entity, LuaScript& scriptComponent);

        // Initialize a single script instance
        void InitializeScript(Entity entity, LuaScriptInstance& script);

        void RegisterLuaAPI();
        
        void BindEntityAPI(Entity entity, sol::environment& env);

        void DiscoverExposedVariables(LuaScriptInstance& script);

        void ReloadScript(Entity entity, size_t scriptIndex);
        void SyncVariablesToLua(LuaScriptInstance& script);
        void SyncVariablesFromLua(LuaScriptInstance& script);

        template<typename... Args>
        void CallLuaFunction(LuaScriptInstance& script, const char* funcName, Args&&... args);

        // NEW SHIT TO DO 
        void RegisterEventListeners();                          // basically subscribe to event then trigger the func

        void NotifyScripts(                                     // this is to let the Lua script to know that there are                                                     
            ComponentArray<LuaScript>& scriptArray,             // events triggering the script
            Entity owner,
            Entity other,
            const char* callbackName);
        
        // These are all callback events
        void OnCollisionEnterEvent(Entity entityA, Entity entityB);
        void OnCollisionExitEvent(Entity entityA, Entity entityB);
        void OnCollisionEvent(Entity entityA, Entity entityB);
        void OnTriggerEnterEvent(Entity entityA, Entity entityB);
        void OnTriggerExitEvent(Entity entityA, Entity entityB);
        void OnTriggerEvent(Entity entityA, Entity entityB);

        // InputSystemAPI
        template<typename Func>
        void BindInputFunction(const char* name, Func func);
        void RegisterInputBindings();
        void RegisterKeyConstants();

        // Helper methods for entity Queries
        void RegisterEntityQueries();

        // Helper methods for utility function
        void RegisterUtilityFUnctions();

        // Helper methods to cross access other entities
        void RegisterCrossEntityAccess();

        // Register component types
        void RegisterComponentTypes();

        // Register entity manipulation
        void RegisterEntityManipulation();

        // on entity destroy
        void OnEntityDestroyed(Entity entity) override; 
        

        std::shared_ptr<sol::state> sharedLua;

        // supporting systems
        Uma_Engine::EventSystem* pEventSystem = nullptr;
        Uma_Engine::HybridInputSystem* pInputSystem = nullptr;
        Coordinator* pCoordinator = nullptr;

        // runtime variables
        float lastDeltaTime{};
    };
}