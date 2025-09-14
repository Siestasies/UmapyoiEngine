#pragma once

#include "Types.h"
#include <array>
#include <unordered_map>

namespace ECS
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
        ECSErrorCode AddData(Entity entity, const T& component);
        ECSErrorCode RemoveData(Entity entity);

        T& GetData(Entity entity);

        // Destroy of entity
        void DestroyEntity(Entity entity) override;

    private:

        // the container that stores all components of the same type of all entities
        std::array<T, MAX_ENTITIES> aComponentArray{};

        std::unordered_map<size_t, Entity> aIndexToEntity;
        std::unordered_map<Entity, size_t> aEntityToIndex;

        size_t mSize = 0; // how many components are currently in use
    };

}


