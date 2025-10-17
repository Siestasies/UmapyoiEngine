/*!
\file   RigidBody.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines 2D rigid body physics component containing velocity, acceleration, and physical properties.

Stores Vec2 velocity and acceleration for integration by PhysicsSystem, with tunable accel_strength for input responsiveness
and fric_coeff for friction damping to slow entities naturally.
Provides JSON serialization for all physics properties including vector components. Used in conjunction with Transform
component for position updates via semi-implicit Euler integration.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

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

        float accel_strength{};   // acceleration (for tweak)
        float fric_coeff{};       // friction

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