#pragma once

#include "Editor/Core/Command.h"

#include "ECS/Core/Types.hpp"
#include "ECS/Core/Coordinator.hpp"
#include "Editor/Core/EntitySnapshot.h"

namespace Uma_Editor
{
    class EntityCreateCmd : public ICommand
    {
    private:
        Uma_ECS::Coordinator* coordinator;
        Uma_ECS::Entity entityID;
        std::optional<Uma_ECS::Entity> parentID;
        std::string description;
    public:
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

        void Execute() override
        {
            entityID = coordinator->CreateEntity();

            coordinator->AddComponent(entityID,
                Uma_ECS::Transform{
                    .name = std::string("new entity"),
                    .position = Vec2(0.f, 0.f),
                    .rotation = Vec2(0.f, 0.f),
                    .scale = Vec2(1, 1),
                });

            if (parentID != std::nullopt)
            {
                coordinator->SetParent(entityID, parentID.value());
            }
        }

        void Undo() override
        {
            coordinator->DestroyEntity(entityID);
        }

        std::string GetDescription() override
        {
            return description;
        }

        Uma_ECS::Entity GetCreatedEntity()
        {
            return entityID;
        }


    };
}