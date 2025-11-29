/*!
\file   EntityDuplicateCmd.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines the EntityDuplicateCmd class, an Editor command used to duplicate
an existing entity. This command integrates with the Undo/Redo system,
creates a deep copy of the source entity (including its components), and
renames the duplicated Transform component for clarity.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "Editor/Core/Command.h"
#include "ECS/Core/Types.hpp"
#include "ECS/Core/Coordinator.hpp"
#include "Editor/Core/EntitySnapshot.h"

namespace Uma_Editor
{
    /**
     * \class EntityDuplicateCmd
     * \brief Command responsible for duplicating an entity along with its components.
     *
     * This command duplicates an entity using Coordinator::DuplicateEntity(),
     * assigns the resulting entity a modified name (e.g., "object_dup"),
     * and supports undoing the duplication by destroying the created entity
     * and all of its children.
     */
    class EntityDuplicateCmd : public ICommand
    {
    private:
        Uma_ECS::Coordinator* coordinator;  //!< Pointer to ECS coordinator
        Uma_ECS::Entity srcEntity;          //!< ID of the entity being duplicated
        Uma_ECS::Entity createdEntity;      //!< ID of the duplicated entity
        std::string description;            //!< Human-readable description for UI

    public:
        /**
         * \brief Constructs the duplicate entity command.
         * \param coord Pointer to the Coordinator managing entity/component operations.
         * \param src The entity to duplicate.
         * \param desc Description string for the command.
         */
        EntityDuplicateCmd(
            Uma_ECS::Coordinator* coord,
            Uma_ECS::Entity src,
            std::string desc
        )
            : coordinator(coord)
            , srcEntity(src)
            , description(desc)
        {
        }

        /**
         * \brief Executes the duplication operation.
         * Creates a full entity clone, then renames its Transform::name
         * by appending "_dup" for identification.
         */
        void Execute() override
        {
            createdEntity = coordinator->DuplicateEntity(srcEntity);

            auto& tf = coordinator->GetComponent<Uma_ECS::Transform>(createdEntity);
            tf.name += "_dup";
        }

        /**
         * \brief Undoes the duplication by destroying the created entity.
         * Also destroys all children recursively to restore the exact previous state.
         */
        void Undo() override
        {
            coordinator->DestroyEntityAndChildren(createdEntity);
        }

        /**
         * \brief Retrieves the description of this command.
         * \return The string passed during construction.
         */
        std::string GetDescription() override
        {
            return description;
        }

        /**
         * \brief Returns the duplicated entity ID.
         * \return The newly created entity identifier.
         */
        Uma_ECS::Entity GetCreatedEntity()
        {
            return createdEntity;
        }
    };
}
