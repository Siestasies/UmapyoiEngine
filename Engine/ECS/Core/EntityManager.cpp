/*!
\file   EntityManager.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Implements entity lifecycle management using a free-list queue pattern for ID reuse.

Creates entities by popping from available IDs queue, destroys entities by resetting signatures and returning IDs to the pool.
Tracks active entity count and maintains per-entity signatures and active status flags.
Provides entity enumeration for serialization and batch operations with bounds checking and assertion-based validation.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

//#include "Precompiled.h"
#include "EntityManager.hpp"

#include <iostream>
#include <cassert>
#include <algorithm>

Uma_ECS::EntityManager::EntityManager()
{
    mActiveEntityCnt = 0;

    // fill the available entities to the container
    for (Entity e = 0; e < MAX_ENTITIES; e++)
    {
        aAvailableEntities.emplace(e);
    }
}

Uma_ECS::EntityManager::EntityManager(const EntityManager& other) noexcept
    : aAvailableEntities(other.aAvailableEntities)
    , aEntityActive(other.aEntityActive)
    , aEntityEnabled(other.aEntityEnabled)
    , aSignatures(other.aSignatures)
    , mActiveEntityCnt(other.mActiveEntityCnt)
{

}

Uma_ECS::Entity Uma_ECS::EntityManager::CreateEntity()
{
    // out of range check
    assert(mActiveEntityCnt < MAX_ENTITIES && "ERROR : Too many active entities.");

    if (mActiveEntityCnt >= MAX_ENTITIES) return 0;

    // pop the queue and set the new entity id
    Entity new_entity = aAvailableEntities.front();
    aAvailableEntities.pop();
    aEntityActive[new_entity] = true;
    aEntityEnabled[new_entity] = true;  // New entities are enabled by default
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

    // Create a new queue with this entity at the front ***
    std::queue<Entity> newQueue;
    newQueue.push(entity);  // Push destroyed entity to front

    // Then add all existing entities back
    while (!aAvailableEntities.empty())
    {
        newQueue.push(aAvailableEntities.front());
        aAvailableEntities.pop();
    }

    aAvailableEntities = newQueue;

    aEntityActive[entity] = false;
    aEntityEnabled[entity] = false;  // Reset enabled state
    --mActiveEntityCnt;
}

bool Uma_ECS::EntityManager::HasActiveEntity(Entity entity) const
{
    return aEntityActive[entity];
}

void Uma_ECS::EntityManager::SetSignature(Entity entity, const Signature& signature)
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

void Uma_ECS::EntityManager::SetEntityEnabled(Entity entity, bool enabled)
{
    assert(entity >= 0 && entity < MAX_ENTITIES && "ERROR : Entity id is Invalid.");
    assert(aEntityActive[entity] && "ERROR: Attempting to set enabled state on inactive entity.");

    aEntityEnabled[entity] = enabled;
}

bool Uma_ECS::EntityManager::IsEntityEnabled(Entity entity) const
{
    assert(entity >= 0 && entity < MAX_ENTITIES && "ERROR : Entity id is Invalid.");

    return aEntityEnabled[entity];
}

void Uma_ECS::EntityManager::DestroyAllEntities()
{
    //std::vector<Entity> entityList = GetAllEntites();
    //for (auto const& entity : entityList)
    //{
    //    DestroyEntity(entity);
    //}
}