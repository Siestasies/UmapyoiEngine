/*!
\file   Prefab.h
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines prefab component that marks entities as prefab instances and stores reference to the prefab asset.

Stores the file path to the original prefab asset and enables the scene serialization system to save
prefab instances as references with property overrides instead of full entity data. Used by the
Coordinator during scene serialization/deserialization to implement prefab asset management.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include <string>
#include <rapidjson/document.h>

namespace Uma_ECS
{
    struct Prefab
    {
        // Path to the prefab file (e.g., "Assets/Prefabs/platform.json")
        std::string prefabPath{};

        // Optional: Track if this is the root entity of the prefab instance
        // (useful for multi-entity prefab hierarchies)
        bool isRoot = true;

        /*!
        \brief Serialize prefab data to JSON, including the prefab file path and root flag.
        \param value Output JSON value to populate.
        \param allocator RapidJSON allocator for creating new values.
        */
        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const
        {
            value.SetObject();

            value.AddMember("prefabPath",
                rapidjson::Value(prefabPath.c_str(), allocator),
                allocator);

            value.AddMember("isRoot", isRoot, allocator);
        }

        /*!
        \brief Deserialize prefab data from JSON, restoring the prefab file path and root flag.
        \param value JSON value containing serialized prefab data.
        */
        void Deserialize(const rapidjson::Value& value)
        {
            if (value.HasMember("prefabPath"))
            {
                prefabPath = value["prefabPath"].GetString();
            }

            if (value.HasMember("isRoot"))
            {
                isRoot = value["isRoot"].GetBool();
            }
        }
    };
}
