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
