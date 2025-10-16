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

    enum class ColliderPurpose
    {
        Physics = 0,      // Entity-to-entity (damage, interaction)
        Environment = 1,  // Entity-to-wall (movement blocking)
        Trigger = 2       // Non-blocking detection zones
    };

    struct ColliderShape
    {
        Vec2 offset{};
        Vec2 size{};
        ColliderPurpose purpose = ColliderPurpose::Physics;
        LayerMask layer = CL_NONE;          // Can have different layers per shape
        LayerMask colliderMask = CL_NONE;
        bool isActive = true;

        /*BoundingBox GetWorldBounds(const Vec2& entityPos, const Vec2& entityScale) const
        {
            Vec2 center = entityPos + offset;
            Vec2 halfSize = (size * entityScale) * 0.5f;
            return BoundingBox{
                .min = center - halfSize,
                .max = center + halfSize
            };
        }*/
    };

    // currently in 2d
    struct Collider
    {
        // Primary shape (backward compatible) - defaults to Physics purpose
        Vec2 size = Vec2(1.0f, 1.0f);
        Vec2 offset = Vec2(0.0f, 0.0f);
        bool autoFitToSprite = false;
        ColliderPurpose primaryPurpose = ColliderPurpose::Physics;

        // Additional shapes for compound colliders
        std::vector<ColliderShape> additionalShapes;

        // Default layer settings (used if shape doesn't override)
        LayerMask layer = CL_NONE;
        LayerMask colliderMask = CL_NONE;
        bool showBBox = false;

        // Runtime data
        BoundingBox boundingBox{};
        std::vector<BoundingBox> additionalBounds;

        // Helper to get all shapes with their purposes
        struct ShapeData
        {
            BoundingBox bounds;
            ColliderPurpose purpose;
            LayerMask layer;
            LayerMask mask;
            size_t index; // 0 = primary, 1+ = additional
        };

        std::vector<ShapeData> GetAllShapesWithPurpose() const
        {
            std::vector<ShapeData> shapes;
            shapes.reserve(1 + additionalShapes.size());

            // Primary shape
            shapes.push_back(ShapeData{
                .bounds = boundingBox,
                .purpose = primaryPurpose,
                .layer = layer,
                .mask = colliderMask,
                .index = 0
                });

            // Additional shapes
            for (size_t i = 0; i < additionalShapes.size(); ++i)
            {
                if (additionalShapes[i].isActive && i < additionalBounds.size())
                {
                    const auto& shape = additionalShapes[i];
                    shapes.push_back(ShapeData{
                        .bounds = additionalBounds[i],
                        .purpose = shape.purpose,
                        .layer = shape.layer != CL_NONE ? shape.layer : layer,
                        .mask = shape.colliderMask != CL_NONE ? shape.colliderMask : colliderMask,
                        .index = i + 1
                        });
                }
            }

            return shapes;
        }

        // Get shapes by purpose
        std::vector<BoundingBox> GetShapesByPurpose(ColliderPurpose purpose) const
        {
            std::vector<BoundingBox> result;

            if (primaryPurpose == purpose)
            {
                result.push_back(boundingBox);
            }

            for (size_t i = 0; i < additionalShapes.size(); ++i)
            {
                if (additionalShapes[i].isActive &&
                    additionalShapes[i].purpose == purpose &&
                    i < additionalBounds.size())
                {
                    result.push_back(additionalBounds[i]);
                }
            }

            return result;
        }

        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const
        {
            value.SetObject();

            // Existing serialization...
            rapidjson::Value sizeVal(rapidjson::kObjectType);
            sizeVal.AddMember("x", size.x, allocator);
            sizeVal.AddMember("y", size.y, allocator);
            value.AddMember("size", sizeVal, allocator);

            rapidjson::Value offsetVal(rapidjson::kObjectType);
            offsetVal.AddMember("x", offset.x, allocator);
            offsetVal.AddMember("y", offset.y, allocator);
            value.AddMember("offset", offsetVal, allocator);

            value.AddMember("autoFitToSprite", autoFitToSprite, allocator);
            value.AddMember("primaryPurpose", static_cast<int>(primaryPurpose), allocator);
            value.AddMember("layer", layer, allocator);
            value.AddMember("colliderMask", colliderMask, allocator);
            value.AddMember("showBBox", showBBox, allocator);

            // Serialize additional shapes
            if (!additionalShapes.empty())
            {
                rapidjson::Value shapesArray(rapidjson::kArrayType);
                for (const auto& shape : additionalShapes)
                {
                    rapidjson::Value shapeObj(rapidjson::kObjectType);

                    rapidjson::Value shapeOffsetVal(rapidjson::kObjectType);
                    shapeOffsetVal.AddMember("x", shape.offset.x, allocator);
                    shapeOffsetVal.AddMember("y", shape.offset.y, allocator);
                    shapeObj.AddMember("offset", shapeOffsetVal, allocator);

                    rapidjson::Value shapeSizeVal(rapidjson::kObjectType);
                    shapeSizeVal.AddMember("x", shape.size.x, allocator);
                    shapeSizeVal.AddMember("y", shape.size.y, allocator);
                    shapeObj.AddMember("size", shapeSizeVal, allocator);

                    shapeObj.AddMember("purpose", static_cast<int>(shape.purpose), allocator);
                    shapeObj.AddMember("layer", shape.layer, allocator);
                    shapeObj.AddMember("colliderMask", shape.colliderMask, allocator);
                    shapeObj.AddMember("isActive", shape.isActive, allocator);

                    shapesArray.PushBack(shapeObj, allocator);
                }
                value.AddMember("additionalShapes", shapesArray, allocator);
            }
        }

        void Deserialize(const rapidjson::Value& value)
        {
            // Existing deserialization...
            if (value.HasMember("size") && value["size"].IsObject())
            {
                const auto& sizeObj = value["size"];
                if (sizeObj.HasMember("x")) size.x = sizeObj["x"].GetFloat();
                if (sizeObj.HasMember("y")) size.y = sizeObj["y"].GetFloat();
            }

            if (value.HasMember("offset") && value["offset"].IsObject())
            {
                const auto& offsetObj = value["offset"];
                if (offsetObj.HasMember("x")) offset.x = offsetObj["x"].GetFloat();
                if (offsetObj.HasMember("y")) offset.y = offsetObj["y"].GetFloat();
            }

            if (value.HasMember("autoFitToSprite"))
                autoFitToSprite = value["autoFitToSprite"].GetBool();

            if (value.HasMember("primaryPurpose"))
                primaryPurpose = static_cast<ColliderPurpose>(value["primaryPurpose"].GetInt());

            if (value.HasMember("layer"))
                layer = value["layer"].GetUint();

            if (value.HasMember("colliderMask"))
                colliderMask = value["colliderMask"].GetUint();

            if (value.HasMember("showBBox"))
                showBBox = value["showBBox"].GetBool();

            // Deserialize additional shapes
            if (value.HasMember("additionalShapes") && value["additionalShapes"].IsArray())
            {
                const auto& shapesArray = value["additionalShapes"];
                additionalShapes.clear();
                additionalShapes.reserve(shapesArray.Size());

                for (const auto& shapeVal : shapesArray.GetArray())
                {
                    ColliderShape shape;

                    if (shapeVal.HasMember("offset"))
                    {
                        const auto& offsetObj = shapeVal["offset"];
                        shape.offset.x = offsetObj["x"].GetFloat();
                        shape.offset.y = offsetObj["y"].GetFloat();
                    }

                    if (shapeVal.HasMember("size"))
                    {
                        const auto& sizeObj = shapeVal["size"];
                        shape.size.x = sizeObj["x"].GetFloat();
                        shape.size.y = sizeObj["y"].GetFloat();
                    }

                    if (shapeVal.HasMember("purpose"))
                        shape.purpose = static_cast<ColliderPurpose>(shapeVal["purpose"].GetInt());

                    if (shapeVal.HasMember("layer"))
                        shape.layer = shapeVal["layer"].GetUint();

                    if (shapeVal.HasMember("colliderMask"))
                        shape.colliderMask = shapeVal["colliderMask"].GetUint();

                    if (shapeVal.HasMember("isActive"))
                        shape.isActive = shapeVal["isActive"].GetBool();

                    additionalShapes.push_back(shape);
                }
            }
        }
    };
}