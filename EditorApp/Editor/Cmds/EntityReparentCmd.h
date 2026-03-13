/*!
\file   EntityReparentCmd.h
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines the EntityReparentCmd class, a command used by the Editor's Undo/Redo
system to handle entity reparenting. Supports setting a new parent, removing
a parent, and restoring the entity's previous hierarchy position on undo.

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
     * \class EntityReparentCmd
     * \brief Command that changes an entity's parent in the hierarchy.
     *
     * Supports both SetParent (reparenting to a new parent) and RemoveParent
     * (detaching from current parent). The previous parent and hierarchy index
     * are stored so the operation can be fully reversed on Undo.
     */
    class EntityReparentCmd : public ICommand
    {
    private:
        Uma_ECS::Coordinator* coordinator;
        Uma_ECS::Entity entityID;
        std::optional<Uma_ECS::Entity> oldParent;
        std::optional<Uma_ECS::Entity> newParent;
        int oldChildIndex;      // Index within old parent's children list (or root hierarchy index)
        std::string description;

    public:
        /**
         * \brief Constructs a reparent command, capturing the current parent and index.
         * \param coord Pointer to the ECS Coordinator.
         * \param entity The entity to reparent.
         * \param targetParent The new parent (nullopt to remove parent).
         * \param desc Description for UI/history.
         */
        EntityReparentCmd(
            Uma_ECS::Coordinator* coord,
            Uma_ECS::Entity entity,
            std::optional<Uma_ECS::Entity> targetParent,
            std::string desc = "Reparent Entity"
        )
            : coordinator(coord)
            , entityID(entity)
            , newParent(targetParent)
            , description(std::move(desc))
        {
            // Capture current parent
            oldParent = coordinator->GetParent(entity);

            // Capture current index for undo restoration
            if (oldParent.has_value())
            {
                auto& transformArray = coordinator->GetComponentArray<Uma_ECS::Transform>();
                auto& parentTransform = transformArray.GetData(oldParent.value());
                oldChildIndex = -1;
                for (int i = 0; i < static_cast<int>(parentTransform.children.size()); i++)
                {
                    if (parentTransform.children[i] == entity)
                    {
                        oldChildIndex = i;
                        break;
                    }
                }
            }
            else
            {
                oldChildIndex = coordinator->GetHierarchyIndex(entity);
            }
        }

        void Execute() override
        {
            if (newParent.has_value())
            {
                coordinator->SetParent(entityID, newParent.value());
            }
            else
            {
                coordinator->RemoveParent(entityID);
            }
        }

        void Undo() override
        {
            if (oldParent.has_value())
            {
                coordinator->SetParent(entityID, oldParent.value());
                // Restore original position within parent's children
                if (oldChildIndex >= 0)
                {
                    coordinator->MoveChildInParent(entityID, oldChildIndex);
                }
            }
            else
            {
                coordinator->RemoveParent(entityID);
                // Restore original position in root hierarchy
                if (oldChildIndex >= 0)
                {
                    coordinator->MoveEntityInHierarchy(entityID, oldChildIndex);
                }
            }
        }

        std::string GetDescription() override
        {
            return description;
        }
    };
}
