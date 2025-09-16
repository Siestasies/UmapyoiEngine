#pragma once

#include "Types.hpp"
#include <array>
#include <unordered_map>
#include <cassert>

namespace Uma_ECS
{
    class BaseComponentArray
    {
    public:
        virtual ~BaseComponentArray() = default;
        virtual void DestroyEntity(Entity entity) = 0; // detroy of entity shd be handled by the child
    };

    template <typename T>
    class ComponentArray : public BaseComponentArray
    {
    public:

        // Add / Remove Component from the array
        ECSErrorCode AddData(Entity entity, const T& component)
        {
#ifndef NDEBUG
            assert(aEntityToIndex.find(entity) == aEntityToIndex.end() && "ERROR : Same component is being added again.");
#else
            if (aEntityToIndex.find(entity) != aEntityToIndex.end())
            {
                return ECSErrorCode::EC_ComponentAlreadyExists;
            }
#endif
            size_t index = mSize;

            aIndexToEntity[index] = entity;
            aEntityToIndex[entity] = index;
            aComponentArray[index] = component;

            mSize++;

            return ECSErrorCode::EC_None;
        }

        ECSErrorCode RemoveData(Entity entity)
        {
#ifndef NDEBUG
            assert(aEntityToIndex.find(entity) != aEntityToIndex.end() && "ERROR : Component is alr removed.");
#else
            if (aEntityToIndex.find(entity) == aEntityToIndex.end())
            {
                return ECSErrorCode::EC_ComponentNotFound;
            }
#endif

            // when we removing component of 1 entity
            // we move the last component into the spot 
            // that was occupied by the component is getting to be removed

            // copy the last component to the index where its gg tobe removed
            size_t index_of_removed_entity = aEntityToIndex[entity];
            size_t index_of_last_entity = mSize - 1;
            aComponentArray[index_of_removed_entity] = aComponentArray[index_of_last_entity];

            // reassign the map so that 
            Entity entity_of_last_component = aIndexToEntity[index_of_last_entity];
            aEntityToIndex[entity_of_last_component] = index_of_removed_entity;
            aIndexToEntity[index_of_removed_entity] = entity_of_last_component;

            aEntityToIndex.erase(entity);
            aIndexToEntity.erase(index_of_last_entity);

            return ECSErrorCode::EC_None;
        }

        T& GetData(Entity entity)
        {
#ifndef NDEBUG
            assert(aEntityToIndex.find(entity) != aEntityToIndex.end() && "ERROR : Entity doesnt contain this data.");
#else
            if (it == aEntityToIndex.end())
            {
                return nullptr;
            }
#endif

            size_t index = aEntityToIndex[entity];

            return aComponentArray[index];
        }

        // Destroy of entity
        void DestroyEntity(Entity entity) override
        {
            if (aEntityToIndex.find(entity) != aEntityToIndex.end())
            {
                RemoveData(entity);
            }
        }

    private:

        // the container that stores all components of the same type of all entities
        std::array<T, MAX_ENTITIES> aComponentArray{};

        std::unordered_map<size_t, Entity> aIndexToEntity;
        std::unordered_map<Entity, size_t> aEntityToIndex;

        size_t mSize = 0; // how many components are currently in use
    };

}


