/*!
\file   EntitySetActiveCmd.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines the EntitySetActiveCmd class, an Editor command used to toggle an
entity's active state. The command stores the previous state for Undo/Redo
operations and updates the entity's active flag using the ECS Coordinator.

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
     * \class EntitySetActiveCmd
     * \brief Command that sets or toggles an entity's active state.
     *
     * This command is part of the Editor Undo/Redo command stack and allows
     * enabling or disabling an entity. The previous state is captured during
     * construction to ensure the change can be reverted correctly.
     */
    class EntitySetActiveCmd : public ICommand
    {
    private:
        Uma_ECS::Coordinator* coordinator;   //!< Pointer to the ECS coordinator
        Uma_ECS::Entity entityID;            //!< Target entity
        bool newActiveState;                 //!< State to apply on Execute()
        bool previousActiveState;            //!< State used when Undo() is called
        std::string description;             //!< Description for UI/history logs

    public:
        /**
         * \brief Constructs the command and stores the previous active state.
         * \param coord Coordinator handling ECS operations.
         * \param entity The entity whose active flag will be modified.
         * \param activeState Desired active state to apply.
         * \param desc Optional description for the command.
         */
        EntitySetActiveCmd(
            Uma_ECS::Coordinator* coord,
            Uma_ECS::Entity entity,
            bool activeState,
            std::string desc = "Toggle Entity Active"
        )
            : coordinator(coord)
            , entityID(entity)
            , newActiveState(activeState)
            , description(desc)
        {
            // Store original state for undo
            previousActiveState = coordinator->IsActiveSelf(entityID);
        }

        /**
         * \brief Applies the new active state to the entity.
         */
        void Execute() override
        {
            coordinator->SetActive(entityID, newActiveState);
        }

        /**
         * \brief Restores the previous active state.
         */
        void Undo() override
        {
            coordinator->SetActive(entityID, previousActiveState);
        }

        /**
         * \brief Provides a descriptive name for UI/history display.
         * \return The command description.
         */
        std::string GetDescription() override
        {
            return description;
        }
    };
}
