/*!
\file   Transform.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines spatial transformation component for 2D entity positioning, rotation, and scaling.

Contains position, rotation (x=angle, y=angular velocity), scale as Vec2 values, and prevPos for swept collision detection.
prevPos is automatically set during deserialization and updated by PhysicsSystem for temporal coherence in collision queries.
Provides JSON serialization for position, rotation, and scale only (prevPos excluded as runtime-only data).
Core component used by nearly all systems for spatial queries and world-space transformations.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "../../Math/Math.h"
#include <optional>
//#include "Core/SerializationBase.h"

namespace Uma_ECS
{
    // currently in 2d
    struct Transform //: public SerializationBase
    {
        // name of the gameobject 
        std::string name{};

        // transform
        Vec2 position{};
        Vec2 rotation{};
        Vec2 scale{};
        Vec2 prevPos{}; // shdnt edit this value manually

        // gameobject grouping
        std::optional<Entity> parent;
        std::vector<Entity> children;

        // Cached world-space transforms (updated by TransformSystem)
        Vec2 worldPosition{};
        Vec2 worldScale{ 1.0f, 1.0f };
        float worldRotation{};
        Vec2 prevWorldPos{};

        bool isDirty = true;

        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const //override
        {
            value.SetObject();

            // name 
            value.AddMember("name",
                rapidjson::Value(name.c_str(), allocator),
                allocator);
            
            // Position
            rapidjson::Value pos(rapidjson::kObjectType);
            pos.AddMember("x", position.x, allocator);
            pos.AddMember("y", position.y, allocator);
            value.AddMember("position", pos, allocator);

            // Rotation
            rapidjson::Value rot(rapidjson::kObjectType);
            rot.AddMember("x", rotation.x, allocator);
            rot.AddMember("y", rotation.y, allocator);
            value.AddMember("rotation", rot, allocator);

            // Scale
            rapidjson::Value scl(rapidjson::kObjectType);
            scl.AddMember("x", scale.x, allocator);
            scl.AddMember("y", scale.y, allocator);
            value.AddMember("scale", scl, allocator);

            // prev pos is runtime object no need to save
            if (parent.has_value())
            {
                value.AddMember("parent", parent.value(), allocator);
            }
            else
            {
                value.AddMember("parent", -1, allocator);  // Use -1 for JSON compatibility
            }
        }

        // Deserialize from JSON
        void Deserialize(const rapidjson::Value& value) //override
        {
            if (value.HasMember("name"))
            {
                name = value["name"].GetString();

                // to make compatibility to old scene
                if (name.empty()) name = "Entity";
            }
            else
            {
                name = "Entity";
            }

            const auto& pos = value["position"];
            position.x = pos["x"].GetFloat();
            position.y = pos["y"].GetFloat();

            const auto& rot = value["rotation"];
            rotation.x = rot["x"].GetFloat();
            rotation.y = rot["y"].GetFloat();

            const auto& scl = value["scale"];
            scale.x = scl["x"].GetFloat();
            scale.y = scl["y"].GetFloat();

            prevPos = position;

            if (value.HasMember("parent"))
            {
                int parentID = value["parent"].GetInt();
                if (parentID >= 0)
                {
                    parent = static_cast<Entity>(parentID);
                }
                else
                {
                    parent = std::nullopt;
                }
            }

            // Don't deserialize children - will be rebuilt in Coordinator::Deserialize
            children.clear();

            // Reset world transforms
            worldPosition = position;
            worldScale = scale;
            worldRotation = rotation.x;
            isDirty = true;
        }
    };
}