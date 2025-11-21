#pragma once 

#include "Editor/Core/Command.h"

#include "ECS/Core/Types.hpp"
#include "ECS/Core/Coordinator.hpp"
#include "Editor/Core/EntitySnapshot.h"

namespace Uma_Editor
{
    class EntitySnapshotCmd : public ICommand
    {
    private:

        Uma_ECS::Coordinator* coordinator;
        EntitySnapshot beforeState;
        EntitySnapshot afterState;
        std::string description;

        void RestoreSnapshot(EntitySnapshot&& snapshot);

    public:

        EntitySnapshotCmd(
            Uma_ECS::Coordinator* coord,
            EntitySnapshot&& before,
            EntitySnapshot&& after,
            const std::string& desc = "Entity Snapshot"
        );

        void Execute() override;

        void Undo() override;

        std::string GetDescription() override;
    };
}