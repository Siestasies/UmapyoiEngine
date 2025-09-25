#pragma once

#include "Math/Math.h"

namespace Uma_ECS
{
    struct Camera
    {
        Vec2 mPosition{};
        float mZoom{1.0f};
    };
}