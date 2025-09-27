#pragma once

#include "Math/Math.h"

namespace Uma_ECS
{
    struct Camera
    {
        float mZoom; // default to 1.0f
        bool followPlayer;
        //float viewportWidth;
        //float viewportHeight;
    };
}