#pragma once

#include "../../Math/Math.h"

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