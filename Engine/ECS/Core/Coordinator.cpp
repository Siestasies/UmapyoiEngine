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
#include <optional>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

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

        aHierarchyOrder.push_back(en);

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

        // Queue all entities for destruction
        for (auto const& en : enList)
        {
            DestroyEntity(en);
        }

        // IMPORTANT: Actually destroy them immediately (don't leave them in queue)
        ProcessDeletionQueue();

        // Clear queue for safety (should already be empty after processing)
        mEntitiesToDestroy.clear();

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

    void Coordinator::RemoveParentSpecial(Entity child)
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

    std::vector<Entity> Coordinator::GetChildrenList(Entity entity)
    {
        if (!aEntityManager->IsEntityActive(entity))
            return {};

        auto& tfArray = aComponentManager->GetComponentArray<Transform>();
        if (!tfArray.Has(entity))
            return {};

        return tfArray.GetData(entity).children;
    }

    Entity Coordinator::GetChildren(Entity entity, int index)
    {
        if (!aEntityManager->IsEntityActive(entity))
            return static_cast<Entity>(-1);

        std::vector<Entity> list = GetChildrenList(entity);

        if (index >= list.size())
            return static_cast<Entity>(-1);

        return list[index];
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

    void Coordinator::SetActive(Entity entity, bool active)
    {
        if (!aEntityManager->IsEntityActive(entity)) {
            Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eWarning,
                "Attempting to SetActive on inactive/destroyed entity");
            return;
        }

        aEntityManager->SetEntityEnabled(entity, active);

        // Emit event for editor/system updates
        if (pEventSystem) {
            pEventSystem->Emit<Uma_Engine::EntityActiveStateChangedEvent>(entity, active);
        }
    }

    bool Coordinator::IsActiveSelf(Entity entity) const
    {
        if (!aEntityManager->IsEntityActive(entity)) {
            return false;
        }

        return aEntityManager->IsEntityEnabled(entity);
    }

    bool Coordinator::IsActiveInHierarchy(Entity entity) const
    {
        if (!aEntityManager->IsEntityActive(entity)) {
            return false;
        }

        // Check if this entity itself is disabled
        if (!aEntityManager->IsEntityEnabled(entity)) {
            return false;
        }

        // Traverse up the parent hierarchy to check if any parent is disabled
        auto& tfArray = aComponentManager->GetComponentArray<Transform>();
        if (!tfArray.Has(entity)) {
            // No transform means no parent, so just return own enabled state
            return true;
        }

        std::optional<Entity> currentParent = tfArray.GetData(entity).parent;
        while (currentParent.has_value())
        {
            Entity parent = currentParent.value();

            // If parent doesn't exist or is disabled, this entity is inactive in hierarchy
            if (!aEntityManager->IsEntityActive(parent) || !aEntityManager->IsEntityEnabled(parent)) {
                return false;
            }

            // Move to next parent up the chain
            if (tfArray.Has(parent)) {
                currentParent = tfArray.GetData(parent).parent;
            }
            else {
                break;
            }
        }

        return true;
    }

    std::vector<Entity> Coordinator::GetActiveEntities(const std::vector<Entity>& entities) const
    {
        std::vector<Entity> activeEntities;
        activeEntities.reserve(entities.size());

        for (Entity entity : entities)
        {
            if (IsActiveInHierarchy(entity))
            {
                activeEntities.push_back(entity);
            }
        }

        return activeEntities;
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

            // Remove from hierarchy order
            auto it = std::find(aHierarchyOrder.begin(), aHierarchyOrder.end(), entity);
            if (it != aHierarchyOrder.end()) {
                aHierarchyOrder.erase(it);
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

        // loop thru all entities in hierarchy order (preserves inspector ordering)
        for (const Entity& en : aHierarchyOrder)
        {
            if (!aEntityManager->IsEntityActive(en)) continue;

            // Skip children of prefab instances - they will be recreated by LoadPrefabInstance
            if (HasComponent<Prefab>(en))
            {
                auto& prefabComp = GetComponent<Prefab>(en);
                if (!prefabComp.isRoot)
                {
                    // This is a child of a prefab instance, skip serializing it
                    continue;
                }
            }

            rapidjson::Value entityObj(rapidjson::kObjectType);
            entityObj.AddMember("id", en, allocator);
            entityObj.AddMember("isActive", aEntityManager->IsEntityEnabled(en), allocator);

            // Serialize components
            rapidjson::Value comps(rapidjson::kObjectType);

            // If this is a prefab instance root, only serialize Transform and Prefab components
            if (HasComponent<Prefab>(en))
            {
                auto& prefabComp = GetComponent<Prefab>(en);
                if (prefabComp.isRoot && !prefabComp.prefabPath.empty())
                {
                    // Only serialize Transform and Prefab - everything else comes from the prefab file
                    if (HasComponent<Transform>(en))
                    {
                        rapidjson::Value transformComp(rapidjson::kObjectType);
                        GetComponent<Transform>(en).Serialize(transformComp, allocator);
                        comps.AddMember("struct Uma_ECS::Transform", transformComp, allocator);
                    }

                    rapidjson::Value prefabCompVal(rapidjson::kObjectType);
                    prefabComp.Serialize(prefabCompVal, allocator);
                    comps.AddMember("struct Uma_ECS::Prefab", prefabCompVal, allocator);
                }
                else
                {
                    // Regular entity - serialize all components
                    aComponentManager->SerializeAll(en, comps, allocator);
                }
            }
            else
            {
                // Regular entity - serialize all components
                aComponentManager->SerializeAll(en, comps, allocator);
            }

            entityObj.AddMember("components", comps, allocator);
            out.PushBack(entityObj, allocator);
        }
    }

    void Coordinator::Deserialize(const rapidjson::Value& in)
    {
        assert(in.IsArray());

        // Map old entity IDs to new entity IDs
        std::unordered_map<Entity, Entity> entityIDMap;

        // First pass: Create all entities and deserialize components
        for (auto& entityVal : in.GetArray())
        {
            Entity oldID = entityVal["id"].GetUint();

            // Handle legacy scene files with old prefab format
            bool isLegacyPrefab = entityVal.HasMember("isPrefab") && entityVal["isPrefab"].GetBool();
            if (isLegacyPrefab)
            {
                // Skip legacy prefab format and warn user
                Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eWarning,
                    "Found legacy prefab format - please re-save the scene to update to new format");

                Entity newID = CreateEntity();
                entityIDMap[oldID] = newID;

                bool isActive = true;
                if (entityVal.HasMember("isActive"))
                {
                    isActive = entityVal["isActive"].GetBool();
                }
                aEntityManager->SetEntityEnabled(newID, isActive);

                // Load using old method for backward compatibility
                if (entityVal.HasMember("prefabPath"))
                {
                    std::string prefabPath = entityVal["prefabPath"].GetString();
                    rapidjson::Value emptyOverride(rapidjson::kObjectType);
                    const rapidjson::Value& transformOverride = entityVal.HasMember("transformOverride")
                        ? entityVal["transformOverride"]
                        : emptyOverride;

                    LoadPrefabInstance(prefabPath, newID, transformOverride);
                }
                continue;
            }

            // Create new entity
            Entity newID = CreateEntity();
            entityIDMap[oldID] = newID;

            // Restore active state
            bool isActive = true;
            if (entityVal.HasMember("isActive"))
            {
                isActive = entityVal["isActive"].GetBool();
            }
            aEntityManager->SetEntityEnabled(newID, isActive);

            // Deserialize components
            if (entityVal.HasMember("components"))
            {
                const auto& comps = entityVal["components"];

                // Check if this is a prefab instance (has Prefab component with isRoot=true)
                bool isPrefabInstance = false;
                std::string prefabPath;

                if (comps.HasMember("struct Uma_ECS::Prefab"))
                {
                    Prefab tempPrefab;
                    tempPrefab.Deserialize(comps["struct Uma_ECS::Prefab"]);
                    if (tempPrefab.isRoot && !tempPrefab.prefabPath.empty())
                    {
                        isPrefabInstance = true;
                        prefabPath = tempPrefab.prefabPath;
                    }
                }

                if (isPrefabInstance)
                {
                    // This is a prefab instance - only read transform and load from prefab file
                    rapidjson::Value emptyOverride(rapidjson::kObjectType);
                    const rapidjson::Value* transformOverride = &emptyOverride;

                    if (comps.HasMember("struct Uma_ECS::Transform"))
                    {
                        transformOverride = &comps["struct Uma_ECS::Transform"];
                    }

                    // Load prefab instance with transform override
                    LoadPrefabInstance(prefabPath, newID, *transformOverride);
                }
                else
                {
                    // Regular entity - deserialize all components normally
                    Signature sign = aComponentManager->DeserializeAll(newID, comps);
                    aEntityManager->SetSignature(newID, sign);
                }
            }
        }

        // Second pass: Remap parent-child relationships
        // Only process entities that were directly deserialized from the scene file
        auto& tfArray = aComponentManager->GetComponentArray<Transform>();

        // First, build reverse map to check which entities came from scene file
        std::unordered_set<Entity> sceneEntities;
        for (const auto& pair : entityIDMap)
        {
            sceneEntities.insert(pair.second);
        }

        // Iterate in hierarchy order so children are added to parents in the correct sequence
        for (const Entity& entity : aHierarchyOrder)
        {
            if (!tfArray.Has(entity)) continue;

            // Skip entities that were created by LoadPrefabInstance (not in scene file)
            if (sceneEntities.count(entity) == 0)
            {
                continue;
            }

            // Also skip prefab instance roots - LoadPrefabInstance already set up their hierarchy
            if (HasComponent<Prefab>(entity))
            {
                auto& prefabComp = GetComponent<Prefab>(entity);
                if (prefabComp.isRoot && !prefabComp.prefabPath.empty())
                {
                    continue;  // Skip prefab instance roots
                }
            }

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

                    // Add to parent's children if not already present
                    // (new scene files have children from deserialization + remap,
                    //  old scene files without "children" field need rebuilding here)
                    if (tfArray.Has(newParentID))
                    {
                        auto& parentTf = tfArray.GetData(newParentID);
                        if (std::find(parentTf.children.begin(), parentTf.children.end(), entity) == parentTf.children.end())
                        {
                            parentTf.children.push_back(entity);
                        }
                    }
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
            entityObj.AddMember("isActive", aEntityManager->IsEntityEnabled(e), allocator);

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

                // Remap children - only keep children that are part of this prefab hierarchy.
                // Stale world IDs left in the array cause circular references on reload
                // because they can accidentally match a valid prefab ID (e.g. 0 = the root itself).
                if (transformComp.HasMember("children"))
                {
                    auto& childrenArray = transformComp["children"];
                    std::vector<Entity> validPrefabChildren;
                    for (auto& childVal : childrenArray.GetArray())
                    {
                        Entity oldChildID = childVal.GetUint();
                        auto it = worldToPrefabID.find(oldChildID);
                        if (it != worldToPrefabID.end())
                        {
                            validPrefabChildren.push_back(it->second);
                        }
                        // Stale reference - drop it silently
                    }
                    childrenArray.Clear();
                    for (Entity prefabChildID : validPrefabChildren)
                    {
                        childrenArray.PushBack(prefabChildID, allocator);
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
        // Guard against cycles
        for (Entity already : outEntities)
        {
            if (already == root)
            {
                Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eError,
                    "Cycle detected in hierarchy at entity: " + std::to_string(root));
                return;
            }
        }

        outEntities.push_back(root);

        auto& tfArray = aComponentManager->GetComponentArray<Transform>();
        if (tfArray.Has(root))
        {
            auto& tf = tfArray.GetData(root);
            for (Entity child : tf.children)
            {
                CollectHierarchy(child, outEntities);
            }
        }
    }

    void Coordinator::CollectPrefabResources(Entity root, PrefabResources& outResources)
    {
        // Collect all entities in the hierarchy
        std::vector<Entity> hierarchyEntities;
        CollectHierarchy(root, hierarchyEntities);

        // Iterate through all entities and collect resource names
        for (Entity entity : hierarchyEntities)
        {
            // Check for Sprite component (uses textures)
            if (HasComponent<Sprite>(entity))
            {
                auto& sprite = GetComponent<Sprite>(entity);
                if (!sprite.texturePath.empty())
                {
                    outResources.textures.insert(sprite.texturePath);
                }
            }

            // Check for AudioComponent (uses sounds)
            if (HasComponent<AudioComponent>(entity))
            {
                auto& audio = GetComponent<AudioComponent>(entity);
                for (const auto& [soundName, soundInstance] : audio.activeSounds)
                {
                    if (!soundName.empty())
                    {
                        outResources.sounds.insert(soundName);
                    }
                }
            }

            // Note: Animator uses sprite sheets which are already tracked via Sprite component
            // Note: Fonts would be tracked here if we had a Text/Label component
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

            // Restore active state (default to true if not present for backward compatibility)
            bool isActive = true;
            if (entityVal.HasMember("isActive"))
            {
                isActive = entityVal["isActive"].GetBool();
            }
            aEntityManager->SetEntityEnabled(newWorldID, isActive);

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
        mStateCache.cachedHierarchyOrder = aHierarchyOrder;
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

        // Step 5: Restore hierarchy order
        aHierarchyOrder = std::move(mStateCache.cachedHierarchyOrder);

        mStateCache.cachedComponentManager.release();
        mStateCache.cachedEntityManager.release();
    }

    Uma_Engine::EventSystem* Coordinator::GetEventSystem()
    {
        return pEventSystem;
    }

    std::unordered_map<Entity, Entity> Coordinator::LoadPrefabInstance(
        const std::string& prefabPath,
        Entity rootEntityID,
        const rapidjson::Value& transformOverride)
    {
        std::unordered_map<Entity, Entity> prefabToWorldID;

        // Load prefab file
        std::ifstream ifs(prefabPath);
        if (!ifs.is_open())
        {
            Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eError,
                "Failed to load prefab file: " + prefabPath);
            return prefabToWorldID;
        }

        rapidjson::IStreamWrapper isw(ifs);
        rapidjson::Document prefabDoc;
        prefabDoc.ParseStream(isw);
        ifs.close();

        if (!prefabDoc.HasMember("Prefab") || !prefabDoc["Prefab"].HasMember("entities"))
        {
            Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eError,
                "Invalid prefab format: " + prefabPath);
            return prefabToWorldID;
        }

        const auto& prefabEntities = prefabDoc["Prefab"]["entities"];
        if (!prefabEntities.IsArray() || prefabEntities.Size() == 0)
        {
            Uma_Engine::Debugger::Log(Uma_Engine::WarningLevel::eError,
                "Empty prefab: " + prefabPath);
            return prefabToWorldID;
        }

        // Clear root entity's stale transform data before deserializing
        // so old children/parent references don't conflict with prefab data
        auto& tfArray = aComponentManager->GetComponentArray<Transform>();
        if (tfArray.Has(rootEntityID))
        {
            auto& rootTf = tfArray.GetData(rootEntityID);
            rootTf.children.clear();
            rootTf.parent = std::nullopt;
        }

        // First pass: Create entities, map IDs, deserialize components
        for (const auto& entityVal : prefabEntities.GetArray())
        {
            Entity prefabID = entityVal["id"].GetUint();
            Entity worldID;

            if (prefabID == 0)
            {
                // Root entity uses the provided ID
                worldID = rootEntityID;
            }
            else
            {
                // Create new entities for children
                worldID = CreateEntity();
            }

            prefabToWorldID[prefabID] = worldID;

            bool isActive = true;
            if (entityVal.HasMember("isActive"))
                isActive = entityVal["isActive"].GetBool();
            aEntityManager->SetEntityEnabled(worldID, isActive);

            // Deserialize all components
            if (entityVal.HasMember("components"))
            {
                const auto& comps = entityVal["components"];
                Signature sign = aComponentManager->DeserializeAll(worldID, comps);
                aEntityManager->SetSignature(worldID, sign);
            }
        }

        // Override root entity's Transform with scene values
        if (HasComponent<Transform>(rootEntityID) && transformOverride.IsObject())
        {
            auto& tf = GetComponent<Transform>(rootEntityID);

            if (transformOverride.HasMember("position"))
            {
                const auto& pos = transformOverride["position"];
                if (pos.IsArray() && pos.Size() >= 2)
                {
                    tf.position.x = pos[0].GetFloat();
                    tf.position.y = pos[1].GetFloat();
                }
                else if (pos.IsObject())
                {
                    if (pos.HasMember("x")) tf.position.x = pos["x"].GetFloat();
                    if (pos.HasMember("y")) tf.position.y = pos["y"].GetFloat();
                }
            }

            if (transformOverride.HasMember("rotation"))
            {
                const auto& rot = transformOverride["rotation"];
                if (rot.IsArray() && rot.Size() >= 2)
                {
                    tf.rotation.x = rot[0].GetFloat();
                    tf.rotation.y = rot[1].GetFloat();
                }
                else if (rot.IsObject())
                {
                    if (rot.HasMember("x")) tf.rotation.x = rot["x"].GetFloat();
                    if (rot.HasMember("y")) tf.rotation.y = rot["y"].GetFloat();
                }
            }

            if (transformOverride.HasMember("scale"))
            {
                const auto& scl = transformOverride["scale"];
                if (scl.IsArray() && scl.Size() >= 2)
                {
                    tf.scale.x = scl[0].GetFloat();
                    tf.scale.y = scl[1].GetFloat();
                }
                else if (scl.IsObject())
                {
                    if (scl.HasMember("x")) tf.scale.x = scl["x"].GetFloat();
                    if (scl.HasMember("y")) tf.scale.y = scl["y"].GetFloat();
                }
            }

            tf.isDirty = true;
        }

        // Second pass: Remap parent and children IDs from prefab space to world space
        // Clear children before remapping to prevent duplicates
        for (const auto& pair : prefabToWorldID)
        {
            Entity worldID = pair.second;

            if (!tfArray.Has(worldID)) continue;

            auto& tf = tfArray.GetData(worldID);

            // Remap parent
            if (tf.parent.has_value())
            {
                Entity oldParentID = tf.parent.value();
                auto it = prefabToWorldID.find(oldParentID);
                tf.parent = (it != prefabToWorldID.end()) ? it->second : -1;
            }

            // Remap children - build fresh list to avoid stale IDs
            std::vector<Entity> remappedChildren;
            for (Entity oldChildID : tf.children)
            {
                auto it = prefabToWorldID.find(oldChildID);
                if (it != prefabToWorldID.end())
                    remappedChildren.push_back(it->second);
            }
            tf.children = std::move(remappedChildren);
        }

        // Third pass: Update systems
        for (const auto& pair : prefabToWorldID)
        {
            Entity worldID = pair.second;
            aSystemManager->EntitySignatureChanged(worldID, GetEntitySignature(worldID));
        }

        // Fourth pass: Add Prefab component to mark all entities as prefab instances
        for (const auto& pair : prefabToWorldID)
        {
            Entity prefabID = pair.first;
            Entity worldID = pair.second;

            if (!HasComponent<Prefab>(worldID))
            {
                Prefab prefabComp;
                prefabComp.prefabPath = prefabPath;
                prefabComp.isRoot = (prefabID == 0);
                AddComponent(worldID, prefabComp);
            }
        }

        return prefabToWorldID;
    }

    // Move entity to a specific index in hierarchy
    void Coordinator::MoveEntityInHierarchy(Entity entity, int newIndex) {
        // Update global hierarchy order
        auto it = std::find(aHierarchyOrder.begin(), aHierarchyOrder.end(), entity);
        if (it == aHierarchyOrder.end()) return;

        aHierarchyOrder.erase(it);
        newIndex = std::clamp(newIndex, 0, static_cast<int>(aHierarchyOrder.size()));
        aHierarchyOrder.insert(aHierarchyOrder.begin() + newIndex, entity);
    }

    // Move entity to a specific index within its parent's children list
    void Coordinator::MoveChildInParent(Entity entity, int newIndex) {
        auto& transformArray = GetComponentArray<Transform>();
        auto& transform = transformArray.GetData(entity);

        if (!transform.parent.has_value()) return;

        Entity parentEntity = transform.parent.value();
        auto& parentTransform = transformArray.GetData(parentEntity);

        auto it = std::find(parentTransform.children.begin(), parentTransform.children.end(), entity);
        if (it == parentTransform.children.end()) return;

        parentTransform.children.erase(it);
        newIndex = std::clamp(newIndex, 0, static_cast<int>(parentTransform.children.size()));
        parentTransform.children.insert(parentTransform.children.begin() + newIndex, entity);

        // Also update global hierarchy order
        auto hierIt = std::find(aHierarchyOrder.begin(), aHierarchyOrder.end(), entity);
        if (hierIt != aHierarchyOrder.end())
        {
            aHierarchyOrder.erase(hierIt);

            // Find insertion point in hierarchy order based on new sibling position
            auto insertPos = aHierarchyOrder.end();
            if (newIndex == 0)
            {
                // First child: insert right after parent
                auto parentIt = std::find(aHierarchyOrder.begin(), aHierarchyOrder.end(), parentEntity);
                if (parentIt != aHierarchyOrder.end())
                    insertPos = parentIt + 1;
            }
            else
            {
                // Insert after the previous sibling and all its descendants
                Entity prevSibling = parentTransform.children[newIndex - 1];
                auto siblingIt = std::find(aHierarchyOrder.begin(), aHierarchyOrder.end(), prevSibling);
                if (siblingIt != aHierarchyOrder.end())
                {
                    insertPos = siblingIt + 1;
                    // Skip past any descendants of the previous sibling
                    while (insertPos != aHierarchyOrder.end())
                    {
                        bool isDescendant = false;
                        if (transformArray.Has(*insertPos))
                        {
                            auto p = transformArray.GetData(*insertPos).parent;
                            while (p.has_value())
                            {
                                if (p.value() == prevSibling) { isDescendant = true; break; }
                                if (transformArray.Has(p.value()))
                                    p = transformArray.GetData(p.value()).parent;
                                else
                                    break;
                            }
                        }
                        if (!isDescendant) break;
                        ++insertPos;
                    }
                }
            }

            aHierarchyOrder.insert(insertPos, entity);
        }
    }

    // Move one position up
    void Coordinator::MoveEntityUp(Entity entity) {
        auto& transformArray = GetComponentArray<Transform>();
        auto& transform = transformArray.GetData(entity);

        // If entity has a parent, move within parent's children
        if (transform.parent.has_value()) {
            Entity parentEntity = transform.parent.value();
            auto& parentTransform = transformArray.GetData(parentEntity);

            auto it = std::find(parentTransform.children.begin(), parentTransform.children.end(), entity);
            if (it != parentTransform.children.end() && it != parentTransform.children.begin()) {
                std::iter_swap(it, it - 1);
            }
        }

        // Also update global hierarchy order
        auto it = std::find(aHierarchyOrder.begin(), aHierarchyOrder.end(), entity);
        if (it != aHierarchyOrder.end() && it != aHierarchyOrder.begin()) {
            std::iter_swap(it, it - 1);
        }
    }

    // Move one position down
    void Coordinator::MoveEntityDown(Entity entity) {
        auto& transformArray = GetComponentArray<Transform>();
        auto& transform = transformArray.GetData(entity);

        // If entity has a parent, move within parent's children
        if (transform.parent.has_value()) {
            Entity parentEntity = transform.parent.value();
            auto& parentTransform = transformArray.GetData(parentEntity);

            auto it = std::find(parentTransform.children.begin(), parentTransform.children.end(), entity);
            if (it != parentTransform.children.end() && it + 1 != parentTransform.children.end()) {
                std::iter_swap(it, it + 1);
            }
        }

        // Also update global hierarchy order
        auto it = std::find(aHierarchyOrder.begin(), aHierarchyOrder.end(), entity);
        if (it != aHierarchyOrder.end() && it + 1 != aHierarchyOrder.end()) {
            std::iter_swap(it, it + 1);
        }
    }

    int Coordinator::GetHierarchyIndex(Entity entity) const
    {
        for (int i = 0; i < aHierarchyOrder.size(); i++)
        {
            if (entity == aHierarchyOrder[i]) return i;
        }
        return -1;
    }

    // Get hierarchy order for rendering inspector
    const std::vector<Entity>& Coordinator::GetHierarchyOrder() const {
        return aHierarchyOrder;
    }
}
