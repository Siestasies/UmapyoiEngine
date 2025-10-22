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

		struct LuaScript
		{
				std::string scriptPath;
				std::vector<LuaVariable> exposedVariables;

				// runtime data dont need serialization
				std::shared_ptr<sol::state> lua;		// Sol state instead of lua_State*
				sol::environment scriptEnv;					// Isolated environment per script
				bool isInitialized = false;
				bool hasError = false;
				std::string errorMessage;

				void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const
				{
						value.SetObject();

						value.AddMember("scriptPath",
								rapidjson::Value(scriptPath.c_str(), allocator), 
								allocator);

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
						}
				}
		};
}