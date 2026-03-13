/*!
\file   EntityReorderCmd.h
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines the EntityReorderCmd class, a command used by the Editor's Undo/Redo
system to handle entity reordering within the hierarchy. Supports reordering
both root-level entities and children within a parent.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "Editor/Core/Command.h"
#include "ECS/Core/Types.hpp"
#include "ECS/Core/Coordinator.hpp"

namespace Uma_Editor
{
    /**
     * \class EntityReorderCmd
     * \brief Command that moves an entity to a different position in the hierarchy.
     *
     * Handles both root-level reordering (MoveEntityInHierarchy) and
     * child reordering within a parent (MoveChildInParent).
     */
    class EntityReorderCmd : public ICommand
    {
    private:
        Uma_ECS::Coordinator* coordinator;
        Uma_ECS::Entity entityID;
        std::optional<Uma_ECS::Entity> parentEntity;  // nullopt = root level
        int oldIndex;
        int newIndex;
        std::string description;

    public:
        /**
         * \brief Constructs a reorder command.
         * \param coord Pointer to the ECS Coordinator.
         * \param entity The entity to reorder.
         * \param parent The parent entity (nullopt for root-level).
         * \param fromIndex The original index.
         * \param toIndex The target index.
         * \param desc Description for UI/history.
         */
        EntityReorderCmd(
            Uma_ECS::Coordinator* coord,
            Uma_ECS::Entity entity,
            std::optional<Uma_ECS::Entity> parent,
            int fromIndex,
            int toIndex,
            std::string desc = "Reorder Entity"
        )
            : coordinator(coord)
            , entityID(entity)
            , parentEntity(parent)
            , oldIndex(fromIndex)
            , newIndex(toIndex)
            , description(std::move(desc))
        {
        }

        void Execute() override
        {
            if (parentEntity.has_value())
            {
                coordinator->MoveChildInParent(entityID, newIndex);
            }
            else
            {
                coordinator->MoveEntityInHierarchy(entityID, newIndex);
            }
        }

        void Undo() override
        {
            if (parentEntity.has_value())
            {
                coordinator->MoveChildInParent(entityID, oldIndex);
            }
            else
            {
                coordinator->MoveEntityInHierarchy(entityID, oldIndex);
            }
        }

        std::string GetDescription() override
        {
            return description;
        }
    };
}
