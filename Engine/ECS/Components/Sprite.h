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
#include "Core/Types.hpp"

namespace Uma_ECS
{
    // currently in 2d
    struct Sprite
    {
        // pointer pointing to the texture in resources manager
        std::string textureName{};
        LayerMask renderLayer = RL_NONE;
        bool flipX{ false };
        bool flipY{ false };
        bool autoFlip{ true };
        bool UseNativeSize{};
        std::shared_ptr<Uma_Engine::Texture> texture = nullptr;

        Vec3 tintColor = Vec3(1.0f, 1.0f, 1.0f);    // RGB multiplier
        float alpha = 1.0f;                         // Opacity
        Vec2 spriteSheetGrid{ 1.0f, 1.0f };         // Total columns and rows (default = full texture)
        Vec2 spriteCell{ 0.0f, 0.0f };              // Which cell to render (col, row)

        void GetUVs(Vec2& uvOffset, Vec2& uvSize) const
        {
            // Calculate size of one cell in UV space
            uvSize.x = 1.0f / spriteSheetGrid.x;
            uvSize.y = 1.0f / spriteSheetGrid.y;

            // Calculate offset for the specific cell
            uvOffset.x = spriteCell.x * uvSize.x;
            uvOffset.y = spriteCell.y * uvSize.y;
        }

        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const //override
        {
            value.SetObject();

            // Save texture name (not the pointer)
            value.AddMember("textureName",
                rapidjson::Value(textureName.c_str(), allocator),
                allocator);

            value.AddMember("layer", renderLayer, allocator);

            value.AddMember("flipX", flipX, allocator);
            value.AddMember("flipY", flipY, allocator);
            value.AddMember("autoFlip", autoFlip, allocator);
            value.AddMember("Native", UseNativeSize, allocator);

            value.AddMember("gridX", spriteSheetGrid.x, allocator);
            value.AddMember("gridY", spriteSheetGrid.y, allocator);
            value.AddMember("cellX", spriteCell.x, allocator);
            value.AddMember("cellY", spriteCell.y, allocator);

            rapidjson::Value tintArray(rapidjson::kArrayType);
            tintArray.PushBack(tintColor.x, allocator);
            tintArray.PushBack(tintColor.y, allocator);
            tintArray.PushBack(tintColor.z, allocator);
            value.AddMember("tintColor", tintArray, allocator);
            value.AddMember("alpha", alpha, allocator);
        }

        // Deserialize from JSON
        void Deserialize(const rapidjson::Value& value) //override
        {
            textureName = value["textureName"].GetString();

            if (value.HasMember("layer"))
            {
                renderLayer = value["layer"].GetUint();
            }

            if (value.HasMember("flipX")) flipX = value["flipX"].GetBool();
            if (value.HasMember("flipY")) flipY = value["flipY"].GetBool();
            if (value.HasMember("autoFlip")) autoFlip = value["autoFlip"].GetBool();
            UseNativeSize = value["Native"].GetBool();

            spriteSheetGrid.x = value.HasMember("gridX") ? value["gridX"].GetFloat() : 1.0f;
            spriteSheetGrid.y = value.HasMember("gridY") ? value["gridY"].GetFloat() : 1.0f;
            spriteCell.x = value.HasMember("cellX") ? value["cellX"].GetFloat() : 0.0f;
            spriteCell.y = value.HasMember("cellY") ? value["cellY"].GetFloat() : 0.0f;

            if (value.HasMember("tintColor") && value["tintColor"].IsArray())
            {
                const auto& tintArray = value["tintColor"].GetArray();
                if (tintArray.Size() >= 3)
                {
                    tintColor.x = tintArray[0].GetFloat();
                    tintColor.y = tintArray[1].GetFloat();
                    tintColor.z = tintArray[2].GetFloat();
                }
            }

            if (value.HasMember("alpha"))
            {
                alpha = value["alpha"].GetFloat();
            }
        }
    };
}