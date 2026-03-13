/*!
\file   ComponentManager.cpp
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Implements ComponentManager methods for entity destruction and component cloning across all registered component types.

Iterates through all component arrays to remove components when an entity is destroyed or copy components during entity duplication.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

//#include "Precompiled.h"
#include "ComponentManager.hpp"
#include "Coordinator.hpp"

void Uma_ECS::ComponentManager::EntityDestroyed(Entity entity)
{
    for (auto const& pair : aComponentArrays)
    {
        auto const& component = pair.second;

        component->DestroyEntity(entity);
    }
}

void Uma_ECS::ComponentManager::CloneEntityComponents(Entity src, Entity dest)
{
    for (auto const& pair : aComponentArrays)
    {
        auto const& componentArray = pair.second;

        if (componentArray->Has(src))
        {
            componentArray->CloneComponent(src, dest);
        }
    }
}

Uma_ECS::ComponentManager::ComponentManager(const ComponentManager& other) noexcept
    : aComponentTypes(other.aComponentTypes)
    , aFriendlyNameToTypeName(other.aFriendlyNameToTypeName)
    , mNextComponentType(other.mNextComponentType)
{
    // Deep copy each component array
    for (const auto& [typeName, baseArray] : other.aComponentArrays)
    {
        // Need to implement Clone() in ComponentArray
        aComponentArrays[typeName] = baseArray->CloneArray();
    }
}

std::vector<Uma_ECS::Entity> Uma_ECS::ComponentManager::GetEntitiesByComponentName(const std::string& componentName)
{
    std::vector<Entity> entities;

    // Try friendly name first
    std::string actualTypeName = componentName;
    auto friendlyIt = aFriendlyNameToTypeName.find(componentName);
    if (friendlyIt != aFriendlyNameToTypeName.end())
    {
        actualTypeName = friendlyIt->second;
    }

    auto it = aComponentArrays.find(actualTypeName);
    if (it == aComponentArrays.end())
    {
        Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eWarning,
            "Component not found: " + componentName + " (tried: " + actualTypeName + ")");

        return entities; // doesnt exist
    }

    auto& componentArray = it->second;

    // You'll need to add a virtual method to BaseComponentArray
    // to get all entities
    return componentArray->GetAllEntities();
}
