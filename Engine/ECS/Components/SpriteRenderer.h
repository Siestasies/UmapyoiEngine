/*!
\file   SpriteRenderer.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines sprite rendering component that references texture assets by name with flip flags for mirroring.

Stores texture name string for serialization and cached Texture pointer (managed by ResourcesManager) for runtime rendering.
Supports horizontal and vertical sprite flipping through flipX/flipY boolean flags.
Serializes only texture name (not pointer) to JSON for persistent storage. Texture pointer is resolved at runtime
by RenderingSystem through ResourcesManager lookup. Used by RenderingSystem for batched sprite drawing.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "../../Math/Math.h"
#include "../Systems/ResourcesTypes.hpp"

namespace Uma_ECS
{
    // currently in 2d
    struct SpriteRenderer
    {
        // pointer pointing to the texture in resources manager
        std::string textureName{};
        bool flipX{};
        bool flipY{};
        Uma_Engine::Texture* texture = nullptr;

        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const //override
        {
            value.SetObject();

            // Save texture name (not the pointer)
            value.AddMember("textureName",
                rapidjson::Value(textureName.c_str(), allocator),
                allocator);

            value.AddMember("flipX", flipX, allocator);
            value.AddMember("flipY", flipY, allocator);
        }

        // Deserialize from JSON
        void Deserialize(const rapidjson::Value& value) //override
        {
            textureName = value["textureName"].GetString();
            flipX = value["flipX"].GetBool();
            flipY = value["flipY"].GetBool();
        }
    };
}