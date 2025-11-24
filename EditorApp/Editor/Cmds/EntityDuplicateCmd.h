#pragma once

#include "Editor/Core/Command.h"

#include "ECS/Core/Types.hpp"
#include "ECS/Core/Coordinator.hpp"
#include "Editor/Core/EntitySnapshot.h"

namespace Uma_Editor
{
    class EntityDuplicateCmd : public ICommand
    {
    private:

        Uma_ECS::Coordinator* coordinator;
        Uma_ECS::Entity srcEntity;
        Uma_ECS::Entity createdEntity;
        std::string description;

    public:

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

        void Execute() override
        {
            createdEntity = coordinator->DuplicateEntity(srcEntity);

            auto& tf = coordinator->GetComponent<Uma_ECS::Transform>(createdEntity);
            tf.name += "_dup";
        }

        void Undo() override
        {
            coordinator->DestroyEntityAndChildren(createdEntity);
        }

        std::string GetDescription() override
        {
            return description;
        }

        Uma_ECS::Entity GetCreatedEntity()
        {
            return createdEntity;
        }
    };
}