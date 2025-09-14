//#include "Precompiled.h"
#include "ComponentManager.h"
#include "Coordinator.h"

template<typename T>
void Uma_ECS::ComponentManager::RegisterComponent()
{
    std::string type_name = std::string(typeid(T).name());

    assert(aComponentTypes.find(type_name) == aComponentTypes.end()
        && "Error : Component being registered more than once.");

    aComponentTypes.insert({ type_name, mNextComponentType });

    // Use make_shared instead of make_unique
    aComponentArrays.insert({ type_name, std::make_shared<ComponentArray<T>>() });

    ++mNextComponentType;
}

template<typename T>
Uma_ECS::ComponentType Uma_ECS::ComponentManager::GetComponentType()
{
    std::string type_name = std::string(typeid(T).name());

    assert(aComponentTypes.find(type_name) != aComponentTypes.end()
        && "Error : Component is not registered.");

    return aComponentTypes[type_name];
}

template<typename T>
void Uma_ECS::ComponentManager::AddComponent(Entity entity, const T& component)
{
    ComponentArray<T>& component_array = GetComponentArray<T>();
    component_array.AddData(entity, component);
}

template<typename T>
void Uma_ECS::ComponentManager::RemoveComponent(Entity entity)
{
    ComponentArray<T>& component_array = GetComponentArray<T>();
    component_array.RemoveData(entity);
}

template<typename T>
T& Uma_ECS::ComponentManager::GetComponent(Entity entity)
{
    ComponentArray<T>& component_array = GetComponentArray<T>();
    return component_array.GetData(entity);
}

void Uma_ECS::ComponentManager::EntityDestroyed(Entity entity)
{
    for (auto const& pair : aComponentArrays)
    {
        auto const& component = pair.second;

        component->DestroyEntity(entity);
    }
}
