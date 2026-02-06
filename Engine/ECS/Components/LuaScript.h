/*!
\file   LuaScript.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines Lua scripting component for entity behavior programming via embedded Lua scripts.

Contains LuaScript component with multiple script instances per entity. Each LuaScriptInstance
maintains isolated Sol2 environment, exposed variables for editor manipulation, and runtime state
tracking. Supports variable types (float, int, bool, string) with optional slider ranges for
ImGui visualization. Provides JSON serialization for script paths, enabled state, and exposed
variables. Script environments use shared_ptr to prevent Sol2 crashes during component array
reordering. Core component for gameplay scripting with full ECS integration via LuaScriptingSystem.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/


#pragma once

#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include "RapidJSON/document.h"

// using sol to do scripting
#pragma warning(push)
#pragma warning(disable: 4244 6287 26498 5321)
#include <sol/sol.hpp>
#pragma warning(pop)

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
				std::string scriptName;
				std::vector<LuaVariable> exposedVariables;

				// runtime data dont need serialization
				std::shared_ptr<sol::environment> scriptEnv;					// Isolated environment per script
				bool isInitialized = false;
				bool hasError = false;
				std::string errorMessage;
				bool isEnabled = true;
				bool isVariableDirty = false;				// set this to true when user set sth to the imgui
				bool wasEnabledLastFrame = false;		// keep track whether the script was previously being eanbled / disabled

				// this sol::environment is really pain in the ass to deal with
				// the purpose of making it a shared ptr
				// is beacause sol::environment is not compatible with being moving around
				// but my component array remove data by swaping it to the last and pop it to remove
				// then it caused sol::environment to crash
				// hence i made it as a shared ptr so it can be moved smoothly 
				// it has more freedom and dont need me to handle the destrruction
				// fuiyooo
				
				//check if null (thanks kai yang >:( )
				/*bool operator!() const noexcept {
					return !scriptEnv || !isInitialized || hasError;
				}*/



				void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const
				{
						value.SetObject();

						value.AddMember("scriptPath",
								rapidjson::Value(scriptPath.c_str(), allocator), 
								allocator);

						value.AddMember("scriptName",
							rapidjson::Value(scriptName.c_str(), allocator),
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

						if (!value.HasMember("scriptName"))
						{
							scriptName = scriptPath.substr(scriptPath.find_last_of('/') + 1);
						}
						else
						{
							scriptName = value["scriptName"].GetString();
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

				//std::shared_ptr<sol::state> lua;

				// add script 
				void AddScript(const std::string& scriptPath)
				{
						LuaScriptInstance instance;
						instance.scriptPath = scriptPath;

						//scriptPath.substr(scriptPath.find_last_of('/'), scriptPath.find(".lua"));

						instance.scriptName = scriptPath.substr(scriptPath.find_last_of('/') + 1);
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

				// get script 
				LuaScriptInstance* GetScriptByName(const std::string& name)
				{
						for (LuaScriptInstance& script : scripts)
						{
								if (script.scriptName == name)
								{
										return &script;
								}
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