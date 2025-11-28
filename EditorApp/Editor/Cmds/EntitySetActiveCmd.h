#pragma once

#include "Editor/Core/Command.h"

#include "ECS/Core/Types.hpp"
#include "ECS/Core/Coordinator.hpp"

namespace Uma_Editor
{
    class EntitySetActiveCmd : public ICommand
    {
    private:
        Uma_ECS::Coordinator* coordinator;
        Uma_ECS::Entity entityID;
        bool newActiveState;
        bool previousActiveState;
        std::string description;

    public:
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
            // Store the previous state for undo
            previousActiveState = coordinator->IsActiveSelf(entityID);
        }

        void Execute() override
        {
            coordinator->SetActive(entityID, newActiveState);
        }

        void Undo() override
        {
            coordinator->SetActive(entityID, previousActiveState);
        }

        std::string GetDescription() override
        {
            return description;
        }
    };
}
