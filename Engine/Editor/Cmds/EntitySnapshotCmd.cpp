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

            // deserialize all components
            coordinator->DeserializeEntity(snapshot.entityID, comps);

            // Re-Init the Luascript 
            auto& luaScriptArray = coordinator->GetComponentArray<Uma_ECS::LuaScript>();
            if (luaScriptArray.Has(snapshot.entityID))
            {
                coordinator->GetEventSystem()->Emit<Uma_Engine::CallLuaToInitScript>(snapshot.entityID);
            }
        }
    }
}