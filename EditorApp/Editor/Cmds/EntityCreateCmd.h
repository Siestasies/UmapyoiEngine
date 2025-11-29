/*!
\file   EntityCreateCmd.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines the EntityCreateCmd class, a command used by the Editor's Undo/Redo
system to handle entity creation. The command supports optionally assigning
a parent entity and automatically initializes a default Transform component.

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
     * \class EntityCreateCmd
     * \brief Command that creates a new entity in the ECS system.
     *
     * This command is used by the Editor’s Undo/Redo system to create entities.
     * A default Transform component is automatically added, and the entity
     * can optionally be assigned a parent.
     *
     * Undo removes the created entity entirely.
     */
    class EntityCreateCmd : public ICommand
    {
    private:
        Uma_ECS::Coordinator* coordinator;           ///< ECS Coordinator pointer.
        Uma_ECS::Entity entityID;                    ///< The entity created during Execute().
        std::optional<Uma_ECS::Entity> parentID;     ///< Optional parent entity (if any).
        std::string description;                     ///< Human-readable command description.

    public:
        /**
         * \brief Constructor for the entity creation command.
         *
         * \param coord Pointer to the ECS Coordinator.
         * \param parent Optional parent entity ID.
         * \param desc Optional string description for UI purposes.
         */
        EntityCreateCmd(
            Uma_ECS::Coordinator* coord,
            std::optional<Uma_ECS::Entity> parent,
            std::string desc = "Create Entity"
        )
            : coordinator(coord)
            , parentID(parent)
            , description(desc)
        {
        }

        /**
         * \brief Executes the entity creation.
         *
         * - Creates a new entity.
         * - Adds a default Transform component.
         * - Assigns a parent if one was provided.
         */
        void Execute() override
        {
            // Create the new entity.
            entityID = coordinator->CreateEntity();

            // Add a default Transform component.
            coordinator->AddComponent(entityID,
                Uma_ECS::Transform{
                    .name = std::string("new entity"),
                    .position = Vec2(0.f, 0.f),
                    .rotation = Vec2(0.f, 0.f),
                    .scale = Vec2(1, 1),
                });

            // Assign parent if specified.
            if (parentID != std::nullopt)
            {
                coordinator->SetParent(entityID, parentID.value());
            }
        }

        /**
         * \brief Undo the creation by destroying the entity.
         */
        void Undo() override
        {
            coordinator->DestroyEntity(entityID);
        }

        /**
         * \brief Gets the command description.
         * \return A string representing what the command does.
         */
        std::string GetDescription() override
        {
            return description;
        }

        /**
         * \brief Retrieves the ID of the entity that was created.
         * \return Entity ID created in Execute().
         */
        Uma_ECS::Entity GetCreatedEntity()
        {
            return entityID;
        }
    };
}
