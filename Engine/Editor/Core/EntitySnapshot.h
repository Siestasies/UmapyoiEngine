#pragma once

#include "ECS/Core/Types.hpp"
#include "RapidJSON/document.h"

#include <optional>
#include <vector>

namespace Uma_Editor
{
    struct EntitySnapshot
    {
        Uma_ECS::Entity entityID;
        rapidjson::Document componentData;

        std::optional<Uma_ECS::Entity> parentID;
        std::vector<Uma_ECS::Entity> childrenIDs;
    };
}