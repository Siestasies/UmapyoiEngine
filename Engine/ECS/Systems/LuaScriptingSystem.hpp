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
        void CacheCallbacks(LuaScriptInstance& script);         // caching the callback (stroing the callbacks in the script instance)

        template <typename... Args>
        void CallCachedFunction(LuaScriptInstance& script, sol::protected_function& func, Args&&... args);
        
        void OnCollisionEvent(Entity entityA, Entity entityB);  // 

        Uma_Engine::EventSystem* pEventSystem = nullptr;
        Coordinator* pCoordinator = nullptr;
        std::shared_ptr<sol::state> sharedLua;

        float lastDeltaTime{};
    };
}