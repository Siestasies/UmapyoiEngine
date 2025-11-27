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
#include <filesystem>
#include <unordered_set>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>   // pretty JSON output

// Forward declarations for prefab serialization
namespace Uma_ECS
{
    class Coordinator;
}

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

            // Serialize prefab entities
            rapidjson::Value prefabSection(rapidjson::kObjectType);
            coordinator->SerializePrefab(entity, prefabSection, allocator);
            doc.AddMember(rapidjson::StringRef("Prefab"), prefabSection, allocator);

            // Mark all entities in the hierarchy with Prefab component
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
                    // Update existing Prefab component
                    auto& prefabComp = coordPtr->GetComponent<Uma_ECS::Prefab>(e);
                    prefabComp.prefabPath = filename;
                    prefabComp.isRoot = (i == 0);
                }
            }

            // Collect and serialize resources used by prefab
            if (resourcesManager)
            {
                // Need to cast to Coordinator to access CollectPrefabResources
                // This is a bit hacky but works within the current architecture
                Uma_ECS::Coordinator* coordPtr = static_cast<Uma_ECS::Coordinator*>(coordinator);
                Uma_ECS::Coordinator::PrefabResources resources;
                coordPtr->CollectPrefabResources(entity, resources);

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

        void loadPrefab(const std::string& filename)
        {
            std::ifstream ifs(filename);
            rapidjson::IStreamWrapper isw(ifs);
            rapidjson::Document doc;
            doc.ParseStream(isw);
            ifs.close();

            // First, load resources if available
            for (auto* s : serializers)
            {
                if (s->GetSerializerName() == "resources_manager" && doc.HasMember("resources"))
                {
                    s->DeserializePrefab(doc["resources"]);
                }
            }

            // Then, load prefab entities
            for (auto* s : serializers)
            {
                if (s->GetSerializerName() == "coordinator" && doc.HasMember("Prefab"))
                {
                    s->DeserializePrefab(doc["Prefab"]);
                }
            }

            std::string log;
            std::stringstream ss(log);

            ss << "Loaded from file : " << filename;

            Debugger::Log(WarningLevel::eCritical, ss.str());
        }

        void ShutDown()
        {
            serializers.clear();
        }
    };
}