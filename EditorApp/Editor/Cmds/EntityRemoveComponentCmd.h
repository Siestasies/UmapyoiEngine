#pragma once

#include "Editor/Core/Command.h"
#include "ECS/Core/Coordinator.hpp"

namespace Uma_Editor
{
    template <typename T>
    class EntityRemoveComponentCmd : public ICommand
    {
    private:
        Uma_ECS::Coordinator* pCoordinator;
        Uma_ECS::Entity mEntity;
        T mBackupData;          // Stores the data BEFORE removal
        std::string mDescription;
        bool mWasRemoved;       // Track if we actually removed it

    public:
        EntityRemoveComponentCmd(
            Uma_ECS::Coordinator* coordinator,
            Uma_ECS::Entity entity,
            const std::string& desc = "Remove Component"
        )
            : pCoordinator(coordinator)
            , mEntity(entity)
            , mDescription(desc)
            , mWasRemoved(false)
        {
            // Capture the data IMMEDIATELY in constructor
            if (pCoordinator->HasActiveEntity(mEntity) && pCoordinator->HasComponent<T>(mEntity))
            {
                mBackupData = pCoordinator->GetComponent<T>(mEntity);
            }
        }

        void Execute() override
        {
            if (pCoordinator->HasActiveEntity(mEntity) && pCoordinator->HasComponent<T>(mEntity))
            {
                pCoordinator->RemoveComponent<T>(mEntity);
                mWasRemoved = true;
            }
            else
            {
                mWasRemoved = false;
            }
        }

        void Undo() override
        {
            // If we successfully removed it, add it back with the backup data
            if (mWasRemoved && pCoordinator->HasActiveEntity(mEntity) && !pCoordinator->HasComponent<T>(mEntity))
            {
                pCoordinator->AddComponent<T>(mEntity, mBackupData);
            }
        }

        std::string GetDescription() override
        {
            return mDescription;
        }
    };
}