/*!
\file   Collider.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines collision detection component using axis-aligned bounding boxes (AABB) with layer-based filtering.

Uses bitmask enums (CollisionLayer) for efficient collision layer management supporting up to 32 layers.
Stores min/max bounds for broadphase tests and layer/colliderMask bitmasks for filtering which layers can interact.
Includes predefined layers: DEFAULT, PLAYER, ENEMY, WALL, PROJECTILE, PICKUP with CL_ALL wildcard.
Provides JSON serialization for bounding box coordinates and layer masks. BoundingBox struct stores Vec2 min/max extents.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "Math/Math.h"

namespace Uma_ECS
{
    using LayerMask = unsigned int;

    // using bitmask (raw integers), faster n cheaper approach 
    // compared to using bitset<N> like what I did in ECS component
    // ...
    enum CollisionLayer : LayerMask
    {
        CL_NONE = 0,
        CL_DEFAULT = 1 << 0,
        CL_PLAYER = 1 << 1,
        CL_ENEMY = 1 << 2,
        CL_WALL = 1 << 3,
        CL_PROJECTILE = 1 << 4,
        CL_PICKUP = 1 << 5,
        CL_ALL = 0xFFFFFFFF
    };

    struct BoundingBox
    {
        Vec2 min{};
        Vec2 max{};
    };

    // currently in 2d
    struct Collider
    {
        // Local collider size in world units (like Unity's Box Collider 2D size)
        Vec2 size = Vec2(1.0f, 1.0f);

        // Offset from entity position (useful for asymmetric colliders)
        Vec2 offset = Vec2(0.0f, 0.0f);

        // Auto-fit to sprite's render size (overrides manual size if true)
        bool autoFitToSprite = false;

        // Layer system
        LayerMask layer = CL_NONE;
        LayerMask colliderMask = CL_NONE;

        // Debug visualization
        bool showBBox = false;

        // === RUNTIME DATA (Not Serialized) ===

        // World-space bounding box, updated each frame by collision system
        BoundingBox boundingBox{};

        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const
        {
            value.SetObject();

            // Serialize size
            rapidjson::Value sizeVal(rapidjson::kObjectType);
            sizeVal.AddMember("x", size.x, allocator);
            sizeVal.AddMember("y", size.y, allocator);
            value.AddMember("size", sizeVal, allocator);

            // Serialize offset
            rapidjson::Value offsetVal(rapidjson::kObjectType);
            offsetVal.AddMember("x", offset.x, allocator);
            offsetVal.AddMember("y", offset.y, allocator);
            value.AddMember("offset", offsetVal, allocator);

            // Serialize auto-fit flag
            value.AddMember("autoFitToSprite", autoFitToSprite, allocator);

            // Serialize layer settings
            value.AddMember("layer", layer, allocator);
            value.AddMember("colliderMask", colliderMask, allocator);
            value.AddMember("showBBox", showBBox, allocator);

            // Don't serialize boundingBox - it's runtime data
        }

        void Deserialize(const rapidjson::Value& value)
        {
            // Check if each key exists before accessing
            if (value.HasMember("size") && value["size"].IsObject())
            {
                const auto& sizeObj = value["size"];
                if (sizeObj.HasMember("x") && sizeObj["x"].IsNumber())
                    size.x = sizeObj["x"].GetFloat();
                if (sizeObj.HasMember("y") && sizeObj["y"].IsNumber())
                    size.y = sizeObj["y"].GetFloat();
            }

            if (value.HasMember("offset") && value["offset"].IsObject())
            {
                const auto& offsetObj = value["offset"];
                if (offsetObj.HasMember("x") && offsetObj["x"].IsNumber())
                    offset.x = offsetObj["x"].GetFloat();
                if (offsetObj.HasMember("y") && offsetObj["y"].IsNumber())
                    offset.y = offsetObj["y"].GetFloat();
            }

            if (value.HasMember("autoFitToSprite") && value["autoFitToSprite"].IsBool())
                autoFitToSprite = value["autoFitToSprite"].GetBool();

            if (value.HasMember("layer") && value["layer"].IsUint())
                layer = value["layer"].GetUint();

            if (value.HasMember("colliderMask") && value["colliderMask"].IsUint())
                colliderMask = value["colliderMask"].GetUint();

            if (value.HasMember("showBBox") && value["showBBox"].IsBool())
                showBBox = value["showBBox"].GetBool();
        }
    };
}