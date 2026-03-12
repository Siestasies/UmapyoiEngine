/*!
\file    Animator.hpp
\par     Project: GAM250
\par     Course: CSD2451
\par     Section A
\par     Software Engineering Project 4

\author Javier Chua Dong Qing (100%)
\par     E-mail: javierdongqing.chua@digipen.edu
\par     DigiPen login: javierdongqing.chua

\brief
Defines Animator component for the ECS

This component uses the SpriteAnimator engine class to store animation state
and provides serialization/deserialization for loading animation clips
from JSON scene files

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once
#include "../../Math/Math.h"
#include "../../Systems/ResourcesTypes.hpp"
#include "../Systems/SpriteAnimator.h"
#include <string>
#include <memory>

namespace Uma_ECS
{
    /**
     * \struct Animator
     * \brief An ECS component that manages 2D sprite animation
     *
     * Uses SpriteAnimator class to store playback state and a
     * collection of AnimationClips. This component is updated by the
     * `AnimationSystem` to calculate UV coordinates, which are then used
     * by RenderSystem
     */
    struct Animator
    {
        Uma_Engine::SpriteAnimator animator;
        bool autoPlay = true;
        std::string initialClip = "";
        bool isInitialized = false;

        // Current frame UVs (cached by AnimatorSystem)
        Vec2 uvOffset = Vec2(0.0f, 0.0f);
        Vec2 uvSize = Vec2(1.0f, 1.0f);

        // Active texture resolved by AnimatorSystem (nullptr = use Sprite's texture)
        std::shared_ptr<Uma_Engine::Texture> activeTexture = nullptr;

        /**
         * \brief Serializes the Animator component data to a rapidjson::Value
         * \param value The rapidjson::Value object to write data into
         * \param allocator The rapidjson allocator used to create new JSON members
         * \details Serializes `autoPlay`, `initialClip`, and the complete list
         * of `AnimationClip`s stored in the `animator`
         */
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
                clipObj.AddMember("texturePath",
                    rapidjson::Value(clip.texturePath.c_str(), allocator),
                    allocator);

                clipsArray.PushBack(clipObj, allocator);
            }

            value.AddMember("clips", clipsArray, allocator);

            //std::cout << "Serialized " << clips.size() << " animation clips" << std::endl;
        }

        /**
         * \brief Deserializes the Animator component data from a rapidjson::Value
         * \param  value rapidjson::Value object to read data from
         * \details Populates the component's data from JSON. This includes
         * reloading all `AnimationClip`s into the `animator` and
         * automatically playing the `initialClip` if `autoPlay` is true
         */
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
                    std::string texturePath = "";
                    if (clipObj.HasMember("texturePath"))
                    {
                        texturePath = clipObj["texturePath"].GetString();
                    }

                    // Add the clip back to the animator
                    animator.AddClip(name, framesX, framesY, startFrame,
                        frameCount, speed, loop, texturePath);
                }

                //std::cout << "Deserialized " << clipsArray.Size() << " animation clips" << std::endl;
                animator.GetUVs(uvOffset, uvSize);
            }
        }
    };
}