#pragma once

#include "../../Math/Math.h"
//#include "Core/SerializationBase.h"

namespace Uma_ECS
{
    // currently in 2d
    struct Transform //: public SerializationBase
    {
        Vec2 position;
        Vec2 rotation;
        Vec2 scale;
        Vec2 prevPos; // shdnt edit this value manually

        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const //override
        {
            value.SetObject();
            
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
        }

        // Deserialize from JSON
        void Deserialize(const rapidjson::Value& value) //override
        {
            const auto& pos = value["position"];
            position.x = pos["x"].GetFloat();
            position.y = pos["y"].GetFloat();

            const auto& rot = value["rotation"];
            rotation.x = rot["x"].GetFloat();
            rotation.y = rot["y"].GetFloat();

            const auto& scl = value["scale"];
            scale.x = scl["x"].GetFloat();
            scale.y = scl["y"].GetFloat();

            // Reset prevPos automatically
            prevPos = position;
        }
    };
}