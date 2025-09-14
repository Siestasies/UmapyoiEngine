#pragma once

#include "Types.h"
#include "ComponentArray.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <cassert>

namespace Uma_ECS
{
    class ComponentManager
    {
    public:

        // Always register the component before use
        template<typename T>
        void RegisterComponent();

        template<typename T>
        ComponentType GetComponentType();

        template<typename T>
        void AddComponent(Entity entity, const T& component);

        template<typename T>
        void RemoveComponent(Entity entity);

        template<typename T>
        T& GetComponent(Entity entity);

        void EntityDestroyed(Entity entity);

    private:

        // Unordered map that maps the name of the component to ComponentType
        std::unordered_map<std::string, ComponentType> aComponentTypes{};

        // Store polymorphic arrays using shared_ptr instead of unique_ptr
        std::unordered_map<std::string, std::shared_ptr<BaseComponentArray>> aComponentArrays{};

        ComponentType mNextComponentType{};

        template<typename T>
        ComponentArray<T>& GetComponentArray()
        {
            std::string type_name = std::string(typeid(T).name());

            assert(aComponentTypes.find(type_name) != aComponentTypes.end()
                && "Error : Component is not registered before.");

            // Use static_pointer_cast to get the derived type back
            return *std::static_pointer_cast<ComponentArray<T>>(aComponentArrays[type_name]);
        }
    };
}
