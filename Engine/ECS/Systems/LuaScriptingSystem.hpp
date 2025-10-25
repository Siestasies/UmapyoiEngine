#pragma once

#include "../Core/System.hpp"
#include "../Core/Coordinator.hpp"
#include "../Components/LuaScript.h"
#include "Debugging/Debugger.hpp"
#include "Core/EventSystem.h"

#include <sol/sol.hpp>
#include <memory>

namespace Uma_ECS
{
    class LuaScriptingSystem : public ECSSystem
    {
    public:

        void Init(Coordinator* c, Uma_Engine::EventSystem* e);
        void Update(float dt);
        void Shutdown();

        void CallStart();

    private:

        // Intialize all scripts for an entity
        void InitializeScripts(Entity entity, LuaScript& scriptComponent);

        // Initialize a single script instance
        void InitializeScript(Entity entity, LuaScriptInstance& script, std::shared_ptr<sol::state> lua);

        void RegisterLuaAPI();
        
        void BindEntityAPI(Entity entity, sol::environment& env);

        void DiscoverExposedVariables(LuaScriptInstance& script);

        void ReloadScript(Entity entity, size_t scriptIndex);
        void SyncVariablesToLua(LuaScriptInstance& script);
        void SyncVariablesFromLua(LuaScriptInstance& script);
        void CallLuaFunction(LuaScriptInstance& script, const char* funcName, float dt = 0.f);

        // NEW SHIT TO DO 
        void RegisterEventListeners();                          // basically subscribe to event then trigger the func

        void NotifyScripts(                                     // this is to let the Lua script to know that there are                                                     
            ComponentArray<LuaScript>& scriptArray,             // events triggering the script
            Entity owner,
            Entity other,
            const char* callbackName);

        void CacheCallbacks(LuaScriptInstance& script);         // caching the callback (stroing the callbacks in the script instance)

        // thi is to call the cached callbacks of the script
        // eg onCollisionEnter, OnTriggerExit, etc...
        template <typename... Args>
        void CallCachedFunction(LuaScriptInstance& script, sol::protected_function& func, Args&&... args);
        
        // These are all callback events
        void OnCollisionEnterEvent(Entity entityA, Entity entityB);
        void OnCollisionExitEvent(Entity entityA, Entity entityB);
        void OnCollisionEvent(Entity entityA, Entity entityB);
        void OnTriggerEnterEvent(Entity entityA, Entity entityB);
        void OnTriggerExitEvent(Entity entityA, Entity entityB);
        void OnTriggerEvent(Entity entityA, Entity entityB);

        Uma_Engine::EventSystem* pEventSystem = nullptr;
        Coordinator* pCoordinator = nullptr;
        std::shared_ptr<sol::state> sharedLua;

        float lastDeltaTime{};
    };
}