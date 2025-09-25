#pragma once

#include "Math/Math.h"

namespace Uma_ECS
{
    using LayerMask = unsigned int;

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
        Vec2 min;
        Vec2 max;
    };

    // currently in 2d
    struct Collider
    {
        BoundingBox boundingBox;
        LayerMask layer; // the layer it is in
        LayerMask colliderMask; // the layer it can collide with
    };
}