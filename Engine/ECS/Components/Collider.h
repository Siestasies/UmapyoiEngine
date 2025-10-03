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
        BoundingBox boundingBox{};
        LayerMask layer = CL_NONE; // the layer it is in
        LayerMask colliderMask = CL_NONE; // the layer it can collide with
        bool showBBox = false;

        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const //override
        {
            value.SetObject();

            // Bounding box
            rapidjson::Value bbox(rapidjson::kObjectType);

            rapidjson::Value minVal(rapidjson::kObjectType);
            minVal.AddMember("x", boundingBox.min.x, allocator);
            minVal.AddMember("y", boundingBox.min.y, allocator);

            rapidjson::Value maxVal(rapidjson::kObjectType);
            maxVal.AddMember("x", boundingBox.max.x, allocator);
            maxVal.AddMember("y", boundingBox.max.y, allocator);

            bbox.AddMember("min", minVal, allocator);
            bbox.AddMember("max", maxVal, allocator);

            value.AddMember("boundingBox", bbox, allocator);

            // Layer + mask as raw integers (bitmasks)
            value.AddMember("layer", layer, allocator);
            value.AddMember("colliderMask", colliderMask, allocator);
        }

        // Deserialize from JSON
        void Deserialize(const rapidjson::Value& value) //override
        {
            const auto& bbox = value["boundingBox"];

            boundingBox.min.x = bbox["min"]["x"].GetFloat();
            boundingBox.min.y = bbox["min"]["y"].GetFloat();

            boundingBox.max.x = bbox["max"]["x"].GetFloat();
            boundingBox.max.y = bbox["max"]["y"].GetFloat();

            layer = value["layer"].GetUint();
            colliderMask = value["colliderMask"].GetUint();
        }
    };
}