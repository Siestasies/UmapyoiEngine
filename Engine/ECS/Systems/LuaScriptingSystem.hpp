#pragma once

#include "../Core/System.hpp"
#include "../Core/Coordinator.hpp"
#include "../Components/LuaScript.h"
#include "Debugging/Debugger.hpp"

#include <sol/sol.hpp>
#include <memory>

namespace Uma_ECS
{
    class LuaScriptingSystem : public ECSSystem
    {
    public:

        void Init(Coordinator* c);
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

        void SyncVariablesToLua(LuaScriptInstance& script);
        void SyncVariablesFromLua(LuaScriptInstance& script);
        void CallLuaFunction(LuaScriptInstance& script, const char* funcName, float dt = 0.f);

        Coordinator* pCoordinator = nullptr;
        std::shared_ptr<sol::state> sharedLua;
    };
}