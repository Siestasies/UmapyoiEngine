/*!
\file   EntityAddComponentCmd.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines the EntityAddComponentCmd class, a templated command used by the Editor's
Undo/Redo system to handle adding components to entities.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

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
        bool mWasAdded;         // Track if actually added it

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

        /**
         * \brief Executes the add component operation.
         * * Checks if the entity exists and does NOT already have the component before adding.
         * Sets mWasAdded to true if successful.
         */
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

        /**
         * \brief Undoes the add operation by removing the component.
         * * Only removes the component if this specific command instance was responsible
         * for adding it (mWasAdded is true).
         */
        void Undo() override
        {
            if (mWasAdded && pCoordinator->HasActiveEntity(mEntity) && pCoordinator->HasComponent<T>(mEntity))
            {
                pCoordinator->RemoveComponent<T>(mEntity);
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