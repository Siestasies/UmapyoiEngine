#pragma once

#include "../Core/UITypes.h"
#include "../../Math/Math.h"
#include "rapidjson/document.h"
#include <vector>
#include <functional>

namespace Uma_UI
{
    enum class EasingType
    {
        Linear = 0,
        EaseInQuad,
        EaseOutQuad,
        EaseInOutQuad,
        EaseInCubic,
        EaseOutCubic,
        EaseInOutCubic,
        EaseInQuart,
        EaseOutQuart,
        EaseInOutQuart,
        EaseInElastic,
        EaseOutElastic,
        EaseInOutElastic,
        EaseInBounce,
        EaseOutBounce,
        EaseInOutBounce
    };

    enum class EffectProperty
    {
        Position = 0,      // RectTransform.anchoredPosition
        Scale,             // RectTransform.sizeDelta
        Rotation,          // Not typically used in UI but available
        ColorTint,         // Image.color
        Alpha,             // Image.color.a / Text.color.a
        FillAmount         // For progress bars, radial fills
    };

    class UIEffectClip
    {
    public:
        EffectProperty property = EffectProperty::Position;
        EasingType easing = EasingType::Linear;

        float duration = 1.0f;
        float delay = 0.0f;
        bool loop = false;

        // Start/end values (interpretation depends on property)
        Vec2 startVec2 = Vec2(0.0f, 0.0f);
        Vec2 endVec2 = Vec2(0.0f, 0.0f);

        Colour startColor = Colour::White();
        Colour endColor = Colour::White();

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
            return std::max(0.0f, std::min(1.0f, t));
        }

        bool IsComplete() const
        {
            if (!hasStarted) return false;
            return currentTime >= (delay + duration);
        }
    };

    class UIEffects
    {
    public:
        std::vector<UIEffectClip> clips;
        bool playOnEnable = false;

        void AddClip(const UIEffectClip& clip)
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

        void Serialize(rapidjson::Value& jsonValue, rapidjson::Document::AllocatorType& allocator) const
        {
            jsonValue.SetObject();
            jsonValue.AddMember("playOnEnable", playOnEnable, allocator);

            rapidjson::Value clipsArray(rapidjson::kArrayType);
            for (const auto& clip : clips)
            {
                rapidjson::Value clipObj(rapidjson::kObjectType);

                clipObj.AddMember("property", static_cast<int>(clip.property), allocator);
                clipObj.AddMember("easing", static_cast<int>(clip.easing), allocator);
                clipObj.AddMember("duration", clip.duration, allocator);
                clipObj.AddMember("delay", clip.delay, allocator);
                clipObj.AddMember("loop", clip.loop, allocator);

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

                UIEffectClip clip;
                clip.property = static_cast<EffectProperty>(clipObj["property"].GetInt());
                clip.easing = static_cast<EasingType>(clipObj["easing"].GetInt());
                clip.duration = clipObj["duration"].GetFloat();
                clip.delay = clipObj["delay"].GetFloat();
                clip.loop = clipObj["loop"].GetBool();

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

    // ========================================================================
    // EASING FUNCTIONS
    // ========================================================================

    namespace Easing
    {
        inline float Linear(float t)
        {
            return t;
        }

        inline float EaseInQuad(float t)
        {
            return t * t;
        }

        inline float EaseOutQuad(float t)
        {
            return t * (2.0f - t);
        }

        inline float EaseInOutQuad(float t)
        {
            return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
        }

        inline float EaseInCubic(float t)
        {
            return t * t * t;
        }

        inline float EaseOutCubic(float t)
        {
            float f = t - 1.0f;
            return f * f * f + 1.0f;
        }

        inline float EaseInOutCubic(float t)
        {
            return t < 0.5f ? 4.0f * t * t * t : (t - 1.0f) * (2.0f * t - 2.0f) * (2.0f * t - 2.0f) + 1.0f;
        }

        inline float EaseInQuart(float t)
        {
            return t * t * t * t;
        }

        inline float EaseOutQuart(float t)
        {
            float f = t - 1.0f;
            return 1.0f - f * f * f * f;
        }

        inline float EaseInOutQuart(float t)
        {
            if (t < 0.5f)
            {
                return 8.0f * t * t * t * t;
            }
            else
            {
                float f = t - 1.0f;
                return 1.0f - 8.0f * f * f * f * f;
            }
        }

        inline float EaseOutElastic(float t)
        {
            if (t == 0.0f || t == 1.0f) return t;

            float p = 0.3f;
            return std::pow(2.0f, -10.0f * t) * std::sin((t - p / 4.0f) * (2.0f * 3.14159f) / p) + 1.0f;
        }

        inline float EaseOutBounce(float t)
        {
            if (t < 1.0f / 2.75f)
            {
                return 7.5625f * t * t;
            }
            else if (t < 2.0f / 2.75f)
            {
                float f = t - 1.5f / 2.75f;
                return 7.5625f * f * f + 0.75f;
            }
            else if (t < 2.5f / 2.75f)
            {
                float f = t - 2.25f / 2.75f;
                return 7.5625f * f * f + 0.9375f;
            }
            else
            {
                float f = t - 2.625f / 2.75f;
                return 7.5625f * f * f + 0.984375f;
            }
        }

        inline float Apply(EasingType type, float t)
        {
            t = std::max(0.0f, std::min(1.0f, t));

            switch (type)
            {
            case EasingType::Linear:          return Linear(t);
            case EasingType::EaseInQuad:      return EaseInQuad(t);
            case EasingType::EaseOutQuad:     return EaseOutQuad(t);
            case EasingType::EaseInOutQuad:   return EaseInOutQuad(t);
            case EasingType::EaseInCubic:     return EaseInCubic(t);
            case EasingType::EaseOutCubic:    return EaseOutCubic(t);
            case EasingType::EaseInOutCubic:  return EaseInOutCubic(t);
            case EasingType::EaseInQuart:     return EaseInQuart(t);
            case EasingType::EaseOutQuart:    return EaseOutQuart(t);
            case EasingType::EaseInOutQuart:  return EaseInOutQuart(t);
            case EasingType::EaseOutElastic:  return EaseOutElastic(t);
            case EasingType::EaseOutBounce:   return EaseOutBounce(t);
            default:                          return Linear(t);
            }
        }
    }

    // ========================================================================
    // INTERPOLATION HELPERS
    // ========================================================================

    inline Vec2 LerpVec2(const Vec2& a, const Vec2& b, float t)
    {
        return Vec2(
            a.x + (b.x - a.x) * t,
            a.y + (b.y - a.y) * t
        );
    }

    inline Colour LerpColor(const Colour& a, const Colour& b, float t)
    {
        return Colour(
            a.r + (b.r - a.r) * t,
            a.g + (b.g - a.g) * t,
            a.b + (b.b - a.b) * t,
            a.a + (b.a - a.a) * t
        );
    }

    inline float LerpFloat(float a, float b, float t)
    {
        return a + (b - a) * t;
    }
}