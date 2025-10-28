#pragma once
#include "../../Math/Math.h"
#include "../Systems/SpriteAnimator.h"
#include <string>

namespace Uma_ECS
{
    struct Animator
    {
        Uma_Engine::SpriteAnimator animator;
        bool autoPlay = true;
        std::string initialClip = "";

        // Current frame UVs
        Vec2 uvOffset = Vec2(0.0f, 0.0f);
        Vec2 uvSize = Vec2(1.0f, 1.0f);

        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const
        {
            value.SetObject();
            value.AddMember("autoPlay", autoPlay, allocator);
            value.AddMember("initialClip",
                rapidjson::Value(initialClip.c_str(), allocator),
                allocator);
        }

        void Deserialize(const rapidjson::Value& value)
        {
            autoPlay = value["autoPlay"].GetBool();
            initialClip = value["initialClip"].GetString();

            uvOffset = Vec2(0.0f, 0.0f);
            uvSize = Vec2(1.0f, 1.0f);
        }
    };
}