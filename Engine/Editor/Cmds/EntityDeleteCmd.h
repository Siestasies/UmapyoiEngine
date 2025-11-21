#pragma once

#include "Editor/Core/Command.h"

#include "ECS/Core/Types.hpp"
#include "ECS/Core/Coordinator.hpp"
#include "Editor/Core/EntitySnapshot.h"
#include <vector>
#include <unordered_map>
#include <memory>

namespace Uma_Editor
{
    class EntityDeleteCmd : public ICommand
    {
    public:
        EntityDeleteCmd(
            Uma_ECS::Coordinator* coord,
            Uma_ECS::Entity entity,
            bool deleteChildren = false,
            std::string desc = "Delete Entity"
        )
            : coordinator(coord)
            , rootEntityID(entity)
            , deleteWithChild(deleteChildren)
            , description(desc)
        {
            CaptureEntityState();
        }

        void Execute() override
        {
            if (coordinator->HasActiveEntity(rootEntityID))
            {
                if (!deleteWithChild)
                {
                    coordinator->DestroyEntity(rootEntityID);
                }
                else
                {
                    coordinator->DestroyEntityAndChildren(rootEntityID);
                }
            }
        }

        void Undo() override
        {
            if (!deleteWithChild)
            {
                RestoreSingleEntityWithChildren();
            }
            else
            {
                RestoreHierarchy();
            }
        }

        std::string GetDescription() override
        {
            return description;
        }

    private:

        void CaptureEntityState()
        {
            if (!coordinator->HasActiveEntity(rootEntityID))
                return;

            if (!deleteWithChild)
            {
                // Capture parent and all children (children's transforms will be modified on delete)
                CaptureSingleEntityAndChildren(rootEntityID);
            }
            else
            {
                // Capture entire hierarchy (all will be deleted)
                CaptureHierarchySnapshots(rootEntityID);
            }
        }

        void CaptureSnapshot(Uma_ECS::Entity entity)
        {
            // Create snapshot using unique_ptr
            auto snapshot = std::make_unique<EntitySnapshot>();
            snapshot->entityID = entity;
            snapshot->componentData.SetObject();
            auto& allocator = snapshot->componentData.GetAllocator();

            rapidjson::Value componentsObj(rapidjson::kObjectType);
            coordinator->SerializeEntity(entity, componentsObj, allocator);
            snapshot->componentData.AddMember("components", componentsObj, allocator);

            // Capture hierarchy
            auto& tfArray = coordinator->GetComponentArray<Uma_ECS::Transform>();
            if (tfArray.Has(entity))
            {
                auto& tf = tfArray.GetData(entity);
                snapshot->parentID = tf.parent;
                snapshot->childrenIDs = tf.children;
            }

            // Store in map using move
            entitySnapshots[entity] = std::move(snapshot);
        }

        void CaptureSingleEntityAndChildren(Uma_ECS::Entity parent)
        {
            // Capture the parent entity
            CaptureSnapshot(parent);

            // Get children list
            auto& tfArray = coordinator->GetComponentArray<Uma_ECS::Transform>();
            if (tfArray.Has(parent))
            {
                auto& tf = tfArray.GetData(parent);
                affectedChildren = tf.children; // Store children IDs

                // Capture all children (their transforms will be modified during delete)
                for (Uma_ECS::Entity child : tf.children)
                {
                    if (coordinator->HasActiveEntity(child))
                    {
                        CaptureSnapshot(child);
                    }
                }
            }
        }

        void CaptureHierarchySnapshots(Uma_ECS::Entity root)
        {
            // Build list of all entities in hierarchy (breadth-first to maintain order)
            std::vector<Uma_ECS::Entity> hierarchyList;
            CollectHierarchyOrdered(root, hierarchyList);

            // Capture snapshots for all entities
            for (Uma_ECS::Entity entity : hierarchyList)
            {
                CaptureSnapshot(entity);
            }

            // Store the restoration order
            restorationOrder = hierarchyList;
        }

        void CollectHierarchyOrdered(Uma_ECS::Entity root, std::vector<Uma_ECS::Entity>& outList)
        {
            if (!coordinator->HasActiveEntity(root))
                return;

            outList.push_back(root);

            auto& tfArray = coordinator->GetComponentArray<Uma_ECS::Transform>();
            if (tfArray.Has(root))
            {
                auto& tf = tfArray.GetData(root);
                for (Uma_ECS::Entity child : tf.children)
                {
                    CollectHierarchyOrdered(child, outList);
                }
            }
        }

        void RestoreSingleEntityWithChildren()
        {
            // First restore the parent entity
            Uma_ECS::Entity restoredParentID = RestoreEntity(rootEntityID);

            // Then restore all affected children's transforms
            auto& tfArray = coordinator->GetComponentArray<Uma_ECS::Transform>();

            for (Uma_ECS::Entity childID : affectedChildren)
            {
                if (!coordinator->HasActiveEntity(childID))
                    continue;

                auto it = entitySnapshots.find(childID);
                if (it == entitySnapshots.end())
                    continue;

                EntitySnapshot* snapshot = it->second.get();

                // Restore child's transform component from snapshot
                if (snapshot->componentData.HasMember("components"))
                {
                    coordinator->DeserializeEntity(childID, snapshot->componentData["components"]);
                }

                // Re-establish parent-child relationship
                if (tfArray.Has(childID))
                {
                    auto& childTf = tfArray.GetData(childID);

                    // Restore parent pointer
                    childTf.parent = restoredParentID;

                    // Restore children list from snapshot
                    childTf.children = snapshot->childrenIDs;

                    childTf.isDirty = true;
                }
            }

            // Ensure parent has correct children list
            if (tfArray.Has(restoredParentID))
            {
                auto& parentTf = tfArray.GetData(restoredParentID);

                // Update children list (some might have new IDs if restoration failed)
                std::vector<Uma_ECS::Entity> validChildren;
                for (Uma_ECS::Entity child : affectedChildren)
                {
                    if (coordinator->HasActiveEntity(child))
                    {
                        validChildren.push_back(child);

                        // Make sure child points to parent
                        if (tfArray.Has(child))
                        {
                            auto& childTf = tfArray.GetData(child);
                            childTf.parent = restoredParentID;
                        }
                    }
                }
                parentTf.children = validChildren;
                parentTf.isDirty = true;
            }
        }

        Uma_ECS::Entity RestoreEntity(Uma_ECS::Entity originalID)
        {
            auto it = entitySnapshots.find(originalID);
            if (it == entitySnapshots.end())
            {
                Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eError,
                    "No snapshot found for entity " + std::to_string(originalID));
                return originalID;
            }

            EntitySnapshot* snapshot = it->second.get();

            // Create entity - should get same ID if it was pushed to front
            Uma_ECS::Entity restoredID = coordinator->CreateEntity();

            // Track ID mapping in case it changed
            if (restoredID != originalID)
            {
                Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eWarning,
                    "Failed to restore entity with same ID: expected " +
                    std::to_string(originalID) + ", got " + std::to_string(restoredID));

                // Update the snapshot's ID for future reference
                snapshot->entityID = restoredID;
            }

            // Restore all components from snapshot
            if (snapshot->componentData.HasMember("components"))
            {
                coordinator->DeserializeEntity(restoredID, snapshot->componentData["components"]);
            }

            // Restore hierarchy relationships
            auto& tfArray = coordinator->GetComponentArray<Uma_ECS::Transform>();
            if (tfArray.Has(restoredID))
            {
                auto& tf = tfArray.GetData(restoredID);

                // Restore children list
                tf.children = snapshot->childrenIDs;

                // Restore parent relationship
                if (snapshot->parentID.has_value())
                {
                    if (coordinator->HasActiveEntity(snapshot->parentID.value()))
                    {
                        coordinator->SetParent(restoredID, snapshot->parentID.value());
                    }
                    else
                    {
                        tf.parent = std::nullopt;
                    }
                }

                tf.isDirty = true;
            }

            return restoredID;
        }

        void RestoreHierarchy()
        {
            if (restorationOrder.empty())
            {
                Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eError,
                    "No restoration order found for hierarchy");
                return;
            }

            // Map old entity IDs to new entity IDs
            std::unordered_map<Uma_ECS::Entity, Uma_ECS::Entity> idRemapping;

            // First pass: Create all entities and restore components
            for (Uma_ECS::Entity originalID : restorationOrder)
            {
                auto it = entitySnapshots.find(originalID);
                if (it == entitySnapshots.end())
                    continue;

                EntitySnapshot* snapshot = it->second.get();

                // Create entity
                Uma_ECS::Entity newID = coordinator->CreateEntity();
                idRemapping[originalID] = newID;

                // Restore components
                if (snapshot->componentData.HasMember("components"))
                {
                    coordinator->DeserializeEntity(newID, snapshot->componentData["components"]);
                }

                // Log if ID changed
                if (newID != originalID)
                {
                    Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo,
                        "Entity ID remapped: " + std::to_string(originalID) +
                        " -> " + std::to_string(newID));
                }
            }

            // Second pass: Restore hierarchy relationships with remapped IDs
            auto& tfArray = coordinator->GetComponentArray<Uma_ECS::Transform>();

            for (Uma_ECS::Entity originalID : restorationOrder)
            {
                Uma_ECS::Entity newID = idRemapping[originalID];
                auto it = entitySnapshots.find(originalID);
                if (it == entitySnapshots.end() || !tfArray.Has(newID))
                    continue;

                EntitySnapshot* snapshot = it->second.get();
                auto& tf = tfArray.GetData(newID);

                // Remap and restore parent
                if (snapshot->parentID.has_value())
                {
                    Uma_ECS::Entity oldParentID = snapshot->parentID.value();

                    // Check if parent was part of deleted hierarchy
                    auto parentIt = idRemapping.find(oldParentID);
                    if (parentIt != idRemapping.end())
                    {
                        // Parent was restored, use remapped ID
                        Uma_ECS::Entity newParentID = parentIt->second;
                        coordinator->SetParent(newID, newParentID);
                    }
                    else if (coordinator->HasActiveEntity(oldParentID))
                    {
                        // Parent still exists with original ID
                        coordinator->SetParent(newID, oldParentID);
                    }
                    else
                    {
                        // Parent no longer exists
                        tf.parent = std::nullopt;
                    }
                }

                // Remap and restore children
                std::vector<Uma_ECS::Entity> remappedChildren;
                for (Uma_ECS::Entity oldChildID : snapshot->childrenIDs)
                {
                    auto childIt = idRemapping.find(oldChildID);
                    if (childIt != idRemapping.end())
                    {
                        // Child was restored, use remapped ID
                        remappedChildren.push_back(childIt->second);
                    }
                    else if (coordinator->HasActiveEntity(oldChildID))
                    {
                        // Child still exists with original ID
                        remappedChildren.push_back(oldChildID);
                    }
                }
                tf.children = remappedChildren;

                // Ensure all children have correct parent pointer
                for (Uma_ECS::Entity childID : tf.children)
                {
                    if (tfArray.Has(childID))
                    {
                        auto& childTf = tfArray.GetData(childID);
                        childTf.parent = newID;
                    }
                }

                tf.isDirty = true;
            }

            // Update root entity ID if it changed
            if (idRemapping.find(rootEntityID) != idRemapping.end())
            {
                rootEntityID = idRemapping[rootEntityID];
            }
        }

        Uma_ECS::Coordinator* coordinator;
        Uma_ECS::Entity rootEntityID;
        bool deleteWithChild;
        std::string description;

        // Storage for snapshots using unique_ptr to avoid copy issues
        std::unordered_map<Uma_ECS::Entity, std::unique_ptr<EntitySnapshot>> entitySnapshots;
        std::vector<Uma_ECS::Entity> restorationOrder;
        std::vector<Uma_ECS::Entity> affectedChildren; // Children affected by single parent deletion
    };
}