#pragma once

#include "../Core/UITypes.h"
#include "../../Math/Math.h"
#include "rapidjson/document.h"
#include <vector>
#include <functional>

namespace Uma_UI
{
    class EffectClip
    {
    public:
        std::string name;
        EffectProperty property = EffectProperty::Position;
        EasingType easing = EasingType::Linear;

        float duration = 1.0f;
        float delay = 0.0f;
        bool loop = false;
        bool applyToChildren = false;

        // Start/end values (interpretation depends on property)
        Vec2 startVec2 = Vec2(0.0f, 0.0f);
        Vec2 endVec2 = Vec2(0.0f, 0.0f);

        Color startColor = Color::White();
        Color endColor = Color::White();

        float startFloat = 0.0f;
        float endFloat = 1.0f;

        // Runtime state
        float currentTime = 0.0f;
        bool isPlaying = false;
        bool hasStarted = false;

        void Reset()
        {
            currentTime = 0.0f;
            isPlaying = false;
            hasStarted = false;
        }

        void Play()
        {
            isPlaying = true;
        }

        void Pause()
        {
            isPlaying = false;
        }

        void Stop()
        {
            Reset();
        }

        float GetProgress() const
        {
            if (duration <= 0.0f) return 1.0f;

            float t = (currentTime - delay) / duration;
            return (std::max)(0.0f, (std::min)(1.0f, t));
        }

        bool IsComplete() const
        {
            if (!hasStarted) return false;
            return currentTime >= (delay + duration);
        }
    };

    class Effects
    {
    public:
        std::vector<EffectClip> clips;
        bool playOnEnable = false;

        void AddClip(const EffectClip& clip)
        {
            clips.push_back(clip);
        }

        void PlayAll()
        {
            for (auto& clip : clips)
            {
                clip.Play();
            }
        }

        void StopAll()
        {
            for (auto& clip : clips)
            {
                clip.Stop();
            }
        }

        void ResetAll()
        {
            for (auto& clip : clips)
            {
                clip.Reset();
            }
        }

        void PlayClip(size_t index)
        {
            if (index < clips.size())
            {
                clips[index].Play();
            }
        }

        void PauseClip(size_t index)
        {
            if (index < clips.size())
            {
                clips[index].Pause();
            }
        }

        void StopClip(size_t index)
        {
            if (index < clips.size())
            {
                clips[index].Stop();
            }
        }

        void ResetClip(size_t index)
        {
            if (index < clips.size())
            {
                clips[index].Reset();
            }
        }

        // Query clip state
        bool IsClipPlaying(size_t index) const
        {
            if (index < clips.size())
            {
                return clips[index].isPlaying;
            }
            return false;
        }

        bool IsClipComplete(size_t index) const
        {
            if (index < clips.size())
            {
                return clips[index].IsComplete();
            }
            return false;
        }

        size_t GetClipCount() const
        {
            return clips.size();
        }

        // Control clips by name
        void PlayClipByName(const std::string& name)
        {
            for (auto& clip : clips)
            {
                if (clip.name == name)
                {
                    clip.Play();
                    return;
                }
            }
        }

        void PauseClipByName(const std::string& name)
        {
            for (auto& clip : clips)
            {
                if (clip.name == name)
                {
                    clip.Pause();
                    return;
                }
            }
        }

        void StopClipByName(const std::string& name)
        {
            for (auto& clip : clips)
            {
                if (clip.name == name)
                {
                    clip.Stop();
                    return;
                }
            }
        }

        void ResetClipByName(const std::string& name)
        {
            for (auto& clip : clips)
            {
                if (clip.name == name)
                {
                    clip.Reset();
                    return;
                }
            }
        }

        // Find clip index by name
        int FindClipIndexByName(const std::string& name) const
        {
            for (size_t i = 0; i < clips.size(); ++i)
            {
                if (clips[i].name == name)
                {
                    return static_cast<int>(i);
                }
            }
            return -1;  // Not found
        }

        void Serialize(rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) const
        {
            jsonValue.SetObject();
            jsonValue.AddMember("playOnEnable", playOnEnable, allocator);

            rapidjson::Value clipsArray(rapidjson::kArrayType);
            for (const auto& clip : clips)
            {
                rapidjson::Value clipObj(rapidjson::kObjectType);

                // Serialize name
                rapidjson::Value nameValue;
                nameValue.SetString(clip.name.c_str(), clip.name.length(), allocator);
                clipObj.AddMember("name", nameValue, allocator);

                clipObj.AddMember("property", static_cast<int>(clip.property), allocator);
                clipObj.AddMember("easing", static_cast<int>(clip.easing), allocator);
                clipObj.AddMember("duration", clip.duration, allocator);
                clipObj.AddMember("delay", clip.delay, allocator);
                clipObj.AddMember("loop", clip.loop, allocator);
                clipObj.AddMember("applyToChildren", clip.applyToChildren, allocator);

                rapidjson::Value startVec(rapidjson::kObjectType);
                startVec.AddMember("x", clip.startVec2.x, allocator);
                startVec.AddMember("y", clip.startVec2.y, allocator);
                clipObj.AddMember("startVec2", startVec, allocator);

                rapidjson::Value endVec(rapidjson::kObjectType);
                endVec.AddMember("x", clip.endVec2.x, allocator);
                endVec.AddMember("y", clip.endVec2.y, allocator);
                clipObj.AddMember("endVec2", endVec, allocator);

                rapidjson::Value startCol(rapidjson::kObjectType);
                startCol.AddMember("r", clip.startColor.r, allocator);
                startCol.AddMember("g", clip.startColor.g, allocator);
                startCol.AddMember("b", clip.startColor.b, allocator);
                startCol.AddMember("a", clip.startColor.a, allocator);
                clipObj.AddMember("startColor", startCol, allocator);

                rapidjson::Value endCol(rapidjson::kObjectType);
                endCol.AddMember("r", clip.endColor.r, allocator);
                endCol.AddMember("g", clip.endColor.g, allocator);
                endCol.AddMember("b", clip.endColor.b, allocator);
                endCol.AddMember("a", clip.endColor.a, allocator);
                clipObj.AddMember("endColor", endCol, allocator);

                clipObj.AddMember("startFloat", clip.startFloat, allocator);
                clipObj.AddMember("endFloat", clip.endFloat, allocator);

                clipsArray.PushBack(clipObj, allocator);
            }
            jsonValue.AddMember("clips", clipsArray, allocator);
        }

        void Deserialize(const rapidjson::Value& jsonValue)
        {
            playOnEnable = jsonValue["playOnEnable"].GetBool();

            clips.clear();
            const auto& clipsArray = jsonValue["clips"];
            for (rapidjson::SizeType i = 0; i < clipsArray.Size(); ++i)
            {
                const auto& clipObj = clipsArray[i];

                EffectClip clip;
                clip.name = clipObj.HasMember("name") ? clipObj["name"].GetString() : "";
                clip.property = static_cast<EffectProperty>(clipObj["property"].GetInt());
                clip.easing = static_cast<EasingType>(clipObj["easing"].GetInt());
                clip.duration = clipObj["duration"].GetFloat();
                clip.delay = clipObj["delay"].GetFloat();
                clip.loop = clipObj["loop"].GetBool();
                clip.applyToChildren = clipObj.HasMember("applyToChildren") ? clipObj["applyToChildren"].GetBool() : false;

                const auto& startVec = clipObj["startVec2"];
                clip.startVec2.x = startVec["x"].GetFloat();
                clip.startVec2.y = startVec["y"].GetFloat();

                const auto& endVec = clipObj["endVec2"];
                clip.endVec2.x = endVec["x"].GetFloat();
                clip.endVec2.y = endVec["y"].GetFloat();

                const auto& startCol = clipObj["startColor"];
                clip.startColor.r = startCol["r"].GetFloat();
                clip.startColor.g = startCol["g"].GetFloat();
                clip.startColor.b = startCol["b"].GetFloat();
                clip.startColor.a = startCol["a"].GetFloat();

                const auto& endCol = clipObj["endColor"];
                clip.endColor.r = endCol["r"].GetFloat();
                clip.endColor.g = endCol["g"].GetFloat();
                clip.endColor.b = endCol["b"].GetFloat();
                clip.endColor.a = endCol["a"].GetFloat();

                clip.startFloat = clipObj["startFloat"].GetFloat();
                clip.endFloat = clipObj["endFloat"].GetFloat();

                clips.push_back(clip);
            }
        }
    };
}