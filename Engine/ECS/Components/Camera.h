#pragma once

#include "Math/Math.h"

namespace Uma_ECS
{
    struct Camera
    {
        float mZoom{}; // default to 1.0f
        bool followPlayer{};
        //float viewportWidth;
        //float viewportHeight;

        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const //override
        {
            value.SetObject();

            value.AddMember("zoom", mZoom, allocator);
            value.AddMember("followPlayer", followPlayer, allocator);
        }

        // Deserialize from JSON
        void Deserialize(const rapidjson::Value& value) //override
        {
            mZoom = value["zoom"].GetFloat();
            followPlayer = value["followPlayer"].GetBool();
        }
    };
}