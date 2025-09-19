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

        virtual bool Has(Entity entity) const = 0;
        virtual void CloneComponent(Entity src, Entity dest) = 0;
    };

    template <typename T>
    class ComponentArray : public BaseComponentArray
    {
    public:

        // Add / Remove Component from the array
        ECSErrorCode AddData(Entity entity, const T& component)
        {
#ifndef NDEBUG
            assert(!Has(entity) && "Error : Same component is being added again.");
#else
            if (Has(entity))
            {
                return ECSErrorCode::EC_ComponentAlreadyExists;
            }
#endif

            size_t index = mSize;
            aEntityToIndex[entity] = index;
            aIndexToEntity[index] = entity;
            aComponentArray[index] = component;
            ++mSize;

            return ECSErrorCode::EC_None;
        }

        ECSErrorCode RemoveData(Entity entity)
        {
#ifndef NDEBUG
            assert(Has(entity) && "Error : This entity doesn't contain this component.");
#else
            if (!Has(entity))
            {
                return ECSErrorCode::EC_ComponentNotFound;
            }
#endif
            size_t index_to_remove = aEntityToIndex[entity];
            size_t last_index = mSize - 1;

            // move last to the remove index
            aComponentArray[index_to_remove] = aComponentArray[last_index];
            // find last entity
            Entity last_entity = aIndexToEntity[last_index];
            // swap their locations
            aEntityToIndex[last_entity] = index_to_remove;
            aIndexToEntity[index_to_remove] = last_entity;

            --mSize;

            return ECSErrorCode::EC_None;
        }

        T& GetData(Entity entity)
        {
            assert(Has(entity) && "ERROR : Entity doesnt contain this data.");

            size_t index = aEntityToIndex[entity];
            return aComponentArray[index];
        }

        // Destroy of entity
        void DestroyEntity(Entity entity) override
        {
            if (Has(entity))
            {
                RemoveData(entity);
            }
        }

        // helper functions that directly access the array
        // This is for optimisation
        size_t Size() const
        {
            return mSize;
        }

        Entity GetEntity(size_t index) 
        {
            return aIndexToEntity[index];
        }

        T& GetComponentAt(size_t index)
        {
            return aComponentArray[index];
        }

        bool Has(Entity entity) const override
        {
            if (entity >= MAX_ENTITIES) return false;

            size_t index = aEntityToIndex[entity];
            return (index < mSize && aIndexToEntity[index] == entity);
        }

        void CloneComponent(Entity src, Entity dest) override
        {
            assert(Has(src) && "Error : src entity doesn't contain this component type.");

            T component = GetData(src);
            AddData(dest, component);
        }

    private:

        // the container that stores all components of the same type of all entities
        std::array<T, MAX_ENTITIES> aComponentArray{};

        std::array<Entity, MAX_ENTITIES> aIndexToEntity;
        std::array<size_t, MAX_ENTITIES> aEntityToIndex;

        size_t mSize = 0; // how many components are currently in use
    };

}


