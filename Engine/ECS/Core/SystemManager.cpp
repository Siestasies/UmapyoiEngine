#include "SystemManager.hpp"
#include "System.hpp"

void Uma_ECS::SystemManager::EntityDestroyed(Entity entity)
{
    // remove the destroyed entity from all systems
    for (auto const& pair : aSystems)
    {
        // getting the system from the map
        auto const& system = pair.second;

        auto& entities = system->aEntities;
        for (size_t i = 0; i < entities.size(); ++i) 
        {
            if (entities[i] == entity) 
            {
                entities[i] = entities.back();  // overwrite with last
                entities.pop_back();            // shrink
                break;                          // only one instance per system anyway
            }
        }
    }
}

void Uma_ECS::SystemManager::EntitySignatureChanged(Entity entity, Signature entitySiganture)
{
    // Let each systems knows that the entity's signature had changed
    for (auto const& pair : aSystems)
    {
        auto const& type = pair.first;
        auto const& system = pair.second;
        auto const& systemSignature = aSignatures[type];

        // Check if the entity contains the components that are required for the system
        // basically using bitwise AND to compare both signatures
        // it will return the bit that both signature have
        // (entitySignature & systemSignature) == systemSignature
        if ((entitySiganture & systemSignature) == systemSignature)
        {
            auto& entities = system->aEntities;
            if (std::find(entities.begin(), entities.end(), entity) == entities.end()) // doesnt exist
            {
                entities.emplace_back(entity);
            }
        }
        else
        {
            auto& entities = system->aEntities;
            for (size_t i = 0; i < entities.size(); ++i)
            {
                if (entities[i] == entity)
                {
                    entities[i] = entities.back();  // overwrite with last
                    entities.pop_back();            // shrink
                    break;                          // only one instance per system anyway
                }
            }
        }
        
    }
}
