//#include "Precompiled.h"
#include "EntityManager.h"

#include <iostream>
#include <cassert>

ECS::EntityManager::EntityManager()
{
    mActiveEntityCnt = 0;

    // fill the available entities to the container
    for (Entity e = 0; e < MAX_ENTITIES; e++)
    {
        aAvailableEntities.emplace(e);
    }
}

ECS::Entity ECS::EntityManager::CreateEntity()
{
    // out of range check
    assert(mActiveEntityCnt < MAX_ENTITIES && "ERROR : Too many active entities.");

    // pop the queue and set the new entity id
    Entity new_entity = aAvailableEntities.front();
    aAvailableEntities.pop();
    aEntityActive[new_entity] = true;
    ++mActiveEntityCnt;

    return new_entity;
}

void ECS::EntityManager::DestroyEntity(Entity entity)
{
    // check if id is valid and whether this entity is active
    assert(entity >= 0 && entity < MAX_ENTITIES && "ERROR : Entity id is Invalid.");
    assert(aEntityActive[entity] && "ERROR: Attempting to destroy an already inactive entity.");

    // reset the signature of the entity 
    aSignatures[entity].reset();

    // add the id back to the container for reuse
    aAvailableEntities.emplace(entity);
    aEntityActive[entity] = false;
    --mActiveEntityCnt;
}

void ECS::EntityManager::SetSignature(Entity entity, Signature& const signature)
{
    assert(entity >= 0 && entity < MAX_ENTITIES && "ERROR : Entity id is Invalid.");

    aSignatures[entity] = signature;
}

ECS::Signature ECS::EntityManager::GetSignature(Entity entity) const
{
    assert(entity >= 0 && entity < MAX_ENTITIES && "ERROR : Entity id is Invalid.");

    return aSignatures[entity];
}
