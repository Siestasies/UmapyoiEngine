#pragma once

#include "../../Math/Vec2.hpp"

namespace Uma_ECS
{
    // currently in 2d
    struct RigidBody
    {
        Vec2 velocity;
        Vec2 acceleration;
    };
}