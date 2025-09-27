#pragma once

#include "Types.hpp"
#include "ComponentArray.hpp"
#include <unordered_map>
#include <string>
#include <memory>
#include <cassert>

#include <Debugging/Debugger.hpp>

#include "rapidjson/document.h"		// rapidjson's DOM-style API

namespace Uma_ECS
{
    class ComponentManager
    {
    public:

        // Always register the component before use
        template<typename T>
        void RegisterComponent()
        {
            std::string type_name = std::string(typeid(T).name());

            // logging
            std::string debugLog = "Registered component: " + type_name;
            Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo, debugLog);

            // error
            if (aComponentTypes.find(type_name) != aComponentTypes.end())
            {
                debugLog = "Component<" + type_name + "> being registered more than once. ";
                Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eError, debugLog);
            }

            assert(aComponentTypes.find(type_name) == aComponentTypes.end()
                && "Error : Component being registered more than once.");



            aComponentTypes.insert({ type_name, mNextComponentType });

            // Use make_shared instead of make_unique
            aComponentArrays.insert({ type_name, std::make_shared<ComponentArray<T>>() });

            ++mNextComponentType;
        }

        template<typename T>
        ComponentType GetComponentType() 
        {
            std::string type_name = std::string(typeid(T).name());

            assert(aComponentTypes.find(type_name) != aComponentTypes.end()
                && "Error : Component is not registered.");

            return aComponentTypes[type_name];
        }

        template<typename T>
        void AddComponent(Entity entity, const T& component)
        {
            ComponentArray<T>& component_array = GetComponentArray<T>();
            component_array.AddData(entity, component);
        }

        template<typename T>
        void RemoveComponent(Entity entity) 
        {
            ComponentArray<T>& component_array = GetComponentArray<T>();
            component_array.RemoveData(entity);
        }

        template<typename T>
        T& GetComponent(Entity entity)
        {
            ComponentArray<T>& component_array = GetComponentArray<T>();
            return component_array.GetData(entity);
        }

        template<typename T>
        ComponentArray<T>& GetComponentArray()
        {
            std::string type_name = std::string(typeid(T).name());

            assert(aComponentTypes.find(type_name) != aComponentTypes.end()
                && "Error : Component is not registered before.");

            // Use static_pointer_cast to get the derived type back
            return *std::static_pointer_cast<ComponentArray<T>>(aComponentArrays[type_name]);
        }

        void EntityDestroyed(Entity entity);

        void CloneEntityComponents(Entity src, Entity dest);

        void SerializeAll(Entity entity, rapidjson::Value& comps, rapidjson::Document::AllocatorType& allocator) 
        {
            for (auto const& pair : aComponentArrays) 
            {
                pair.second->Serialize(entity, comps, allocator);
            }
        }

        void DeserializeAll(Entity entity, const rapidjson::Value& comps) 
        {
            for (auto const& pair : aComponentArrays) 
            {
                pair.second->Deserialize(entity, comps);
            }
        }

    private:

        // Unordered map that maps the name of the component to ComponentType
        std::unordered_map<std::string, ComponentType> aComponentTypes{};

        std::unordered_map<std::string, std::shared_ptr<BaseComponentArray>> aComponentArrays{};

        ComponentType mNextComponentType{};
    };
}
