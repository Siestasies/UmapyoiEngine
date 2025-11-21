#include "EntitySnapshotCmd.h"

// for calling luascript system to reinit the scripting environment
#include "Events/IMGUIEvents.h"

namespace Uma_Editor
{
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

    void EntitySnapshotCmd::Execute()
    {
        RestoreSnapshot(std::move(afterState));
    }

    void EntitySnapshotCmd::Undo()
    {
        RestoreSnapshot(std::move(beforeState));
    }

    std::string EntitySnapshotCmd::GetDescription()
    {
        return description;
    }

    void EntitySnapshotCmd::RestoreSnapshot(EntitySnapshot&& snapshot)
    {
        if (!coordinator->HasActiveEntity(snapshot.entityID))
            return;

        if (snapshot.componentData.HasMember("components"))
        {
            const auto& comps = snapshot.componentData["components"];

            // deserialize all components (this will CLEAR children in Transform)
            coordinator->DeserializeEntity(snapshot.entityID, comps);

            // *** FIX: Restore children vector from snapshot ***
            auto& tfArray = coordinator->GetComponentArray<Uma_ECS::Transform>();
            if (tfArray.Has(snapshot.entityID))
            {
                auto& tf = tfArray.GetData(snapshot.entityID);

                // Restore the children vector that was cleared during deserialization
                tf.children = snapshot.childrenIDs;

                // Verify parent relationship is still valid
                if (snapshot.parentID.has_value())
                {
                    if (coordinator->HasActiveEntity(snapshot.parentID.value()))
                    {
                        tf.parent = snapshot.parentID;
                    }
                    else
                    {
                        tf.parent = std::nullopt; // Parent no longer exists
                    }
                }
                else
                {
                    tf.parent = std::nullopt;
                }

                // Ensure all children still exist and have this entity as parent
                std::vector<Uma_ECS::Entity> validChildren;
                for (Uma_ECS::Entity child : tf.children)
                {
                    if (coordinator->HasActiveEntity(child) && tfArray.Has(child))
                    {
                        auto& childTf = tfArray.GetData(child);

                        // Make sure child's parent pointer is correct
                        if (!childTf.parent.has_value() || childTf.parent.value() != snapshot.entityID)
                        {
                            childTf.parent = snapshot.entityID;
                        }

                        validChildren.push_back(child);
                    }
                }
                tf.children = validChildren;

                tf.isDirty = true;
            }

            // Re-Init the Luascript 
            auto& luaScriptArray = coordinator->GetComponentArray<Uma_ECS::LuaScript>();
            if (luaScriptArray.Has(snapshot.entityID))
            {
                coordinator->GetEventSystem()->Emit<Uma_Engine::CallLuaToInitScript>(snapshot.entityID);
            }
        }
    }
}