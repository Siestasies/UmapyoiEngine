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
    };
}