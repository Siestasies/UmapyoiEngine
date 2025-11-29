/*!
\file   EntityDeleteCmd.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines the EntityDeleteCmd class, a command used by the Editor's Undo/Redo
system to delete entities. This command supports two deletion behaviors:
removing only the selected entity, or removing the entity along with all
its children (its entire hierarchy).

The command captures full entity snapshots before deletion, storing all
components and hierarchy information so that Undo() can fully restore the
original state, even for complex multi-level hierarchies.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

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
    /**
     * \class EntityDeleteCmd
     * \brief Command that deletes an entity from the ECS.
     *
     * Supports:
     * - Deleting only the root entity.
     * - Deleting the entire hierarchy (entity + all children).
     *
     * The Undo() operation restores the full state of all involved entities,
     * recreating components, hierarchy relationships, and transforming data.
     */
    class EntityDeleteCmd : public ICommand
    {
    public:
        /**
         * \brief Constructor for the entity deletion command.
         *
         * \param coord Pointer to the ECS Coordinator.
         * \param entity The entity to delete.
         * \param deleteChildren Whether to delete the entity’s full hierarchy.
         * \param desc Optional description used for the UI.
         */
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
            // Take a snapshot before deletion.
            CaptureEntityState();
        }

        /**
         * \brief Executes the deletion of the entity or hierarchy.
         */
        void Execute() override
        {
            if (coordinator->HasActiveEntity(rootEntityID))
            {
                if (!deleteWithChild)
                {
                    // Delete only the entity itself.
                    coordinator->DestroyEntity(rootEntityID);
                }
                else
                {
                    // Delete the entity and the entire hierarchy of children.
                    coordinator->DestroyEntityAndChildren(rootEntityID);
                }
            }
        }

        /**
         * \brief Restores the entity or entire hierarchy depending on deletion mode.
         */
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

        /**
         * \brief Gets the description of the command.
         * \return The descriptive string.
         */
        std::string GetDescription() override
        {
            return description;
        }

    private:

        // =====================================================================
        //  Snapshot Capture
        // =====================================================================

        /**
         * \brief Captures the state of the entity before deletion.
         */
        void CaptureEntityState()
        {
            if (!coordinator->HasActiveEntity(rootEntityID))
                return;

            if (!deleteWithChild)
            {
                // Single-entity delete: capture the entity and its children.
                CaptureSingleEntityAndChildren(rootEntityID);
            }
            else
            {
                // Hierarchy delete: capture entire subtree.
                CaptureHierarchySnapshots(rootEntityID);
            }
        }

        /**
         * \brief Records the components and hierarchy data of one entity.
         */
        void CaptureSnapshot(Uma_ECS::Entity entity)
        {
            auto snapshot = std::make_unique<EntitySnapshot>();
            snapshot->entityID = entity;
            snapshot->componentData.SetObject();
            auto& allocator = snapshot->componentData.GetAllocator();

            // Serialize all components.
            rapidjson::Value componentsObj(rapidjson::kObjectType);
            coordinator->SerializeEntity(entity, componentsObj, allocator);
            snapshot->componentData.AddMember("components", componentsObj, allocator);

            // Capture hierarchy data (parent & children).
            auto& tfArray = coordinator->GetComponentArray<Uma_ECS::Transform>();
            if (tfArray.Has(entity))
            {
                auto& tf = tfArray.GetData(entity);
                snapshot->parentID = tf.parent;
                snapshot->childrenIDs = tf.children;
            }

            entitySnapshots[entity] = std::move(snapshot);
        }

        /**
         * \brief Captures the entity and all its immediate children.
         */
        void CaptureSingleEntityAndChildren(Uma_ECS::Entity parent)
        {
            CaptureSnapshot(parent);

            auto& tfArray = coordinator->GetComponentArray<Uma_ECS::Transform>();
            if (tfArray.Has(parent))
            {
                auto& tf = tfArray.GetData(parent);
                affectedChildren = tf.children;

                for (Uma_ECS::Entity child : tf.children)
                {
                    if (coordinator->HasActiveEntity(child))
                        CaptureSnapshot(child);
                }
            }
        }

        /**
         * \brief Captures the entire entity hierarchy (breadth-first order).
         */
        void CaptureHierarchySnapshots(Uma_ECS::Entity root)
        {
            std::vector<Uma_ECS::Entity> hierarchyList;
            CollectHierarchyOrdered(root, hierarchyList);

            for (Uma_ECS::Entity entity : hierarchyList)
            {
                CaptureSnapshot(entity);
            }

            restorationOrder = hierarchyList;
        }

        /**
         * \brief Recursively collects hierarchy entities in order.
         */
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


        // =====================================================================
        //  Entity Restoration
        // =====================================================================

        /**
         * \brief Restores a single entity and its children.
         */
        void RestoreSingleEntityWithChildren()
        {
            Uma_ECS::Entity restoredParentID = RestoreEntity(rootEntityID);
            auto& tfArray = coordinator->GetComponentArray<Uma_ECS::Transform>();

            for (Uma_ECS::Entity childID : affectedChildren)
            {
                if (!coordinator->HasActiveEntity(childID))
                    continue;

                auto it = entitySnapshots.find(childID);
                if (it == entitySnapshots.end())
                    continue;

                EntitySnapshot* snapshot = it->second.get();

                if (snapshot->componentData.HasMember("components"))
                {
                    coordinator->DeserializeEntity(childID, snapshot->componentData["components"]);
                }

                if (tfArray.Has(childID))
                {
                    auto& childTf = tfArray.GetData(childID);
                    childTf.parent = restoredParentID;
                    childTf.children = snapshot->childrenIDs;
                    childTf.isDirty = true;
                }
            }

            // Rebuild parent's child list.
            if (tfArray.Has(restoredParentID))
            {
                auto& parentTf = tfArray.GetData(restoredParentID);
                std::vector<Uma_ECS::Entity> validChildren;

                for (Uma_ECS::Entity child : affectedChildren)
                {
                    if (coordinator->HasActiveEntity(child))
                        validChildren.push_back(child);
                }

                parentTf.children = validChildren;
                parentTf.isDirty = true;
            }
        }

        /**
         * \brief Restores a single entity from its snapshot.
         * \return The restored entity ID (may differ from original).
         */
        Uma_ECS::Entity RestoreEntity(Uma_ECS::Entity originalID)
        {
            auto it = entitySnapshots.find(originalID);
            if (it == entitySnapshots.end())
                return originalID;

            EntitySnapshot* snapshot = it->second.get();
            Uma_ECS::Entity restoredID = coordinator->CreateEntity();

            if (snapshot->componentData.HasMember("components"))
            {
                coordinator->DeserializeEntity(restoredID, snapshot->componentData["components"]);
            }

            auto& tfArray = coordinator->GetComponentArray<Uma_ECS::Transform>();
            if (tfArray.Has(restoredID))
            {
                auto& tf = tfArray.GetData(restoredID);
                tf.children = snapshot->childrenIDs;

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

        /**
         * \brief Restores a full deleted hierarchy.
         *
         * Restores entity IDs, components, and hierarchy using a two-pass approach.
         */
        void RestoreHierarchy()
        {
            if (restorationOrder.empty())
                return;

            std::unordered_map<Uma_ECS::Entity, Uma_ECS::Entity> idRemapping;

            // First pass: create entities & restore components.
            for (Uma_ECS::Entity originalID : restorationOrder)
            {
                auto it = entitySnapshots.find(originalID);
                if (it == entitySnapshots.end())
                    continue;

                EntitySnapshot* snapshot = it->second.get();
                Uma_ECS::Entity newID = coordinator->CreateEntity();
                idRemapping[originalID] = newID;

                if (snapshot->componentData.HasMember("components"))
                {
                    coordinator->DeserializeEntity(newID, snapshot->componentData["components"]);
                }
            }

            // Second pass: restore hierarchy relationships.
            auto& tfArray = coordinator->GetComponentArray<Uma_ECS::Transform>();

            for (Uma_ECS::Entity originalID : restorationOrder)
            {
                Uma_ECS::Entity newID = idRemapping[originalID];
                auto it = entitySnapshots.find(originalID);

                if (it == entitySnapshots.end() || !tfArray.Has(newID))
                    continue;

                EntitySnapshot* snapshot = it->second.get();
                auto& tf = tfArray.GetData(newID);

                // Remap parent.
                if (snapshot->parentID.has_value())
                {
                    Uma_ECS::Entity oldParent = snapshot->parentID.value();

                    if (idRemapping.count(oldParent))
                    {
                        coordinator->SetParent(newID, idRemapping[oldParent]);
                    }
                    else if (coordinator->HasActiveEntity(oldParent))
                    {
                        coordinator->SetParent(newID, oldParent);
                    }
                    else
                    {
                        tf.parent = std::nullopt;
                    }
                }

                // Remap children.
                std::vector<Uma_ECS::Entity> remapped;
                for (Uma_ECS::Entity oldChild : snapshot->childrenIDs)
                {
                    if (idRemapping.count(oldChild))
                        remapped.push_back(idRemapping[oldChild]);
                }

                tf.children = remapped;

                // Fix child parent links.
                for (Uma_ECS::Entity child : tf.children)
                {
                    if (tfArray.Has(child))
                        tfArray.GetData(child).parent = newID;
                }

                tf.isDirty = true;
            }

            if (idRemapping.count(rootEntityID))
                rootEntityID = idRemapping[rootEntityID];
        }

        // =====================================================================
        //  Member Variables
        // =====================================================================

        Uma_ECS::Coordinator* coordinator; ///< Coordinator pointer for ECS operations.
        Uma_ECS::Entity rootEntityID;      ///< Entity being deleted.
        bool deleteWithChild;              ///< Whether deletion includes children.
        std::string description;           ///< Command description.

        std::unordered_map<Uma_ECS::Entity, std::unique_ptr<EntitySnapshot>> entitySnapshots;
        std::vector<Uma_ECS::Entity> restorationOrder;
        std::vector<Uma_ECS::Entity> affectedChildren;
    };
}
