/*!
\file   EntitySnapshotCmd.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Implements the EntitySnapshotCmd class, which restores an entity's full
state before or after an edit operation. This command is used by the
Undo/Redo system and supports restoration of components, transform
hierarchy, parent–child relationships, and script reinitialization.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "EntitySnapshotCmd.h"

// For calling the Lua scripting system to reinitialise script state
#include "Events/IMGUIEvents.h"

namespace Uma_Editor
{
    /**
     * \brief Constructs a snapshot command storing both pre-edit and post-edit states.
     * \param coord Pointer to the ECS Coordinator.
     * \param before The snapshot representing the entity before modification.
     * \param after The snapshot representing the entity after modification.
     * \param desc Description for the command history.
     */
    EntitySnapshotCmd::EntitySnapshotCmd(
        Uma_ECS::Coordinator* coord,
        EntitySnapshot&& before,
        EntitySnapshot&& after,
        const std::string& desc
    )
        : coordinator(coord)
        , beforeState(std::move(before))
        , afterState(std::move(after))
        , description(desc)
    {
    }

    /**
     * \brief Restores the entity to the "after" state.
     */
    void EntitySnapshotCmd::Execute()
    {
        RestoreSnapshot(std::move(afterState));
    }

    /**
     * \brief Restores the entity to the "before" state.
     */
    void EntitySnapshotCmd::Undo()
    {
        RestoreSnapshot(std::move(beforeState));
    }

    /**
     * \brief Retrieves the command description.
     */
    std::string EntitySnapshotCmd::GetDescription()
    {
        return description;
    }

    /**
     * \brief Internal helper used to restore full entity data.
     *
     * This method:
     * - Deserialises all entity components.
     * - Restores Transform hierarchy (parent/children).
     * - Validates existence of parent and children entities.
     * - Ensures parent pointers of children are updated correctly.
     * - Marks Transform as dirty so systems can update it.
     * - Reinitialises Lua scripts when present.
     *
     * \param snapshot An rvalue snapshot containing the stored state.
     */
    void EntitySnapshotCmd::RestoreSnapshot(EntitySnapshot&& snapshot)
    {
        if (!coordinator->HasActiveEntity(snapshot.entityID))
            return;

        if (snapshot.componentData.HasMember("components"))
        {
            const auto& comps = snapshot.componentData["components"];

            // Deserialize all components (this will clear children in Transform)
            coordinator->DeserializeEntity(snapshot.entityID, comps);

            // Restore Transform hierarchy
            auto& tfArray = coordinator->GetComponentArray<Uma_ECS::Transform>();
            if (tfArray.Has(snapshot.entityID))
            {
                auto& tf = tfArray.GetData(snapshot.entityID);

                // Restore the children vector that was cleared during deserialization
                tf.children = snapshot.childrenIDs;

                // Restore parent if valid
                if (snapshot.parentID.has_value())
                {
                    if (coordinator->HasActiveEntity(snapshot.parentID.value()))
                    {
                        tf.parent = snapshot.parentID;
                    }
                    else
                    {
                        tf.parent = std::nullopt; // parent no longer exists
                    }
                }
                else
                {
                    tf.parent = std::nullopt;
                }

                // Validate and correct children relationships
                std::vector<Uma_ECS::Entity> validChildren;
                for (Uma_ECS::Entity child : tf.children)
                {
                    if (coordinator->HasActiveEntity(child) && tfArray.Has(child))
                    {
                        auto& childTf = tfArray.GetData(child);

                        // Ensure child references this as the parent
                        if (!childTf.parent.has_value() || childTf.parent.value() != snapshot.entityID)
                        {
                            childTf.parent = snapshot.entityID;
                        }

                        validChildren.push_back(child);
                    }
                }
                tf.children = validChildren;

                // Mark dirty so dependent systems update transforms
                tf.isDirty = true;
            }

            // Re-initialise Lua script if present
            auto& luaScriptArray = coordinator->GetComponentArray<Uma_ECS::LuaScript>();
            if (luaScriptArray.Has(snapshot.entityID))
            {
                coordinator->GetEventSystem()->Emit<Uma_Engine::CallLuaToInitScript>(snapshot.entityID);
            }
        }
    }
}
