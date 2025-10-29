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

            // Serialize animation clips
            rapidjson::Value clipsArray(rapidjson::kArrayType);

            const auto& clips = animator.GetClips();
            for (const auto& [name, clip] : clips)
            {
                rapidjson::Value clipObj(rapidjson::kObjectType);

                // Clip name
                clipObj.AddMember("name",
                    rapidjson::Value(name.c_str(), allocator),
                    allocator);

                // Clip data
                clipObj.AddMember("framesX", clip.framesX, allocator);
                clipObj.AddMember("framesY", clip.framesY, allocator);
                clipObj.AddMember("startFrame", clip.startFrame, allocator);
                clipObj.AddMember("frameCount", clip.frameCount, allocator);
                clipObj.AddMember("speed", clip.speed, allocator);
                clipObj.AddMember("loop", clip.loop, allocator);

                clipsArray.PushBack(clipObj, allocator);
            }

            value.AddMember("clips", clipsArray, allocator);

            std::cout << "Serialized " << clips.size() << " animation clips" << std::endl;
        }

        void Deserialize(const rapidjson::Value& value)
        {
            autoPlay = value["autoPlay"].GetBool();
            initialClip = value["initialClip"].GetString();

            uvOffset = Vec2(0.0f, 0.0f);
            uvSize = Vec2(1.0f, 1.0f);

            // Deserialize animation clips
            if (value.HasMember("clips") && value["clips"].IsArray())
            {
                const auto& clipsArray = value["clips"];

                for (rapidjson::SizeType i = 0; i < clipsArray.Size(); i++)
                {
                    const auto& clipObj = clipsArray[i];

                    std::string name = clipObj["name"].GetString();
                    int framesX = clipObj["framesX"].GetInt();
                    int framesY = clipObj["framesY"].GetInt();
                    int startFrame = clipObj["startFrame"].GetInt();
                    int frameCount = clipObj["frameCount"].GetInt();
                    float speed = clipObj["speed"].GetFloat();
                    bool loop = clipObj["loop"].GetBool();

                    // Add the clip back to the animator
                    animator.AddClip(name, framesX, framesY, startFrame,
                        frameCount, speed, loop);
                }

                std::cout << "Deserialized " << clipsArray.Size() << " animation clips" << std::endl;

                // Auto-play the initial clip if specified
                if (autoPlay && !initialClip.empty())
                {
                    animator.Play(initialClip);
                }
            }
        }
    };
}