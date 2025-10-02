#pragma once

#include "../../Math/Math.h"
#include "../Systems/ResourcesTypes.hpp"

namespace Uma_ECS
{
    // currently in 2d
    struct SpriteRenderer
    {
        // pointer pointing to the texture in resources manager
        std::string textureName{};
        bool flipX{};
        bool flipY{};
        Uma_Engine::Texture* texture = nullptr;

        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const //override
        {
            value.SetObject();

            // Save texture name (not the pointer)
            value.AddMember("textureName",
                rapidjson::Value(textureName.c_str(), allocator),
                allocator);

            value.AddMember("flipX", flipX, allocator);
            value.AddMember("flipY", flipY, allocator);
        }

        // Deserialize from JSON
        void Deserialize(const rapidjson::Value& value) //override
        {
            textureName = value["textureName"].GetString();
            flipX = value["flipX"].GetBool();
            flipY = value["flipY"].GetBool();
        }
    };
}