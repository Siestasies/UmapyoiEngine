#pragma once

#include "Types.h"
#include <set>

namespace ECS
{
    class ISystem
    {
    public:
        std::set<Entity> aEntities;
    };
}
