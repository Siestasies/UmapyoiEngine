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
#include "Core/Types.hpp"

namespace Uma_ECS
{

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
        Vec2 size{};
        Vec2 offset{};
        ColliderPurpose purpose = ColliderPurpose::Physics;
        LayerMask layer = CL_NONE;
        LayerMask colliderMask = CL_NONE;
        bool isActive = true;
        bool autoFitToSprite = false;  // Add this per-shape flag
    };

    struct Collider
    {
        std::vector<ColliderShape> shapes;

        LayerMask defaultLayer = CL_DEFAULT;
        LayerMask defaultMask = CL_ALL;
        bool showBBox = false;

        // Runtime data
        std::vector<BoundingBox> bounds;

        // Constructor with default shape
        Collider()
        {
            shapes.push_back(ColliderShape{
                .size = Vec2(1.0f, 1.0f),
                .offset = Vec2(0.0f, 0.0f),
                .purpose = ColliderPurpose::Physics,
                .autoFitToSprite = true  // Primary shape auto-fits by default
                });
            bounds.resize(1);
        }

        inline LayerMask GetEffectiveLayer(size_t index) const
        {
            if (index >= shapes.size()) return defaultLayer;
            return shapes[index].layer != CL_NONE ? shapes[index].layer : defaultLayer;
        }

        inline LayerMask GetEffectiveMask(size_t index) const
        {
            if (index >= shapes.size()) return defaultMask;
            return shapes[index].colliderMask != CL_NONE ? shapes[index].colliderMask : defaultMask;
        }

        // Direct access helpers
        inline ColliderShape& GetPrimaryShape() { return shapes[0]; }
        inline const ColliderShape& GetPrimaryShape() const { return shapes[0]; }

        inline BoundingBox& GetPrimaryBounds() { return bounds[0]; }
        inline const BoundingBox& GetPrimaryBounds() const { return bounds[0]; }

        // Serialize/Deserialize updated below
        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const
        {
            value.SetObject();

            value.AddMember("defaultLayer", defaultLayer, allocator);
            value.AddMember("defaultMask", defaultMask, allocator);
            value.AddMember("showBBox", showBBox, allocator);

            rapidjson::Value shapesArray(rapidjson::kArrayType);
            for (const auto& shape : shapes)
            {
                rapidjson::Value shapeObj(rapidjson::kObjectType);

                rapidjson::Value offsetVal(rapidjson::kObjectType);
                offsetVal.AddMember("x", shape.offset.x, allocator);
                offsetVal.AddMember("y", shape.offset.y, allocator);
                shapeObj.AddMember("offset", offsetVal, allocator);

                rapidjson::Value sizeVal(rapidjson::kObjectType);
                sizeVal.AddMember("x", shape.size.x, allocator);
                sizeVal.AddMember("y", shape.size.y, allocator);
                shapeObj.AddMember("size", sizeVal, allocator);

                shapeObj.AddMember("purpose", static_cast<int>(shape.purpose), allocator);
                shapeObj.AddMember("layer", shape.layer, allocator);
                shapeObj.AddMember("colliderMask", shape.colliderMask, allocator);
                shapeObj.AddMember("isActive", shape.isActive, allocator);
                shapeObj.AddMember("autoFitToSprite", shape.autoFitToSprite, allocator);

                shapesArray.PushBack(shapeObj, allocator);
            }
            value.AddMember("shapes", shapesArray, allocator);
        }

        void Deserialize(const rapidjson::Value& value)
        {
            if (value.HasMember("defaultLayer"))
                defaultLayer = value["defaultLayer"].GetUint();

            if (value.HasMember("defaultMask"))
                defaultMask = value["defaultMask"].GetUint();

            if (value.HasMember("showBBox"))
                showBBox = value["showBBox"].GetBool();

            if (value.HasMember("shapes") && value["shapes"].IsArray())
            {
                const auto& shapesArray = value["shapes"];
                shapes.clear();
                shapes.reserve(shapesArray.Size());

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

                    if (shapeVal.HasMember("autoFitToSprite"))
                        shape.autoFitToSprite = shapeVal["autoFitToSprite"].GetBool();

                    shapes.push_back(shape);
                }

                if (shapes.empty())
                {
                    shapes.push_back(ColliderShape{ .autoFitToSprite = true });
                }

                bounds.resize(shapes.size());
            }
        }
    };
}