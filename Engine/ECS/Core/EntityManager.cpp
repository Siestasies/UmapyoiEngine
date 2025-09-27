//#include "Precompiled.h"
#include "EntityManager.hpp"

#include <iostream>
#include <cassert>

Uma_ECS::EntityManager::EntityManager()
{
    mActiveEntityCnt = 0;

    // fill the available entities to the container
    for (Entity e = 0; e < MAX_ENTITIES; e++)
    {
        aAvailableEntities.emplace(e);
    }
}

Uma_ECS::Entity Uma_ECS::EntityManager::CreateEntity()
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

void Uma_ECS::EntityManager::DestroyEntity(Entity entity)
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

void Uma_ECS::EntityManager::SetSignature(Entity entity, Signature& const signature)
{
    assert(entity >= 0 && entity < MAX_ENTITIES && "ERROR : Entity id is Invalid.");

    aSignatures[entity] = signature;
}

Uma_ECS::Signature Uma_ECS::EntityManager::GetSignature(Entity entity) const
{
    assert(entity >= 0 && entity < MAX_ENTITIES && "ERROR : Entity id is Invalid.");

    return aSignatures[entity];
}

int Uma_ECS::EntityManager::GetEntityCount() const
{
    return static_cast<int>(mActiveEntityCnt);
}

std::vector<Uma_ECS::Entity> Uma_ECS::EntityManager::GetAllEntites() const
{
    std::vector<Entity> allEntities;

    allEntities.reserve(mActiveEntityCnt);

    for (Entity entity = 0; entity < MAX_ENTITIES; ++entity)
    {
        if (aEntityActive[entity])
        {
            allEntities.push_back(entity);
        }
    }
    return allEntities;
}

void Uma_ECS::EntityManager::DestroyAllEntities()
{
    //std::vector<Entity> entityList = GetAllEntites();
    //for (auto const& entity : entityList)
    //{
    //    DestroyEntity(entity);
    //}
}
