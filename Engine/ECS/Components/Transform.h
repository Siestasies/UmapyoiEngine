#pragma once

#include "../../Math/Math.hpp"

namespace Uma_ECS
{
    // currently in 2d
    struct Transform
    {
        Vec2 position;
        Vec2 rotation;
        Vec2 scale;
    };
}