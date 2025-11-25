/*!
\file   Coordinator.cpp
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Implements the Coordinator's entity lifecycle management, event emission, and serialization methods.

Coordinates entity creation/destruction across all three managers (Entity, Component, System) and emits corresponding events.
Handles entity duplication by cloning components and updating system membership.
Template methods handle component and system operations with automatic signature updates and system membership
recalculation. Implements ISerializer for JSON-based scene serialization with RapidJSON. Integrates with
Uma_Engine::EventSystem to emit entity lifecycle events for external observers.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#include "Coordinator.hpp"
#include "Events/IMGUIEvents.h"

#include "Debugging/Debugger.hpp"

#include <fstream>
#include <rapidjson/document.h>

#include "Components/Transform.h"

namespace Uma_ECS
{
    void Coordinator::Init(Uma_Engine::EventSystem* eventSystem)
    {
        aComponentManager = std::make_unique<ComponentManager>();
        aEntityManager = std::make_unique<EntityManager>();
        aSystemManager = std::make_unique<SystemManager>();

        mStateCache.cachedEntityManager = nullptr;
        mStateCache.cachedComponentManager = nullptr;

        pEventSystem = eventSystem;
    }

    Entity Coordinator::CreateEntity()
    {
        Entity en = aEntityManager->CreateEntity();

        if (en >= 0) // entity is created
        {
            pEventSystem->Emit<Uma_Engine::EntityCreatedEvent>(en, GetEntityCount());
        }

        std::string log;
        std::stringstream ss(log);
        ss << "Created Entity : " << en;
        Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo, ss.str());

        return en;
    }

    void Coordinator::DestroyEntity(Entity entity)
    {
        if (!aEntityManager->IsEntityActive(entity)) {
            return; // Already queued or deleted
        }

        mEntitiesToDestroy.insert(entity);
    }

    bool Coordinator::HasActiveEntity(Entity entity) const
    {
        return aEntityManager->HasActiveEntity(entity);
    }

    Signature Coordinator::GetEntitySignature(Entity entity)
    {
        return aEntityManager->GetSignature(entity);
    }

    int Coordinator::GetEntityCount() const
    {
        return aEntityManager->GetEntityCount();
    }

    Entity Coordinator::DuplicateEntity(Entity src)
    {
        // Use helper function that handles hierarchy
        std::unordered_map<Entity, Entity> oldToNewMap;
        Entity rootDuplicate = DuplicateEntityHierarchy(src, oldToNewMap);
        return rootDuplicate;
    }

    // New helper function for recursive duplication
    Entity Coordinator::DuplicateEntityHierarchy(Entity src, std::unordered_map<Entity, Entity>& oldToNewMap)
    {
        // Create new entity
        Entity newEntity = CreateEntity();
        oldToNewMap[src] = newEntity;

        // Handle LuaScript specially - remove before cloning
        auto& luaScriptArray = aComponentManager->GetComponentArray<LuaScript>();
        bool hadLuaScript = luaScriptArray.Has(src);

        LuaScript srcLuaScriptBackup;
        if (hadLuaScript)
        {
            srcLuaScriptBackup = luaScriptArray.GetData(src);
            luaScriptArray.RemoveData(src);
        }

        // Clone all components except LuaScript
        aComponentManager->CloneEntityComponents(src, newEntity);

        // Restore LuaScript to source
        if (hadLuaScript)
        {
            luaScriptArray.AddData(src, srcLuaScriptBackup);
        }

        // Get transform to handle children
        auto& tfArray = aComponentManager->GetComponentArray<Transform>();

        // Clear parent on the duplicate (will be set later if needed)
        if (tfArray.Has(newEntity))
        {
            auto& newTf = tfArray.GetData(newEntity);
            newTf.parent = std::nullopt;
            newTf.children.clear();
        }

        // Recursively duplicate children
        if (tfArray.Has(src))
        {
            auto& srcTf = tfArray.GetData(src);
            std::vector<Entity> originalChildren = srcTf.children; // Copy to avoid iterator invalidation

            for (Entity child : originalChildren)
            {
                Entity duplicatedChild = DuplicateEntityHierarchy(child, oldToNewMap);

                // Set parent-child relationship
                SetParent(duplicatedChild, newEntity);
            }
        }

        // Build fresh LuaScript if needed
        if (hadLuaScript)
        {
            LuaScript newLuaScript;

            for (const auto& script : srcLuaScriptBackup.scripts)
            {
                newLuaScript.AddScript(script.scriptPath);

                size_t idx = newLuaScript.scripts.size() - 1;
                newLuaScript.scripts[idx].exposedVariables = script.exposedVariables;
                newLuaScript.scripts[idx].isEnabled = script.isEnabled;
                newLuaScript.scripts[idx].isVariableDirty = true;
            }

            AddComponent(newEntity, newLuaScript);
            pEventSystem->Emit<Uma_Engine::CallLuaToInitScript>(newEntity);
        }

        aEntityManager->SetSignature(newEntity, GetEntitySignature(src));

        aSystemManager->EntitySignatureChanged(newEntity, GetEntitySignature(newEntity));

        return newEntity;
    }

    void Coordinator::DestroyAllEntities()
    {
        std::vector<Entity> enList = aEntityManager->GetAllEntites();

        for (auto const& en : enList)
        {
            DestroyEntity(en);
        }

        std::string log;
        std::stringstream ss(log);
        ss << "Destroyed Entities : " << enList.size();
        Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eInfo, ss.str());
    }

    // Find entities with a component by string name (for Lua)
    std::vector<Entity> Coordinator::FindEntitiesWithComponentByName(const std::string& componentName)
    {
        std::vector<Entity> result = aComponentManager->GetEntitiesByComponentName(componentName);

        return result;
    }

    // Find first entity with a component by string name (for Lua)
    Entity Coordinator::FindEntityWithComponentByName(const std::string& componentName)
    {
        std::vector<Entity> result = aComponentManager->GetEntitiesByComponentName(componentName);

        if (result.size() <= 0)
        {
            return static_cast<Entity>(-1);
        }

        return result[0];
    }

    void Coordinator::SetParent(Entity child, Entity parent)
    {
        if (child == parent)
        {
            Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eWarning,
                "Cannot set entity as its own parent");
            return;
        }

        // check for circular dependency
        std::optional<Entity> checkEntity = parent;
        while (checkEntity.has_value())
        {
            if (checkEntity.value() == child)
            {
                Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eWarning,
                    "Circular parent-child relationship detected");
                return;
            }
            checkEntity = GetParent(checkEntity.value());
        }

        auto& childTf = GetComponent<Transform>(child);

        // remove from old parent
        if (childTf.parent.has_value())
        {
            auto& oldParentTf = GetComponent<Transform>(childTf.parent.value());
            auto it = std::find(oldParentTf.children.begin(), oldParentTf.children.end(), child);
            if (it != oldParentTf.children.end())
            {
                oldParentTf.children.erase(it);
            }
        }

        // set new parent
        childTf.parent = parent;

        auto& parentTf = GetComponent<Transform>(parent);
        parentTf.children.push_back(child);

        // Convert world position to local position relative to new parent
        //childTf.position = childTf.worldPosition - parentTf.worldPosition;
        childTf.isDirty = true;
    }

    void Coordinator::RemoveParent(Entity child)
    {
        auto& childTf = GetComponent<Transform>(child);

        if (!childTf.parent.has_value())
        {
            // this entity doesnt have any parent
            Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eWarning,
                "Entity doesnt have any parent");
            return;
        }

        // have a parent
        auto& parentTf = GetComponent<Transform>(childTf.parent.value());
        auto it = std::find(std::begin(parentTf.children), std::end(parentTf.children), child);
        if (it != std::end(parentTf.children))
        {
            parentTf.children.erase(it);
        }

        childTf.parent = std::nullopt;
        childTf.position = childTf.worldPosition;
        childTf.isDirty = true;
    }

    std::optional<Entity> Coordinator::GetParent(Entity entity)
    {
        if (!aEntityManager->IsEntityActive(entity))
        {
            return std::nullopt;
        }

        auto& tfArray = aComponentManager->GetComponentArray<Transform>();
        if (!tfArray.Has(entity))
        {
            return std::nullopt;
        }

        return tfArray.GetData(entity).parent;
    }

    std::vector<Entity> Coordinator::GetChildren(Entity entity)
    {
        if (!aEntityManager->IsEntityActive(entity))
            return {};

        auto& tfArray = aComponentManager->GetComponentArray<Transform>();
        if (!tfArray.Has(entity))
            return {};

        return tfArray.GetData(entity).children;
    }

    void Coordinator::DestroyEntityAndChildren(Entity entity)
    {
        if (!aEntityManager->IsEntityActive(entity)) {
            return;
        }

        // Collect entire hierarchy
        std::vector<Entity> hierarchy;
        CollectHierarchy(entity, hierarchy);

        // Queue all at once
        for (Entity e : hierarchy) {
            mEntitiesToDestroy.insert(e);
        }
    }

    void Coordinator::ProcessDeletionQueue()
    {
        if (mEntitiesToDestroy.empty() || mIsProcessingDeletions) {
            return;
        }

        mIsProcessingDeletions = true;

        // Copy and clear to avoid issues with recursive deletions
        std::unordered_set<Entity> toDelete = std::move(mEntitiesToDestroy);
        mEntitiesToDestroy.clear();

        for (Entity entity : toDelete) {
            if (!aEntityManager->IsEntityActive(entity)) {
                continue; // Skip if already deleted
            }

            auto& tfArray = aComponentManager->GetComponentArray<Transform>();
            if (tfArray.Has(entity)) {
                auto& tf = tfArray.GetData(entity);

                // Remove from parent's children list
                if (tf.parent.has_value() &&
                    aEntityManager->IsEntityActive(tf.parent.value())) {
                    auto& parentTf = tfArray.GetData(tf.parent.value());
                    auto it = std::find(parentTf.children.begin(),
                        parentTf.children.end(), entity);
                    if (it != parentTf.children.end()) {
                        parentTf.children.erase(it);
                    }
                }

                // Orphan children
                for (Entity child : tf.children) {
                    if (tfArray.Has(child) && aEntityManager->IsEntityActive(child)) {
                        auto& childTf = tfArray.GetData(child);
                        childTf.parent = std::nullopt;
                        childTf.position = childTf.worldPosition;
                    }
                }
            }

            // Actual deletion
            aSystemManager->EntityDestroyed(entity);
            aEntityManager->DestroyEntity(entity);
            aComponentManager->EntityDestroyed(entity);

            // Single event per entity (could batch these too)
            pEventSystem->Emit<Uma_Engine::EntityDestroyedEvent>(
                entity, GetEntityCount());
        }

        mIsProcessingDeletions = false;
    }

    void Coordinator::Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator)
    {
        out.SetArray();

        // loop thru all entities
        for (const Entity& en : aEntityManager->GetAllEntites())
        {
            if (!aEntityManager->IsEntityActive(en)) continue;

            rapidjson::Value entityObj(rapidjson::kObjectType);
            entityObj.AddMember("id", en, allocator);

            rapidjson::Value comps(rapidjson::kObjectType);
            aComponentManager->SerializeAll(en, comps, allocator);
            entityObj.AddMember("components", comps, allocator);

            out.PushBack(entityObj, allocator);
        }
    }

    void Coordinator::Deserialize(const rapidjson::Value& in)
    {
        assert(in.IsArray());

        // Map old entity IDs to new entity IDs
        std::unordered_map<Entity, Entity> entityIDMap;

        // First pass: Create all entities
        for (auto& entityVal : in.GetArray())
        {
            Entity oldID = entityVal["id"].GetUint();
            Entity newID = CreateEntity();
            entityIDMap[oldID] = newID;

            const auto& comps = entityVal["components"];
            Signature sign = aComponentManager->DeserializeAll(newID, comps);
            aEntityManager->SetSignature(newID, sign);
        }

        // Second pass: Remap parent-child relationships
        auto& tfArray = aComponentManager->GetComponentArray<Transform>();
        for (size_t i = 0; i < tfArray.Size(); ++i)
        {
            Entity entity = tfArray.GetEntity(i);
            auto& tf = tfArray.GetData(entity);

            // Remap parent ID
            if (tf.parent.has_value())
            {
                Entity oldParentID = tf.parent.value();

                auto it = entityIDMap.find(oldParentID);
                if (it != entityIDMap.end())
                {
                    Entity newParentID = it->second;
                    tf.parent = newParentID;

                    // Add to parent's children
                    auto& parentTf = tfArray.GetData(newParentID);
                    parentTf.children.push_back(entity);
                }
                else
                {
                    Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eWarning,
                        "Invalid parent ID during deserialization");
                    tf.parent = std::nullopt;
                }
            }

            // Remap children IDs
            std::vector<Entity> newChildren;
            for (Entity oldChildID : tf.children)
            {
                auto it = entityIDMap.find(oldChildID);
                if (it != entityIDMap.end())
                {
                    newChildren.push_back(it->second);
                }
            }
            tf.children = std::move(newChildren);
        }

        // Third pass: Update systems
        for (auto& pair : entityIDMap)
        {
            Entity newID = pair.second;
            aSystemManager->EntitySignatureChanged(newID, GetEntitySignature(newID));
        }
    }

    void Coordinator::SerializePrefab(Entity entity, rapidjson::Value& out, rapidjson::Document::AllocatorType& allocator)
    {
        if (!aEntityManager->IsEntityActive(entity)) return;

        out.SetObject();

        // Serialize the entire hierarchy starting from this entity
        rapidjson::Value entitiesArray(rapidjson::kArrayType);

        std::vector<Entity> hierarchyEntities;
        CollectHierarchy(entity, hierarchyEntities);

        // Create ID mapping for the prefab (starting from 0)
        std::unordered_map<Entity, Entity> worldToPrefabID;
        for (size_t i = 0; i < hierarchyEntities.size(); ++i)
        {
            worldToPrefabID[hierarchyEntities[i]] = static_cast<Entity>(i);
        }

        // Serialize each entity with remapped IDs
        for (Entity e : hierarchyEntities)
        {
            rapidjson::Value entityObj(rapidjson::kObjectType);

            Entity prefabID = worldToPrefabID[e];
            entityObj.AddMember("id", prefabID, allocator);

            // Serialize components
            rapidjson::Value comps(rapidjson::kObjectType);
            aComponentManager->SerializeAll(e, comps, allocator);

            // Remap parent and children IDs in Transform component
            if (comps.HasMember("struct Uma_ECS::Transform")) // Use your actual type name
            {
                auto& transformComp = comps["struct Uma_ECS::Transform"];

                // Remap parent
                if (transformComp.HasMember("parent"))
                {
                    int oldParentID = transformComp["parent"].GetInt();
                    if (oldParentID >= 0)
                    {
                        auto it = worldToPrefabID.find(static_cast<Entity>(oldParentID));
                        if (it != worldToPrefabID.end())
                        {
                            transformComp["parent"] = it->second;
                        }
                        else
                        {
                            // Parent is outside hierarchy - make this a root
                            transformComp["parent"] = -1;
                        }
                    }
                }

                // Remap children
                if (transformComp.HasMember("children"))
                {
                    auto& childrenArray = transformComp["children"];
                    for (auto& childVal : childrenArray.GetArray())
                    {
                        Entity oldChildID = childVal.GetUint();
                        auto it = worldToPrefabID.find(oldChildID);
                        if (it != worldToPrefabID.end())
                        {
                            childVal = it->second;
                        }
                    }
                }
            }

            entityObj.AddMember("components", comps, allocator);
            entitiesArray.PushBack(entityObj, allocator);
        }

        out.AddMember("entities", entitiesArray, allocator);
        out.AddMember("rootEntity", worldToPrefabID[entity], allocator);
    }

    // Helper function to collect entire hierarchy
    void Coordinator::CollectHierarchy(Entity root, std::vector<Entity>& outEntities)
    {
        outEntities.push_back(root);

        auto& tfArray = aComponentManager->GetComponentArray<Transform>();
        if (tfArray.Has(root))
        {
            auto& tf = tfArray.GetData(root);
            for (Entity child : tf.children)
            {
                CollectHierarchy(child, outEntities); // Recursive
            }
        }
    }

    Entity Coordinator::DeserializePrefab(const rapidjson::Value& in)
    {
        if (!in.HasMember("entities") || !in["entities"].IsArray())
        {
            Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eError,
                "Invalid prefab format: missing entities array");
            return static_cast<Entity>(-1);
        }

        const auto& entitiesArray = in["entities"];

        // Map prefab IDs to new world IDs
        std::unordered_map<Entity, Entity> prefabToWorldID;

        // First pass: Create all entities
        for (const auto& entityVal : entitiesArray.GetArray())
        {
            Entity prefabID = entityVal["id"].GetUint();
            Entity newWorldID = CreateEntity();
            prefabToWorldID[prefabID] = newWorldID;

            const auto& comps = entityVal["components"];
            Signature sign = aComponentManager->DeserializeAll(newWorldID, comps);
            aEntityManager->SetSignature(newWorldID, sign);
        }

        // Second pass: Remap parent-child relationships
        auto& tfArray = aComponentManager->GetComponentArray<Transform>();

        for (const auto& entityVal : entitiesArray.GetArray())
        {
            Entity prefabID = entityVal["id"].GetUint();
            Entity worldID = prefabToWorldID[prefabID];

            if (!tfArray.Has(worldID)) continue;

            auto& tf = tfArray.GetData(worldID);

            // Remap parent
            if (tf.parent.has_value())
            {
                Entity oldParentID = tf.parent.value();
                auto it = prefabToWorldID.find(oldParentID);

                if (it != prefabToWorldID.end())
                {
                    tf.parent = it->second;
                }
                else
                {
                    // Parent not in prefab - clear it
                    tf.parent = std::nullopt;
                }
            }

            // Remap children
            std::vector<Entity> remappedChildren;
            for (Entity oldChildID : tf.children)
            {
                auto it = prefabToWorldID.find(oldChildID);
                if (it != prefabToWorldID.end())
                {
                    remappedChildren.push_back(it->second);
                }
            }
            tf.children = std::move(remappedChildren);
        }

        // Third pass: Rebuild parent-child relationships (ensure consistency)
        for (const auto& pair : prefabToWorldID)
        {
            Entity worldID = pair.second;

            if (!tfArray.Has(worldID)) continue;

            auto& tf = tfArray.GetData(worldID);

            if (tf.parent.has_value())
            {
                Entity parentID = tf.parent.value();

                if (tfArray.Has(parentID))
                {
                    auto& parentTf = tfArray.GetData(parentID);

                    // Ensure child is in parent's children list
                    if (std::find(parentTf.children.begin(), parentTf.children.end(), worldID)
                        == parentTf.children.end())
                    {
                        parentTf.children.push_back(worldID);
                    }
                }
            }
        }

        // Fourth pass: Update systems
        for (const auto& pair : prefabToWorldID)
        {
            Entity worldID = pair.second;
            aSystemManager->EntitySignatureChanged(worldID, GetEntitySignature(worldID));
        }

        // Return the root entity
        if (in.HasMember("rootEntity"))
        {
            Entity prefabRootID = in["rootEntity"].GetUint();
            return prefabToWorldID[prefabRootID];
        }

        // Fallback: return first entity
        return prefabToWorldID.begin()->second;
    }

    void Coordinator::SerializeEntity(Entity entity, rapidjson::Value& comps, rapidjson::Document::AllocatorType& allocator)
    {
        aComponentManager->SerializeAll(entity, comps, allocator);
    }

    void Coordinator::DeserializeEntity(Entity entity, const rapidjson::Value& comps)
    {
        Signature sign = aComponentManager->DeserializeAll(entity, comps);
        aEntityManager->SetSignature(entity, sign);
        aSystemManager->EntitySignatureChanged(entity, sign);
    }

    void Coordinator::CacheState()
    {
        mStateCache.cachedEntityManager = std::make_unique<EntityManager>(*aEntityManager);
        mStateCache.cachedComponentManager = std::make_unique<ComponentManager>(*aComponentManager);
    }

    void Coordinator::RestoreState()
    {
        if (!mStateCache.cachedEntityManager || !mStateCache.cachedComponentManager) return;

        // Step 1: Clear all entity references from systems
        aSystemManager->ClearAllEntities();

        // Step 2: Restore the cached entity and component managers
        aEntityManager = std::make_unique<EntityManager>(*mStateCache.cachedEntityManager);
        aComponentManager = std::make_unique<ComponentManager>(*mStateCache.cachedComponentManager);

        // Step 3: Clear Lua runtime data from restored LuaScript components
        auto& luaScriptArray = aComponentManager->GetComponentArray<LuaScript>();
        for (size_t i = 0; i < luaScriptArray.Size(); ++i)
        {
            Entity entity = luaScriptArray.GetEntity(i);
            auto& luaScript = luaScriptArray.GetData(entity);

            // Clear runtime-only data for each script instance
            for (auto& script : luaScript.scripts)
            {
                script.scriptEnv.reset();           // Release the stale Lua environment
                script.isInitialized = false;       // Mark as uninitialized
                script.hasError = false;            // Clear error state
                script.errorMessage.clear();        // Clear error message
                script.wasEnabledLastFrame = false; // Reset frame tracking
                // Note: Keep exposedVariables, scriptPath, and isEnabled to preserve editor settings
            }
        }

        // Step 4: Rebuild system entity sets by notifying systems of all active entities
        for (Entity entity = 0; entity < MAX_ENTITIES; ++entity)
        {
            if (aEntityManager->IsEntityActive(entity))
            {
                Signature signature = aEntityManager->GetSignature(entity);
                aSystemManager->EntitySignatureChanged(entity, signature);
            }
        }

        mStateCache.cachedComponentManager.release();
        mStateCache.cachedEntityManager.release();
    }

    Uma_Engine::EventSystem* Coordinator::GetEventSystem()
    {
        return pEventSystem;
    }
}
