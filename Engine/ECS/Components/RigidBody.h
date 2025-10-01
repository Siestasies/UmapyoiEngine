#pragma once

#include "../../Math/Math.h"
//#include "../../Math/Vector.hpp"

namespace Uma_ECS
{
    // currently in 2d
    struct RigidBody
    {
        Vec2 velocity{};
        Vec2 acceleration{};

        float accel_strength;   // acceleration (for tweak)
        float fric_coeff;       // friction

        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const //override
        {
            value.SetObject();

            // velocity
            rapidjson::Value vel(rapidjson::kObjectType);
            vel.AddMember("x", velocity.x, allocator);
            vel.AddMember("y", velocity.y, allocator);
            value.AddMember("velocity", vel, allocator);

            // velocity
            rapidjson::Value acc(rapidjson::kObjectType);
            acc.AddMember("x", acceleration.x, allocator);
            acc.AddMember("y", acceleration.y, allocator);
            value.AddMember("acceleration", acc, allocator);

            value.AddMember("accel_strength", accel_strength, allocator);
            value.AddMember("fric_coeff", fric_coeff, allocator);
        }

        // Deserialize from JSON
        void Deserialize(const rapidjson::Value& value) //override
        {
            const auto& vel = value["velocity"];
            velocity.x = vel["x"].GetFloat();
            velocity.y = vel["y"].GetFloat();

            const auto& acc = value["acceleration"];
            acceleration.x = acc["x"].GetFloat();
            acceleration.y = acc["y"].GetFloat();

            accel_strength = value["accel_strength"].GetFloat();
            fric_coeff = value["fric_coeff"].GetFloat();
        }
    };
}