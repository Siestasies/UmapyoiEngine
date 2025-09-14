#pragma once

#include "VMath.h"

namespace ECS
{
    // currently in 2d
    struct Transform
    {
        Vec2 position;
        Vec2 rotation;
        Vec2 scale;
    };
}