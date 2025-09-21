#pragma once

#include "../../Math/Math.h"
#include "../Systems/ResourcesTypes.hpp"

namespace Uma_ECS
{
    // currently in 2d
    struct SpriteRenderer
    {
        // pointer pointing to the texture in resources manager
        Uma_Engine::Texture* texture = nullptr;

        bool flipX;
        BOOL flipY;
    };
}