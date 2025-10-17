/*!
\file   ComponentManager.hpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Manages registration, storage, and retrieval of all component types in the ECS using type-indexed maps.

Maps component type names (via typeid) to unique ComponentType identifiers and corresponding ComponentArray instances.
Provides template-based API for type-safe component operations (add, remove, get) with compile-time type resolution.
Handles batch serialization/deserialization of all components for a given entity, returning signatures for deserialized components.
Uses shared pointers for polymorphic component array storage and maintains component type counter for unique identification.
Integrates with Uma_Engine debugger for component registration logging.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

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

        ComponentType GetComponentType(const std::string& compType)
        {
            assert(aComponentTypes.find(compType) != aComponentTypes.end()
                && "Error : Component is not registered.");

            return aComponentTypes[compType];
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

        Signature DeserializeAll(Entity entity, const rapidjson::Value& comps) 
        {
            Signature sign;
            for (auto const& pair : aComponentArrays) 
            {
                std::string compType = pair.second->Deserialize(entity, comps); // ""

                if (!compType.empty())
                {
                    ComponentType typeIndex = GetComponentType(compType); // returns int
                    sign.set(typeIndex);  // << set the bit
                }
            }
            return sign;
        }

    private:

        // Unordered map that maps the name of the component to ComponentType
        std::unordered_map<std::string, ComponentType> aComponentTypes{};

        std::unordered_map<std::string, std::shared_ptr<BaseComponentArray>> aComponentArrays{};

        ComponentType mNextComponentType{};
    };
}
