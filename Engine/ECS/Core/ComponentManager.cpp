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
