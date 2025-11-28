#pragma once

#include "Editor/Core/Command.h"
#include "ECS/Core/Coordinator.hpp"

namespace Uma_Editor
{
    // Templated class so it works for ANY component (RigidBody, Sprite, etc.)
    template <typename T>
    class EntityAddComponentCmd : public ICommand
    {
    private:
        Uma_ECS::Coordinator* pCoordinator;
        Uma_ECS::Entity mEntity;
        T mComponentData;       // The data to add (e.g., default values)
        std::string mDescription;
        bool mWasAdded;         // Track if we actually added it (safety for Undo)

    public:
        EntityAddComponentCmd(
            Uma_ECS::Coordinator* coordinator,
            Uma_ECS::Entity entity,
            const T& componentData,
            const std::string& desc = "Add Component"
        )
            : pCoordinator(coordinator)
            , mEntity(entity)
            , mComponentData(componentData)
            , mDescription(desc)
            , mWasAdded(false)
        {
        }

        void Execute() override
        {
            // Only add if the entity exists and DOES NOT have the component
            if (pCoordinator->HasActiveEntity(mEntity) && !pCoordinator->HasComponent<T>(mEntity))
            {
                pCoordinator->AddComponent<T>(mEntity, mComponentData);
                mWasAdded = true;
            }
            else
            {
                mWasAdded = false;
            }
        }

        void Undo() override
        {
            // Only remove if we were the ones who added it
            if (mWasAdded && pCoordinator->HasActiveEntity(mEntity) && pCoordinator->HasComponent<T>(mEntity))
            {
                pCoordinator->RemoveComponent<T>(mEntity);
            }
        }

        std::string GetDescription() override
        {
            return mDescription;
        }
    };
}