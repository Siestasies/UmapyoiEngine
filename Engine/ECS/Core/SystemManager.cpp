#include "SystemManager.hpp"
#include "System.hpp"

void Uma_ECS::SystemManager::EntityDestroyed(Entity entity)
{
    // remove the destroyed entity from all systems
    for (auto const& pair : aSystems)
    {
        // getting the system from the map
        auto const& system = pair.second;

        system->aEntities.erase(entity);
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
            system->aEntities.insert(entity);
        }
        else
        {
            system->aEntities.erase(entity);
        }
        
    }
}
