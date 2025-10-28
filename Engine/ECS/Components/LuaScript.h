#pragma once

#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include "RapidJSON/document.h"

// using sol to do scripting
#include <sol/sol.hpp>

namespace Uma_ECS
{
    using LuaValue = std::variant<float, int, bool, std::string>;

		enum class LuaVarType
		{
				T_FLOAT,
				T_INT,
				T_BOOL,
				T_STRING
		};

		struct LuaVariable
		{
				std::string name;
				LuaValue value;
				LuaVarType type;

				float min = 0.f;
				float max = 100.f;
				bool isSlider = false;
		};

		struct LuaScriptInstance
		{
				std::string scriptPath;
				std::vector<LuaVariable> exposedVariables;

				// runtime data dont need serialization
				//std::shared_ptr<sol::state> lua;		// Sol state instead of lua_State*
				sol::environment scriptEnv;					// Isolated environment per script
				bool isInitialized = false;
				bool hasError = false;
				std::string errorMessage;
				bool isEnabled = true;
				bool isVariableDirty = false;				// set this to true when user set sth to the imgui
				bool wasEnabledLastFrame = false;		// keep track whether the script was previously being eanbled / disabled

				
				// CALLBACK CACHE: Store which callbacks exist + their functions
				struct CallbackCache
				{
						// Collision callbacks
						bool hasOnCollision = false;
						bool hasOnCollisionEnter = false;
						bool hasOnCollisionExit = false;

						// Trigger callbacks
						bool hasOnTriggerEnter = false;
						bool hasOnTriggerExit = false;

						//// Damage/Health callbacks
						//bool hasOnDamage = false;
						//bool hasOnHeal = false;
						//bool hasOnDeath = false;

						//// Interaction callbacks
						//bool hasOnInteract = false;
						//bool hasOnPickup = false;

						// Cached function references (avoids repeated Lua table lookups)
						sol::protected_function onCollisionFunc;
						sol::protected_function onCollisionEnterFunc;
						sol::protected_function onCollisionExitFunc;
						sol::protected_function onTriggerEnterFunc;
						sol::protected_function onTriggerExitFunc;
						//sol::protected_function onDamageFunc;
						//sol::protected_function onHealFunc;
						//sol::protected_function onDeathFunc;
						//sol::protected_function onInteractFunc;
						//sol::protected_function onPickupFunc;
				} callbacks;

				void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const
				{
						value.SetObject();

						value.AddMember("scriptPath",
								rapidjson::Value(scriptPath.c_str(), allocator), 
								allocator);

						value.AddMember("isEnabled", isEnabled, allocator);

						rapidjson::Value varsArray(rapidjson::kArrayType);
						for (const auto& var : exposedVariables)
						{
								// saving the LuaVariable in the exposed data
								rapidjson::Value varObj(rapidjson::kObjectType);

								varObj.AddMember("name",
										rapidjson::Value(var.name.c_str(), allocator),
										allocator);

								varObj.AddMember("type", static_cast<int>(var.type), allocator);

								// diff data type
								switch (var.type)
								{
								case LuaVarType::T_FLOAT:
										varObj.AddMember("value", std::get<float>(var.value), allocator);
										break;
								case LuaVarType::T_INT:
										varObj.AddMember("value", std::get<int>(var.value), allocator);
										break;
								case LuaVarType::T_BOOL:
										varObj.AddMember("value", std::get<bool>(var.value), allocator);
										break;
								case LuaVarType::T_STRING:
										varObj.AddMember("value", 
												rapidjson::Value(std::get<std::string>(var.value).c_str(), allocator), 
												allocator);
										break;
								}

								varObj.AddMember("min", var.min, allocator);
								varObj.AddMember("max", var.max, allocator);
								varObj.AddMember("isSlider", var.isSlider, allocator);

								varsArray.PushBack(varObj, allocator);
						}
						value.AddMember("exposedVariables", varsArray, allocator);
				}

				void Deserialize(const rapidjson::Value& value)
				{
						scriptPath = value["scriptPath"].GetString();

						if (value.HasMember("isEnabled"))
						{
								isEnabled = value["isEnabled"].GetBool();
						}

						if (value.HasMember("exposedVariables") && value["exposedVariables"].IsArray())
						{
								const auto& varsArray = value["exposedVariables"];
								exposedVariables.clear();
								exposedVariables.reserve(varsArray.Size());

								for (const auto& varVal : varsArray.GetArray())
								{
										LuaVariable var;
										var.name = varVal["name"].GetString();
										var.type = static_cast<LuaVarType>(varVal["type"].GetInt());

										// diff data type
										switch (var.type)
										{
										case LuaVarType::T_FLOAT:
												var.value = varVal["value"].GetFloat();
												break;
										case LuaVarType::T_INT:
												var.value = varVal["value"].GetInt();
												break;
										case LuaVarType::T_BOOL:
												var.value = varVal["value"].GetBool();
												break;
										case LuaVarType::T_STRING:
												var.value = std::string(varVal["value"].GetString());
												break;
										}

										if (varVal.HasMember("min"))
												var.min = varVal["min"].GetFloat();
										if (varVal.HasMember("max"))
												var.max = varVal["max"].GetFloat();
										if (varVal.HasMember("isSlider"))
												var.isSlider = varVal["isSlider"].GetBool();

										exposedVariables.push_back(var);
								}

								isVariableDirty = true;
						}
				}
		};

		struct LuaScript
		{
				std::vector<LuaScriptInstance> scripts;

				std::shared_ptr<sol::state> lua;

				// add script 
				void AddScript(const std::string& scriptPath)
				{
						LuaScriptInstance instance;
						instance.scriptPath = scriptPath;
						instance.isEnabled = true;
						scripts.push_back(instance);
				}

				// remove script
				void RemoveScript(size_t idx)
				{
						if (idx < scripts.size())
						{
								scripts.erase(scripts.begin() + idx);
						}
				}

				// get script 
				LuaScriptInstance* GetScript(size_t idx)
				{
						if (idx < scripts.size())
						{
								return &scripts[idx];
						}

						return nullptr;
				}

				// get script by path
				LuaScriptInstance* GetScriptByPath(const std::string& path)
				{
						for (LuaScriptInstance& script : scripts)
						{
								if (script.scriptPath == path)
								{
										return &script;
								}
						}

						return nullptr;
				}

				void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const
				{
						value.SetObject();

						rapidjson::Value scriptsArray(rapidjson::kArrayType);
						for (const auto& script : scripts)
						{
								rapidjson::Value scriptObj(rapidjson::kObjectType);
								script.Serialize(scriptObj, allocator);
								scriptsArray.PushBack(scriptObj, allocator);
						}
						value.AddMember("scripts", scriptsArray, allocator);
				}

				void Deserialize(const rapidjson::Value& value)
				{
						if (value.HasMember("scripts") && value["scripts"].IsArray())
						{
								const auto& scriptsArray = value["scripts"];
								scripts.clear();
								scripts.reserve(scriptsArray.Size());

								for (const auto& scriptVal : scriptsArray.GetArray())
								{
										LuaScriptInstance instance;
										instance.Deserialize(scriptVal);
										scripts.push_back(instance);
								}
						}
				}
		};
}