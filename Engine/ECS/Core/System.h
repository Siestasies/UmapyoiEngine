#pragma once

#include "Types.h"
#include <set>

namespace Uma_ECS
{
    class ISystem
    {
    public:
        std::set<Entity> aEntities;
    };
}
