/*!
\file   EntitySnapshotCmd.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Declares the EntitySnapshotCmd class, a powerful command used by the
Editor Undo/Redo system to restore complete serialized entity states.
This command stores both a "before" and "after" snapshot of an entity,
and can fully reconstruct its components, hierarchy, and script state.

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
     * \class EntitySnapshotCmd
     * \brief Command that restores an entity from serialized snapshots.
     *
     * This command stores two EntitySnapshot objects:
     * - beforeState — the entity state before modification
     * - afterState  — the entity state after modification
     *
     * Executing the command applies the "after" snapshot, and undoing the command
     * restores the "before" snapshot. This is used for complex entity editing,
     * including component modifications, hierarchy changes, and script reloads.
     */
    class EntitySnapshotCmd : public ICommand
    {
    public:

        /**
         * \brief Constructs a snapshot command with before/after states.
         * \param coord Pointer to the ECS Coordinator.
         * \param before The entity state before modifications (rvalue reference).
         * \param after The entity state after modifications (rvalue reference).
         * \param desc Optional command label for UI usage.
         */
        EntitySnapshotCmd(
            Uma_ECS::Coordinator* coord,
            EntitySnapshot&& before,
            EntitySnapshot&& after,
            const std::string& desc = "Entity Snapshot"
        );

        /**
         * \brief Applies the "after" snapshot to the entity.
         */
        void Execute() override;

        /**
         * \brief Restores the "before" snapshot to the entity.
         */
        void Undo() override;

        /**
         * \brief Returns a descriptive name for editor command logs.
         */
        std::string GetDescription() override;

    private:

        /**
         * \brief Restores the entity state from the given snapshot.
         * \param snapshot A snapshot containing component data, parent/child info,
         *        and script state. Passed as rvalue because snapshots are consumed.
         */
        void RestoreSnapshot(EntitySnapshot&& snapshot);

        Uma_ECS::Coordinator* coordinator;  //!< ECS Coordinator
        EntitySnapshot beforeState;         //!< State before change
        EntitySnapshot afterState;          //!< State after change
        std::string description;            //!< Command label
    };
}
