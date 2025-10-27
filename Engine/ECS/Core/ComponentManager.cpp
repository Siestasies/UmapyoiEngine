/*!
\file   ComponentManager.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

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

std::vector<Uma_ECS::Entity> Uma_ECS::ComponentManager::GetEntitiesByComponentName(const std::string& componentName)
{
    std::vector<Entity> entities;

    auto it = aComponentArrays.find(componentName);
    if (it == aComponentArrays.end())
    {
        return entities; // doesnt exist
    }

    auto& componentArray = it->second;

    // You'll need to add a virtual method to BaseComponentArray
    // to get all entities
    return componentArray->GetAllEntities();
}
