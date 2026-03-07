#pragma once
#include <string>
#include <unordered_map>
#include <variant>
#include <glm/glm.hpp>
#include "rapidjson/document.h"

namespace Uma_ECS
{
    using MaterialValue = std::variant<float, glm::vec2, glm::vec3, glm::vec4, int>;

    struct SpriteMaterial
    {
        std::string effectName{};  // key into ResourcesManager effect map, empty = default
        std::unordered_map<std::string, MaterialValue> properties{};

        // Setters (used by inspector + Lua)
        void SetFloat(const std::string& name, float v) { properties[name] = v; }
        void SetVec2(const std::string& name, glm::vec2 v) { properties[name] = v; }
        void SetVec3(const std::string& name, glm::vec3 v) { properties[name] = v; }
        void SetVec4(const std::string& name, glm::vec4 v) { properties[name] = v; }
        void SetInt(const std::string& name, int v) { properties[name] = v; }

        // Serialization
        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const
        {
            value.SetObject();
            value.AddMember("effectName",
                rapidjson::Value(effectName.c_str(), allocator), allocator);

            rapidjson::Value propsArr(rapidjson::kArrayType);
            for (const auto& [name, val] : properties)
            {
                rapidjson::Value obj(rapidjson::kObjectType);
                obj.AddMember("name",
                    rapidjson::Value(name.c_str(), allocator), allocator);

                std::visit([&](auto&& v)
                    {
                        using T = std::decay_t<decltype(v)>;
                        if constexpr (std::is_same_v<T, float>)
                        {
                            obj.AddMember("type", "float", allocator);
                            obj.AddMember("value", v, allocator);
                        }
                        else if constexpr (std::is_same_v<T, int>)
                        {
                            obj.AddMember("type", "int", allocator);
                            obj.AddMember("value", v, allocator);
                        }
                        else if constexpr (std::is_same_v<T, glm::vec2>)
                        {
                            obj.AddMember("type", "vec2", allocator);
                            rapidjson::Value arr(rapidjson::kArrayType);
                            arr.PushBack(v.x, allocator);
                            arr.PushBack(v.y, allocator);
                            obj.AddMember("value", arr, allocator);
                        }
                        else if constexpr (std::is_same_v<T, glm::vec3>)
                        {
                            obj.AddMember("type", "vec3", allocator);
                            rapidjson::Value arr(rapidjson::kArrayType);
                            arr.PushBack(v.x, allocator);
                            arr.PushBack(v.y, allocator);
                            arr.PushBack(v.z, allocator);
                            obj.AddMember("value", arr, allocator);
                        }
                        else if constexpr (std::is_same_v<T, glm::vec4>)
                        {
                            obj.AddMember("type", "vec4", allocator);
                            rapidjson::Value arr(rapidjson::kArrayType);
                            arr.PushBack(v.x, allocator);
                            arr.PushBack(v.y, allocator);
                            arr.PushBack(v.z, allocator);
                            arr.PushBack(v.w, allocator);
                            obj.AddMember("value", arr, allocator);
                        }
                    }, val);

                propsArr.PushBack(obj, allocator);
            }
            value.AddMember("properties", propsArr, allocator);
        }

        void Deserialize(const rapidjson::Value& value)
        {
            if (value.HasMember("effectName"))
                effectName = value["effectName"].GetString();

            if (value.HasMember("properties") && value["properties"].IsArray())
            {
                for (const auto& prop : value["properties"].GetArray())
                {
                    std::string name = prop["name"].GetString();
                    std::string type = prop["type"].GetString();

                    if (type == "float")
                        properties[name] = prop["value"].GetFloat();
                    else if (type == "int")
                        properties[name] = prop["value"].GetInt();
                    else if (type == "vec2")
                    {
                        auto arr = prop["value"].GetArray();
                        properties[name] = glm::vec2(arr[0].GetFloat(), arr[1].GetFloat());
                    }
                    else if (type == "vec3")
                    {
                        auto arr = prop["value"].GetArray();
                        properties[name] = glm::vec3(arr[0].GetFloat(), arr[1].GetFloat(), arr[2].GetFloat());
                    }
                    else if (type == "vec4")
                    {
                        auto arr = prop["value"].GetArray();
                        properties[name] = glm::vec4(arr[0].GetFloat(), arr[1].GetFloat(),
                            arr[2].GetFloat(), arr[3].GetFloat());
                    }
                }
            }
        }
    };
}
