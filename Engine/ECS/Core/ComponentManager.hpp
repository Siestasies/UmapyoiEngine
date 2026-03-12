/*!
\file   ComponentManager.hpp
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

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

        ComponentManager() = default;

        /*!
        \brief Copy constructor. Deep copies all component type mappings and component arrays.
        \param other The ComponentManager instance to copy from.
        */
        ComponentManager(const ComponentManager& other) noexcept;

        /*!
        \brief Registers a new component type, creating its ComponentArray and assigning a unique ComponentType ID.
        */
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

            RegisterComponentFriendlyName<T>();

            aComponentTypes.insert({ type_name, mNextComponentType });

            // Use make_shared instead of make_unique
            aComponentArrays.insert({ type_name, std::make_shared<ComponentArray<T>>() });

            ++mNextComponentType;
        }

        /*!
        \brief Returns the unique ComponentType ID for the given component type.
        \return The ComponentType identifier.
        */
        template<typename T>
        ComponentType GetComponentType()
        {
            std::string type_name = std::string(typeid(T).name());

            assert(aComponentTypes.find(type_name) != aComponentTypes.end()
                && "Error : Component is not registered.");

            return aComponentTypes[type_name];
        }

        /*!
        \brief Returns the ComponentType ID for a component identified by its type name string.
        \param compType The typeid name string of the component.
        \return The ComponentType identifier.
        */
        ComponentType GetComponentType(const std::string& compType)
        {
            assert(aComponentTypes.find(compType) != aComponentTypes.end()
                && "Error : Component is not registered.");

            return aComponentTypes[compType];
        }

        /*!
        \brief Adds a component to the specified entity's ComponentArray.
        \param entity The Entity ID to add the component to.
        \param component The component data to add.
        */
        template<typename T>
        void AddComponent(Entity entity, const T& component)
        {
            ComponentArray<T>& component_array = GetComponentArray<T>();
            component_array.AddData(entity, component);
        }

        /*!
        \brief Removes a component from the specified entity.
        \param entity The Entity ID to remove the component from.
        */
        template<typename T>
        void RemoveComponent(Entity entity)
        {
            ComponentArray<T>& component_array = GetComponentArray<T>();
            component_array.RemoveData(entity);
        }

        /*!
        \brief Retrieves a reference to the component data for the specified entity.
        \param entity The Entity ID to get the component from.
        \return A reference to the component data.
        */
        template<typename T>
        T& GetComponent(Entity entity)
        {
            ComponentArray<T>& component_array = GetComponentArray<T>();
            return component_array.GetData(entity);
        }

        /*!
        \brief Checks whether the specified entity has a component of the given type.
        \param entity The Entity ID to check.
        \return True if the entity has the component, false otherwise.
        */
        template<typename T>
        bool HasComponent(Entity entity)
        {
            ComponentArray<T>& component_array = GetComponentArray<T>();
            return component_array.Has(entity);
        }

        /*!
        \brief Retrieves all entities that have a component matching the given friendly name string.
        \param componentName The friendly name of the component type.
        \return A vector of Entity IDs that have the component.
        */
        std::vector<Entity> GetEntitiesByComponentName(const std::string& componentName);

        /*!
        \brief Returns a reference to the typed ComponentArray for the given component type.
        \return A reference to the ComponentArray of type T.
        */
        template<typename T>
        ComponentArray<T>& GetComponentArray()
        {
            std::string type_name = std::string(typeid(T).name());

            assert(aComponentTypes.find(type_name) != aComponentTypes.end()
                && "Error : Component is not registered before.");

            // Use static_pointer_cast to get the derived type back
            return *std::static_pointer_cast<ComponentArray<T>>(aComponentArrays[type_name]);
        }

        /*!
        \brief Registers a human-readable friendly name mapping for the given component type.
        Used by the Lua scripting system for component lookup by name.
        */
        template<typename T>
        void RegisterComponentFriendlyName()
        {
            std::string type_name = std::string(typeid(T).name());
            std::string friendly_name = GetFriendlyName<T>();

            aFriendlyNameToTypeName[friendly_name] = type_name;

            std::string debugLog = "Registered friendly name: " + friendly_name + " -> " + type_name;
            Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo, debugLog);
        }
        
        /*!
        \brief Extracts a short, human-readable name from the full typeid name of a component.
        \return The friendly name string (class name without namespace or prefix).
        */
        template<typename T>
        std::string GetFriendlyName()
        {
            // Extract class name from full type name
            std::string fullName = typeid(T).name();

            // Remove "struct " or "class " prefix (MSVC)
            size_t pos = fullName.find_last_of(" ");
            if (pos != std::string::npos)
                fullName = fullName.substr(pos + 1);

            // Remove namespace (everything before ::)
            pos = fullName.find_last_of(":");
            if (pos != std::string::npos)
                fullName = fullName.substr(pos + 1);

            return fullName;
        }

        /*!
        \brief Notifies all ComponentArrays that an entity has been destroyed, removing its data.
        \param entity The Entity ID that was destroyed.
        */
        void EntityDestroyed(Entity entity);

        /*!
        \brief Clones all components from a source entity to a destination entity.
        \param src The source Entity ID to copy from.
        \param dest The destination Entity ID to copy to.
        */
        void CloneEntityComponents(Entity src, Entity dest);

        /*!
        \brief Serializes all components of an entity into a RapidJSON value.
        \param entity The Entity ID to serialize.
        \param comps The RapidJSON value to write component data into.
        \param allocator The RapidJSON allocator for memory management.
        */
        void SerializeAll(Entity entity, rapidjson::Value& comps, rapidjson::Document::AllocatorType& allocator) 
        {
            for (auto const& pair : aComponentArrays) 
            {
                pair.second->Serialize(entity, comps, allocator);
            }
        }

        /*!
        \brief Deserializes all components for an entity from a RapidJSON value and returns the resulting signature.
        \param entity The Entity ID to deserialize components for.
        \param comps The RapidJSON value containing serialized component data.
        \return The Signature bitset representing all deserialized components.
        */
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

        // ugly approach
        std::unordered_map<std::string, std::string> aFriendlyNameToTypeName{};

        ComponentType mNextComponentType{};
    };
}
