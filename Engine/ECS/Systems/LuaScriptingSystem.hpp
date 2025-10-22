#pragma once

#include "../Core/System.hpp"
#include "../Core/Coordinator.hpp"
#include "../Components/LuaScript.h"
#include "../Components/Transform.h"
#include "../Components/RigidBody.h"
#include "../Components/Sprite.h"
#include "Debugging/Debugger.hpp"

#include <sol/sol.hpp>

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
        void RegisterLuaAPI();
        void InitializeScript(Entity entity, LuaScript& script);
        void BindEntityAPI(Entity entity, sol::environment& env);
        void DiscoverExposedVariables(LuaScript& script);
        void SyncVariablesToLua(LuaScript& script);
        void SyncVariablesFromLua(LuaScript& script);
        void CallLuaFunction(LuaScript& script, const char* funcName, float dt = 0.f);

        Coordinator* pCoordinator = nullptr;
        std::shared_ptr<sol::state> sharedLua;
    };
}