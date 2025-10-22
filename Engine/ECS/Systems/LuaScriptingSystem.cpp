#include "LuaScriptingSystem.hpp"

namespace Uma_ECS
{
    void LuaScriptingSystem::Init(Coordinator* c)
    {
        pCoordinator = c;

        // create shared Lua state with all standard libraries
        sharedLua = std::make_shared<sol::state>();
        sharedLua->open_libraries(
            sol::lib::base,
            sol::lib::math,
            sol::lib::string,
            sol::lib::table
        );

        RegisterLuaAPI();
    }

    void LuaScriptingSystem::Update(float dt)
    {
        auto& scriptArray = pCoordinator->GetComponentArray<LuaScript>();

        for (auto const& entity : aEntities)
        {
            auto& script = scriptArray.GetData(entity);

            if (!script.isInitialized)
            {
                InitializeScript(entity, script);
            }

            if (script.hasError)
                continue;

            // sync c++ variables to lua
            SyncVariablesToLua(script);

            // call update
            CallLuaFunction(script, "Update", dt);

            // sync lua variables back to c++
            SyncVariablesFromLua(script);
        }
    }

    void LuaScriptingSystem::CallStart() 
    {
        auto& scriptArray = pCoordinator->GetComponentArray<LuaScript>();

        for (auto const& entity : aEntities)
        {
            auto& script = scriptArray.GetData(entity);

            if (!script.isInitialized)
            {
                InitializeScript(entity, script);
            }

            if (!script.hasError)
            {
                CallLuaFunction(script, "Start");
            }
        }
    }

    void LuaScriptingSystem::Shutdown()
    {
        auto& scriptArray = pCoordinator->GetComponentArray<LuaScript>();

        for (auto const& entity : aEntities)
        {
            auto& script = scriptArray.GetData(entity);
            script.lua.reset();
        }

        sharedLua.reset();
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

    // setting up the environment for the Lua script to run
    void LuaScriptingSystem::InitializeScript(Entity entity, LuaScript& script)
    {
        script.lua = sharedLua; // use shared state

        // creating the isolated environment for this script
        script.scriptEnv = sol::environment(*script.lua, sol::create, script.lua->globals());

        // Bind entity-specific functions to this environment
        BindEntityAPI(entity, script.scriptEnv);

        // load and run the script
        try
        {
            auto result = script.lua->script_file(script.scriptPath, script.scriptEnv);

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

        script.isInitialized = true;

        Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo,
            "Lua script loaded: " + script.scriptPath);
    }

    // basically setting up functions that lua script can use 
    // eg. get transform/rigidbody or other of entity
    void LuaScriptingSystem::BindEntityAPI(Entity entity, sol::environment& env)
    {
        // Get components THIS IS UGLY
        // CAN BE OPTIMISED
        env.set_function("GetTransform", [this, entity]() -> Transform& 
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
            });

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

    }

    void LuaScriptingSystem::DiscoverExposedVariables(LuaScript& script)
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

    void LuaScriptingSystem::SyncVariablesToLua(LuaScript& script)
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

    void LuaScriptingSystem::SyncVariablesFromLua(LuaScript& script)
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


    // Invoking the Lua code
    // BISMILLAH PLEASE WORK
    void LuaScriptingSystem::CallLuaFunction(LuaScript& script, const char* funcName, float dt)
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