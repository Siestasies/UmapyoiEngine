/*!
\file   ComponentArray.hpp
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Implements a packed array storage container for components of a specific type using template-based design.

Provides O(1) component access through entity-to-index mapping with contiguous memory layout for cache efficiency.
Components are tightly packed by swapping removed elements with the last element to maintain density.
Includes serialization/deserialization support via RapidJSON and component cloning for entity duplication.
Base class (BaseComponentArray) enables polymorphic storage of different component types in a single container.
Supports up to MAX_ENTITIES components with entity existence checking and boundary validation.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "Types.hpp"
#include <array>
#include <unordered_map>
#include <cassert>
#include <string>
#include <algorithm>

#include "rapidjson/document.h"		// rapidjson's DOM-style API

namespace Uma_ECS
{
    /*!
    \brief Abstract base class for polymorphic component array storage.
    Enables the ComponentManager to store different typed ComponentArrays in a single container.
    */
    class BaseComponentArray
    {
    public:
        virtual ~BaseComponentArray() = default;

        /*!
        \brief Removes all component data associated with the given entity.
        \param entity The Entity ID to destroy data for.
        */
        virtual void DestroyEntity(Entity entity) = 0;

        /*!
        \brief Checks whether the given entity has a component in this array.
        \param entity The Entity ID to check.
        \return True if the entity has this component type.
        */
        virtual bool Has(Entity entity) const = 0;

        /*!
        \brief Copies the component data from a source entity to a destination entity.
        \param src The source Entity ID.
        \param dest The destination Entity ID.
        */
        virtual void CloneComponent(Entity src, Entity dest) = 0;

        /*!
        \brief Returns all entities that have a component in this array.
        \return A vector of Entity IDs.
        */
        virtual std::vector<Entity> GetAllEntities() const = 0;

        /*!
        \brief Creates a deep copy of this entire component array.
        \return A shared pointer to the cloned BaseComponentArray.
        */
        virtual std::shared_ptr<BaseComponentArray> CloneArray() const = 0;

        /*!
        \brief Returns whether this component type should be cloned during entity duplication.
        \return True if the component should be cloned (default), false to skip.
        */
        virtual bool ShouldClone() const { return true; }

        /*!
        \brief Serializes the component data for an entity into a RapidJSON value.
        \param entity The Entity ID to serialize.
        \param comps The RapidJSON value to write into.
        \param allocator The RapidJSON allocator.
        */
        virtual void Serialize(Entity entity, rapidjson::Value& comps, rapidjson::Document::AllocatorType& allocator) = 0;

        /*!
        \brief Deserializes component data for an entity from a RapidJSON value.
        \param entity The Entity ID to deserialize for.
        \param comps The RapidJSON value containing component data.
        \return The typeid name string of the deserialized component, or empty if not found.
        */
        virtual std::string Deserialize(Entity entity, const rapidjson::Value& comps) = 0;
    };

    template <typename T>
    class ComponentArray : public BaseComponentArray
    {
    public:

        /*!
        \brief Default constructor. Initializes entity-index mappings to invalid values.
        */
        ComponentArray()
        {
            aEntityToIndex.fill(MAX_ENTITIES);
            aIndexToEntity.fill(MAX_ENTITIES);
        }

        /*!
        \brief Adds a component to the packed array for the given entity.
        \param entity The Entity ID to add the component to.
        \param component The component data to store.
        \return EC_None on success, EC_ComponentAlreadyExists if the entity already has this component.
        */
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

        /*!
        \brief Removes the component for the given entity, swapping with the last element to maintain packing.
        \param entity The Entity ID to remove the component from.
        \return EC_None on success, EC_ComponentNotFound if the entity doesn't have this component.
        */
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

            // Clear the removed entity's mapping
            aEntityToIndex[entity] = MAX_ENTITIES;
            aIndexToEntity[last_index] = MAX_ENTITIES;  // Clear the now-unused slot

            --mSize;

            return ECSErrorCode::EC_None;
        }

        /*!
        \brief Retrieves a reference to the component data for the given entity.
        \param entity The Entity ID to get data for.
        \return A reference to the component.
        */
        T& GetData(Entity entity)
        {
            assert(Has(entity) && "ERROR : Entity doesnt contain this data.");

            size_t index = aEntityToIndex[entity];
            return aComponentArray[index];
        }

        /*!
        \brief Safe version of GetData that returns nullptr when the entity has no component.
        \param entity The Entity ID to look up.
        \return Pointer to the component, or nullptr if not found.
        */
        T* TryGetData(Entity entity)
        {
            if (!Has(entity)) return nullptr;

            size_t index = aEntityToIndex[entity];
            return &aComponentArray[index];
        }

        /*!
        \brief Removes the entity's component data if it exists. Called when an entity is destroyed.
        \param entity The Entity ID to clean up.
        */
        void DestroyEntity(Entity entity) override
        {
            if (Has(entity))
            {
                RemoveData(entity);
            }
        }

        /*!
        \brief Returns the number of components currently stored in the array.
        \return The component count.
        */
        size_t Size() const
        {
            return mSize;
        }

        /*!
        \brief Returns the Entity ID at the given packed array index.
        \param index The array index to look up.
        \return The Entity ID at that index.
        */
        Entity GetEntity(size_t index)
        {
            return aIndexToEntity[index];
        }

        /*!
        \brief Returns a reference to the component at the given packed array index.
        \param index The array index to access.
        \return A reference to the component data.
        */
        T& GetComponentAt(size_t index)
        {
            return aComponentArray[index];
        }

        /*!
        \brief Checks whether the given entity has a component in this array.
        \param entity The Entity ID to check.
        \return True if the entity has this component type.
        */
        bool Has(Entity entity) const override
        {
            if (entity >= MAX_ENTITIES) return false;

            size_t index = aEntityToIndex[entity];
            return (index < mSize && aIndexToEntity[index] == entity);
        }

        /*!
        \brief Copies the component from a source entity to a destination entity.
        \param src The source Entity ID.
        \param dest The destination Entity ID.
        */
        void CloneComponent(Entity src, Entity dest) override
        {
            assert(Has(src) && "Error : src entity doesn't contain this component type.");

            T component = GetData(src);
            AddData(dest, component);
        }

        /*!
        \brief Returns all entities that currently have this component type.
        \return A vector of Entity IDs.
        */
        std::vector<Entity> GetAllEntities() const override
        {
            std::vector<Entity> result;
            result.reserve(mSize);

            for (size_t i = 0; i < mSize; ++i)
            {
                result.push_back(aIndexToEntity[i]);
            }

            return result;
        }

        /*!
        \brief Creates a deep copy of this entire component array including all data and mappings.
        \return A shared pointer to the cloned ComponentArray.
        */
        std::shared_ptr<BaseComponentArray> CloneArray() const override
        {
            auto copy = std::make_shared<ComponentArray<T>>();
            copy->aComponentArray = this->aComponentArray;
            copy->aIndexToEntity = this->aIndexToEntity;
            copy->aEntityToIndex = this->aEntityToIndex;
            copy->mSize = this->mSize;
            return copy;
        }

        /*!
        \brief Serializes the component for the given entity into a RapidJSON value.
        \param entity The Entity ID to serialize.
        \param comps The RapidJSON value to write into.
        \param allocator The RapidJSON allocator.
        */
        void Serialize(Entity entity, rapidjson::Value& comps, rapidjson::Document::AllocatorType& allocator) override
        {
            if (!Has(entity)) return; // entity not exists

            T& component = aComponentArray[aEntityToIndex[entity]];
            rapidjson::Value componentObj;
            component.Serialize(componentObj, allocator);
            comps.AddMember(rapidjson::StringRef(typeid(T).name()), componentObj, allocator);
        }

        /*!
        \brief Deserializes the component for the given entity from a RapidJSON value.
        \param entity The Entity ID to deserialize for.
        \param comps The RapidJSON value containing component data.
        \return The typeid name string of the component if found, or empty string otherwise.
        */
        std::string Deserialize(Entity entity, const rapidjson::Value& comps) override
        {
            std::string compType = "";
            if (comps.HasMember(typeid(T).name())) 
            {
                T component;
                component.Deserialize(comps[typeid(T).name()]);

                if (Has(entity))
                {
                    // Update existing component
                    GetData(entity) = component;
                }
                else
                {
                    // Add new component
                    AddData(entity, component);
                }

                compType = typeid(T).name();
            }
            return compType;
        }

    private:

        // the container that stores all components of the same type of all entities
        std::array<T, MAX_ENTITIES> aComponentArray{};

        std::array<Entity, MAX_ENTITIES> aIndexToEntity{};
        std::array<size_t, MAX_ENTITIES> aEntityToIndex{};

        size_t mSize = 0; // how many components are currently in use
    };
}


