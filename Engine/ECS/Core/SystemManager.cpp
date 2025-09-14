#include "SystemManager.h"

template<typename T>
std::shared_ptr<T> ECS::SystemManager::RegisterSystem()
{
    std::string type_name = std::string(typeid(T).name());

    assert(aSystems.find(type_name) == aSystems.end() && "Error : Registering the system more than once.");

    // Create a pointer to the system and return it, so that it can be used externally
    auto system = std::make_shared<T>();
    aSystems.insert({ type_name, system });
    return system;
}

template<typename T>
void ECS::SystemManager::SetSignature(Signature signature)
{
    std::string type_name = std::string(typeid(T).name());

    assert(aSystems.find(type_name) != aSystems && "Error : setting the signature of the system before registering.");

    aSignatures.insert({ type_name, signature });
}

void ECS::SystemManager::EntityDestroyed(Entity entity)
{
    // remove the destroyed entity from all systems
    for (auto const& pair : aSystems)
    {
        // getting the system from the map
        auto const& system = pair.second;

        system->aEntities.erase(entity);
    }
}

void ECS::SystemManager::EntitySignatureChanged(Entity entity, Signature entitySiganture)
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
