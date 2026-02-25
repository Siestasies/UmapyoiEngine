/*!
\file   GameSerializer.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Implements multi-system serialization coordinator that aggregates ISerializer implementations into unified JSON documents.

Maintains registry of serializers and orchestrates section-based save/load operations with each serializer contributing
its own named JSON object (e.g., "entities", "resources") to the document hierarchy.
Uses RapidJSON PrettyWriter for human-readable output with 3 decimal place precision for floating-point values.
Provides file I/O operations with debug logging via Uma_Engine::Debugger. Includes commented legacy code showing
previous monolithic entity serialization approach before ISerializer abstraction.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "Core/BaseSerializer.h"
#include "Debugging/Debugger.hpp"

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <unordered_set>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>   // pretty JSON output

// Full Coordinator definition needed for template methods (HasComponent/GetComponent)
#include "ECS/Core/Coordinator.hpp"
#include "ECS/Components/Prefab.h"

namespace Uma_Engine
{
    using Entity = unsigned int;
    class ResourcesManager;

    class GameSerializer
    {
    private:
        std::vector<ISerializer*> serializers;

    public:

        void Register(ISerializer* s) 
        {
            serializers.push_back(s);
        }

        void save(const std::string& filename)
        {
            rapidjson::Document doc;
            doc.SetObject();
            auto& allocator = doc.GetAllocator();

            for (auto* s : serializers) 
            {
                rapidjson::Value section(rapidjson::kObjectType);
                s->Serialize(section, allocator);
                doc.AddMember(rapidjson::StringRef(s->GetSectionName()), section, allocator);
            }

            // write to file
            rapidjson::StringBuffer buffer;
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
            //writer.SetIndent(' ', 4); // 4 spaces per indent
            writer.SetMaxDecimalPlaces(3);
            doc.Accept(writer);
            std::ofstream ofs(filename);
            ofs << buffer.GetString();
            ofs.close();

            std::string log;
            std::stringstream ss(log);
            ss << "Saved to file : " << filename;
            Debugger::Log(WarningLevel::eCritical, ss.str());
        }

        void load(const std::string& filename)
        {
            // Use std::filesystem to explicitly check if the file exists
            if (!std::filesystem::exists(filename))
            {
                std::string log;
                std::stringstream ss(log);
                ss << "File does not exist: " << filename;
                Debugger::Log(WarningLevel::eCritical, ss.str());

                // If the file doesn't exist, create an empty one
                std::ofstream ofs(filename);  // Create the file if it doesn't exist
                ofs.close();
                // copy file from test_base file
                std::filesystem::copy_file(
                    "Assets/Scenes/test_base.scn",
                    filename,
                    std::filesystem::copy_options::overwrite_existing
                );

                std::stringstream createLog;
                createLog << "Created an empty file: " << filename;
                Debugger::Log(WarningLevel::eCritical, createLog.str());
            }

            std::ifstream ifs(filename);
            rapidjson::IStreamWrapper isw(ifs);
            rapidjson::Document doc;
            doc.ParseStream(isw);
            ifs.close();

            // Pre-load resources from prefab files
            if (doc.HasMember("entities") && doc["entities"].IsArray())
            {
                std::unordered_set<std::string> loadedPrefabs;

                for (const auto& entityVal : doc["entities"].GetArray())
                {
                    if (entityVal.HasMember("isPrefab") && entityVal["isPrefab"].GetBool() &&
                        entityVal.HasMember("prefabPath"))
                    {
                        std::string prefabPath = entityVal["prefabPath"].GetString();

                        // Only load each prefab's resources once
                        if (loadedPrefabs.find(prefabPath) == loadedPrefabs.end())
                        {
                            loadedPrefabs.insert(prefabPath);

                            // Load prefab file to get resources
                            if (std::filesystem::exists(prefabPath))
                            {
                                std::ifstream prefabIfs(prefabPath);
                                rapidjson::IStreamWrapper prefabIsw(prefabIfs);
                                rapidjson::Document prefabDoc;
                                prefabDoc.ParseStream(prefabIsw);
                                prefabIfs.close();

                                // Load resources from prefab
                                if (prefabDoc.HasMember("resources"))
                                {
                                    for (auto* s : serializers)
                                    {
                                        if (s->GetSectionName() == std::string("resources"))
                                        {
                                            s->Deserialize(prefabDoc["resources"]);
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Load scene resources and entities
            for (auto* s : serializers)
            {
                if (doc.HasMember(s->GetSectionName()))
                {
                    s->Deserialize(doc[s->GetSectionName()]);
                }
            }

            std::string log;
            std::stringstream ss(log);

            ss << "Loaded from file : " << filename;

            Debugger::Log(WarningLevel::eCritical, ss.str());
        }

        void savePrefab(Entity entity, const std::string& filename)
        {
            rapidjson::Document doc;
            doc.SetObject();
            auto& allocator = doc.GetAllocator();

            // Find Coordinator and ResourcesManager
            ISerializer* coordinator = nullptr;
            ISerializer* resourcesManager = nullptr;

            for (auto* s : serializers)
            {
                if (s->GetSerializerName() == "coordinator")
                    coordinator = s;
                else if (s->GetSerializerName() == "resources_manager")
                    resourcesManager = s;
            }

            if (!coordinator)
            {
                Debugger::Log(WarningLevel::eError, "Coordinator not found in serializers");
                return;
            }

            // Mark all entities in the hierarchy with Prefab component BEFORE serialization
            // so the Prefab component data is correctly written into the file.
            Uma_ECS::Coordinator* coordPtr = static_cast<Uma_ECS::Coordinator*>(coordinator);
            std::vector<Uma_ECS::Entity> hierarchyEntities;
            coordPtr->CollectHierarchy(entity, hierarchyEntities);

            for (size_t i = 0; i < hierarchyEntities.size(); ++i)
            {
                Uma_ECS::Entity e = hierarchyEntities[i];

                if (!coordPtr->HasComponent<Uma_ECS::Prefab>(e))
                {
                    Uma_ECS::Prefab prefabComp;
                    prefabComp.prefabPath = filename;
                    prefabComp.isRoot = (i == 0);  // First entity is root
                    coordPtr->AddComponent(e, prefabComp);
                }
                else
                {
                    auto& prefabComp = coordPtr->GetComponent<Uma_ECS::Prefab>(e);
                    if (i == 0)
                    {
                        // Root entity: always update to the target prefab file
                        prefabComp.prefabPath = filename;
                        prefabComp.isRoot = true;
                    }
                    else if (prefabComp.prefabPath == filename)
                    {
                        // Already belongs to this prefab (re-save) - ensure isRoot is correct
                        prefabComp.isRoot = false;
                    }
                    // else: nested prefab instance pointing to a different file - preserve it
                }
            }

            // Serialize prefab entities (Prefab components are now correctly set up)
            rapidjson::Value prefabSection(rapidjson::kObjectType);
            coordinator->SerializePrefab(entity, prefabSection, allocator);
            doc.AddMember(rapidjson::StringRef("Prefab"), prefabSection, allocator);

            // Collect and serialize resources used by prefab
            if (resourcesManager)
            {
                // Need to cast to Coordinator to access CollectPrefabResources
                // This is a bit hacky but works within the current architecture
                Uma_ECS::Coordinator* newCoord = static_cast<Uma_ECS::Coordinator*>(coordinator);
                Uma_ECS::Coordinator::PrefabResources resources;
                newCoord->CollectPrefabResources(entity, resources);

                // Serialize resources
                Uma_Engine::ResourcesManager* resMgrPtr = static_cast<Uma_Engine::ResourcesManager*>(resourcesManager);
                rapidjson::Value resourcesSection(rapidjson::kObjectType);
                resMgrPtr->SerializeSpecificResources(
                    resources.textures,
                    resources.sounds,
                    resources.fonts,
                    resourcesSection,
                    allocator
                );
                doc.AddMember(rapidjson::StringRef("resources"), resourcesSection, allocator);
            }

            // write to file
            rapidjson::StringBuffer buffer;
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
            //writer.SetIndent(' ', 4); // 4 spaces per indent
            writer.SetMaxDecimalPlaces(3);
            doc.Accept(writer);
            std::ofstream ofs(filename);
            ofs << buffer.GetString();
            ofs.close();

            std::string log;
            std::stringstream ss(log);
            ss << "Saved to file : " << filename;
            Debugger::Log(WarningLevel::eCritical, ss.str());
        }

        // Returns the root world Entity of the loaded prefab.
        // loadingStack is used internally for cycle detection across recursive calls.
        Uma_ECS::Entity loadPrefab(const std::string& filename,
            std::unordered_set<std::string>* loadingStack = nullptr)
        {
            // Cycle detection: prevent A.prefab -> B.prefab -> A.prefab infinite loops
            std::unordered_set<std::string> localStack;
            if (!loadingStack)
                loadingStack = &localStack;

            if (loadingStack->count(filename))
            {
                Debugger::Log(WarningLevel::eError,
                    "Circular prefab dependency detected: " + filename);
                return static_cast<Uma_ECS::Entity>(-1);
            }
            loadingStack->insert(filename);

            std::ifstream ifs(filename);
            if (!ifs.is_open())
            {
                Debugger::Log(WarningLevel::eError, "Failed to open prefab file: " + filename);
                loadingStack->erase(filename);
                return static_cast<Uma_ECS::Entity>(-1);
            }

            rapidjson::IStreamWrapper isw(ifs);
            rapidjson::Document doc;
            doc.ParseStream(isw);
            ifs.close();

            // Load resources first
            for (auto* s : serializers)
            {
                if (s->GetSerializerName() == "resources_manager" && doc.HasMember("resources"))
                    s->DeserializePrefab(doc["resources"]);
            }

            // Load all prefab entities (embedded nested data is loaded as-is for now)
            Uma_ECS::Entity outerRoot = static_cast<Uma_ECS::Entity>(-1);
            Uma_ECS::Coordinator* coordPtr = nullptr;

            for (auto* s : serializers)
            {
                if (s->GetSerializerName() == "coordinator" && doc.HasMember("Prefab"))
                {
                    outerRoot = s->DeserializePrefab(doc["Prefab"]);
                    coordPtr = static_cast<Uma_ECS::Coordinator*>(s);
                }
            }

            if (outerRoot == static_cast<Uma_ECS::Entity>(-1) || !coordPtr)
            {
                Debugger::Log(WarningLevel::eCritical, "Loaded from file : " + filename);
                loadingStack->erase(filename);
                return outerRoot;
            }

            // --- Nested prefab replacement ---
            // Scan the loaded hierarchy for entities that are roots of a DIFFERENT prefab.
            // Only collect the outermost such roots; skip entities already inside another
            // nested sub-hierarchy so we don't double-process deep nesting.

            struct NestedInfo
            {
                Uma_ECS::Entity embeddedEntity;
                std::string     nestedPath;
                std::optional<Uma_ECS::Entity> parentEntity;
                Uma_Math::Vec2  position;
                Uma_Math::Vec2  rotation;
                Uma_Math::Vec2  scale;
                std::string     name;
            };

            std::vector<Uma_ECS::Entity> outerHierarchy;
            coordPtr->CollectHierarchy(outerRoot, outerHierarchy);

            std::vector<NestedInfo>              nestedPrefabs;
            std::unordered_set<Uma_ECS::Entity>  handledEntities; // tracks sub-hierarchies already claimed

            for (Uma_ECS::Entity e : outerHierarchy)
            {
                if (e == outerRoot) continue;
                if (handledEntities.count(e)) continue; // inside an already-found nested prefab

                if (!coordPtr->HasComponent<Uma_ECS::Prefab>(e)) continue;

                const auto& prefabComp = coordPtr->GetComponent<Uma_ECS::Prefab>(e);
                if (!prefabComp.isRoot)            continue; // not a nested root
                if (prefabComp.prefabPath == filename) continue; // same file, not nested

                // Found a top-level nested prefab root
                NestedInfo info;
                info.embeddedEntity = e;
                info.nestedPath     = prefabComp.prefabPath;

                const auto& tf  = coordPtr->GetComponent<Uma_ECS::Transform>(e);
                info.parentEntity = tf.parent;
                info.position     = tf.position;
                info.rotation     = tf.rotation;
                info.scale        = tf.scale;
                info.name         = tf.name;

                nestedPrefabs.push_back(info);

                // Mark the entire sub-hierarchy so we don't re-visit it
                std::vector<Uma_ECS::Entity> sub;
                coordPtr->CollectHierarchy(e, sub);
                for (Uma_ECS::Entity ne : sub)
                    handledEntities.insert(ne);
            }

            // Destroy every embedded nested prefab entity (queued; not immediate)
            for (const auto& info : nestedPrefabs)
                coordPtr->DestroyEntityAndChildren(info.embeddedEntity);

            // Flush the deletion queue so IDs are freed before we load replacements
            coordPtr->ProcessDeletionQueue();

            // Recursively load each nested prefab from its own file and re-attach
            for (const auto& info : nestedPrefabs)
            {
                Uma_ECS::Entity nestedRoot = loadPrefab(info.nestedPath, loadingStack);
                if (nestedRoot == static_cast<Uma_ECS::Entity>(-1)) continue;

                // Restore the transform context that was stored in the outer prefab
                auto& tf    = coordPtr->GetComponent<Uma_ECS::Transform>(nestedRoot);
                tf.position = info.position;
                tf.rotation = info.rotation;
                tf.scale    = info.scale;
                tf.name     = info.name;
                tf.isDirty  = true;

                // Re-attach to the correct parent in the outer hierarchy
                if (info.parentEntity.has_value())
                    coordPtr->SetParent(nestedRoot, info.parentEntity.value());
            }

            Debugger::Log(WarningLevel::eCritical, "Loaded from file : " + filename);
            loadingStack->erase(filename);
            return outerRoot;
        }

        void ShutDown()
        {
            serializers.clear();
        }
    };
}