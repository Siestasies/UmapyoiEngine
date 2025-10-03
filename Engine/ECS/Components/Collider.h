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