/*!
\file   EntityRemoveComponentCmd.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Javier Chua Dong Qing (100%)
\par    E-mail: javierdongqing.chua@digipen.edu
\par    DigiPen login: javierdongqing.chua

\brief
Defines the EntityRemoveComponentCmd class, a templated command used by the Editor's
Undo/Redo system to handle removing components from entities.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

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
        bool mWasRemoved;       // Track if actually removed it

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

        /**
         * \brief Executes the remove component operation.
         * * Removes the component from the entity if it exists.
         * Sets mWasRemoved to true if successful.
         */
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

        /**
         * \brief Undoes the remove operation by restoring the component.
         * * Re-adds the component using the backed-up data (mBackupData) if it was
         * successfully removed by this command instance.
         */
        void Undo() override
        {
            // If we successfully removed it, add it back with the backup data
            if (mWasRemoved && pCoordinator->HasActiveEntity(mEntity) && !pCoordinator->HasComponent<T>(mEntity))
            {
                pCoordinator->AddComponent<T>(mEntity, mBackupData);
            }
        }

        /**
         * \brief Gets the description of the command.
         * \return The string description passed in the constructor.
         */
        std::string GetDescription() override
        {
            return mDescription;
        }
    };
}