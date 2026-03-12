/*!
\file   UITypes.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Core UI type definitions and utility structures.

This header provides fundamental types used throughout the UI system including
color, rectangle, enums for UI states, and anchor presets.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "../../ECS/Core/Types.hpp"
#include "Math/Math.h"

#include "rapidjson/document.h"
#include <functional>

namespace Uma_UI
{
    using UICallback = std::function<void(Uma_ECS::Entity)>;

    /*!
     * \struct Color
     * \brief RGBA color with float components in range [0,1].
     */
    struct Color
    {
        float r = 1.0f;
        float g = 1.0f;
        float b = 1.0f;
        float a = 1.0f;

        /*!
         * \brief Default constructor. Initializes to white (1,1,1,1).
         */
        constexpr Color() = default;

        /*!
         * \brief Constructs a color from RGBA components.
         * \param r_ Red component [0,1].
         * \param g_ Green component [0,1].
         * \param b_ Blue component [0,1].
         * \param a_ Alpha component [0,1].
         */
        constexpr Color(float r_, float g_, float b_, float a_ = 1.0f) : r(r_), g(g_), b(b_), a(a_) {}

        /*!
         * \brief Converts color to Vec3 (RGB only).
         * \return Vec3 containing RGB components.
         */
        Vec3 ToVec3() const { return Vec3(r, g, b); }

        /*! \brief Returns opaque white. \return Color(1,1,1,1). */
        static constexpr Color White() { return Color(1.0f, 1.0f, 1.0f, 1.0f); }
        /*! \brief Returns opaque black. \return Color(0,0,0,1). */
        static constexpr Color Black() { return Color(0.0f, 0.0f, 0.0f, 1.0f); }
        /*! \brief Returns fully transparent black. \return Color(0,0,0,0). */
        static constexpr Color Clear() { return Color(0.0f, 0.0f, 0.0f, 0.0f); }
        /*! \brief Returns opaque red. \return Color(1,0,0,1). */
        static constexpr Color Red() { return Color(1.0f, 0.0f, 0.0f, 1.0f); }
        /*! \brief Returns opaque green. \return Color(0,1,0,1). */
        static constexpr Color Green() { return Color(0.0f, 1.0f, 0.0f, 1.0f); }
        /*! \brief Returns opaque blue. \return Color(0,0,1,1). */
        static constexpr Color Blue() { return Color(0.0f, 0.0f, 1.0f, 1.0f); }
        /*! \brief Returns opaque gray (50%). \return Color(0.5,0.5,0.5,1). */
        static constexpr Color Gray() { return Color(0.5f, 0.5f, 0.5f, 1.0f); }
    };

    /*!
     * \struct Rect
     * \brief Axis-aligned rectangle in NDC space [-1,1].
     */
    struct Rect
    {
        float x = 0.0f;
        float y = 0.0f;
        float width = 1.0f;
        float height = 1.0f;

        /*! \brief Default constructor. Initializes to origin with size (1,1). */
        constexpr Rect() = default;

        /*!
         * \brief Constructs a rectangle with position and size.
         * \param x_ Center X position.
         * \param y_ Center Y position.
         * \param w Width.
         * \param h Height.
         */
        constexpr Rect(float x_, float y_, float w, float h) : x(x_), y(y_), width(w), height(h) {}

        /*!
         * \brief Gets the left edge position.
         * \return Left edge in NDC space.
         */
        float Left() const { return x - width * 0.5f; }

        /*!
         * \brief Gets the right edge position.
         * \return Right edge in NDC space.
         */
        float Right() const { return x + width * 0.5f; }

        /*!
         * \brief Gets the bottom edge position.
         * \return Bottom edge in NDC space.
         */
        float Bottom() const { return y - height * 0.5f; }

        /*!
         * \brief Gets the top edge position.
         * \return Top edge in NDC space.
         */
        float Top() const { return y + height * 0.5f; }

        /*!
         * \brief Gets the center point.
         * \return Center position as Vec2.
         */
        Vec2 Center() const { return Vec2(x, y); }

        /*!
         * \brief Gets the size.
         * \return Size as Vec2 (width, height).
         */
        Vec2 Size() const { return Vec2(width, height); }

        /*!
         * \brief Tests if a point is inside the rectangle.
         * \param point Point in NDC space to test.
         * \return True if point is inside the rect.
         */
        bool Contains(const Vec2& point) const
        {
            return point.x >= Left() && point.x <= Right() && point.y >= Bottom() && point.y <= Top();
        }
    };

    /*!
     * \enum CanvasScaleMode
     * \brief Defines how UI scales with screen resolution.
     */
    enum class CanvasScaleMode
    {
        ConstantPixelSize = 0,
        ScaleWithScreenSize = 1
    };

    /*!
     * \enum ButtonState
     * \brief Current interaction state of a button.
     */
    enum class ButtonState
    {
        Normal = 0,
        Hovered = 1,
        Pressed = 2,
        Disabled = 3
    };

    /*!
     * \enum CheckboxState
     * \brief Current interaction state of a checkbox.
     */
    enum class CheckboxState
    {
        Normal = 0,
        Hovered = 1,
        Pressed = 2,
        Disabled = 3
    };

    /*!
     * \enum TextAlignment
     * \brief Horizontal text alignment within a rect.
     */
    enum class TextAlignment
    {
        Left = 0,
        Center = 1,
        Right = 2
    };

    /*!
     * \struct AnchorPreset
     * \brief Common Unity-style anchor configurations.
     */
    struct AnchorPreset
    {
        Vec2 anchorMin;
        Vec2 anchorMax;
        Vec2 pivot;

        /*! \brief Anchor preset for top-left corner. \return AnchorPreset anchored to top-left. */
        static AnchorPreset TopLeft() { return { Vec2(0.0f, 1.0f), Vec2(0.0f, 1.0f), Vec2(0.0f, 1.0f) }; }
        /*! \brief Anchor preset for top-center. \return AnchorPreset anchored to top-center. */
        static AnchorPreset TopCenter() { return { Vec2(0.5f, 1.0f), Vec2(0.5f, 1.0f), Vec2(0.5f, 1.0f) }; }
        /*! \brief Anchor preset for top-right corner. \return AnchorPreset anchored to top-right. */
        static AnchorPreset TopRight() { return { Vec2(1.0f, 1.0f), Vec2(1.0f, 1.0f), Vec2(1.0f, 1.0f) }; }
        /*! \brief Anchor preset for middle-left. \return AnchorPreset anchored to middle-left. */
        static AnchorPreset MiddleLeft() { return { Vec2(0.0f, 0.5f), Vec2(0.0f, 0.5f), Vec2(0.0f, 0.5f) }; }
        /*! \brief Anchor preset for middle-center. \return AnchorPreset anchored to middle-center. */
        static AnchorPreset MiddleCenter() { return { Vec2(0.5f, 0.5f), Vec2(0.5f, 0.5f), Vec2(0.5f, 0.5f) }; }
        /*! \brief Anchor preset for middle-right. \return AnchorPreset anchored to middle-right. */
        static AnchorPreset MiddleRight() { return { Vec2(1.0f, 0.5f), Vec2(1.0f, 0.5f), Vec2(1.0f, 0.5f) }; }
        /*! \brief Anchor preset for bottom-left corner. \return AnchorPreset anchored to bottom-left. */
        static AnchorPreset BottomLeft() { return { Vec2(0.0f, 0.0f), Vec2(0.0f, 0.0f), Vec2(0.0f, 0.0f) }; }
        /*! \brief Anchor preset for bottom-center. \return AnchorPreset anchored to bottom-center. */
        static AnchorPreset BottomCenter() { return { Vec2(0.5f, 0.0f), Vec2(0.5f, 0.0f), Vec2(0.5f, 0.0f) }; }
        /*! \brief Anchor preset for bottom-right corner. \return AnchorPreset anchored to bottom-right. */
        static AnchorPreset BottomRight() { return { Vec2(1.0f, 0.0f), Vec2(1.0f, 0.0f), Vec2(1.0f, 0.0f) }; }
        /*! \brief Anchor preset for horizontal stretch. \return AnchorPreset stretching full width. */
        static AnchorPreset StretchHorizontal() { return { Vec2(0.0f, 0.5f), Vec2(1.0f, 0.5f), Vec2(0.5f, 0.5f) }; }
        /*! \brief Anchor preset for vertical stretch. \return AnchorPreset stretching full height. */
        static AnchorPreset StretchVertical() { return { Vec2(0.5f, 0.0f), Vec2(0.5f, 1.0f), Vec2(0.5f, 0.5f) }; }
        /*! \brief Anchor preset for full stretch in both axes. \return AnchorPreset stretching to fill parent. */
        static AnchorPreset StretchAll() { return { Vec2(0.0f, 0.0f), Vec2(1.0f, 1.0f), Vec2(0.5f, 0.5f) }; }
    };

    /*!
     * \enum EasingType
     * \brief Enumeration of supported easing/interpolation curves.
     */
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

    /*!
     * \enum EffectProperty
     * \brief Identifies which property an effect clip animates.
     */
    enum class EffectProperty
    {
        Position = 0,      // RectTransform.anchoredPosition
        Scale,             // RectTransform.sizeDelta
        ColorTint,         // Image.color
        Alpha,             // Image.color.a / Text.color.a
        FillAmount,         // For progress bars, radial fills
        SpritesheetFrame
    };

    /*!
     * \struct DialogueLine
     * \brief A single line of dialogue with speaker, text, and optional portrait.
     */
    struct DialogueLine
    {
        std::string speaker = "";
        std::string text = "";   
        std::string portrait = "";

        /*!
         * \brief Serializes this dialogue line to a JSON value.
         * \param out Output JSON value to populate.
         * \param alloc RapidJSON allocator for memory management.
         */
        void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& alloc) const
        {
            out.SetObject();
            out.AddMember("speaker", rapidjson::Value(speaker.c_str(), alloc), alloc);
            out.AddMember("text", rapidjson::Value(text.c_str(), alloc), alloc);
            out.AddMember("portrait", rapidjson::Value(portrait.c_str(), alloc), alloc);
        }

        /*!
         * \brief Deserializes this dialogue line from a JSON value.
         * \param in Input JSON value to read from.
         */
        void Deserialize(const rapidjson::Value& in)
        {
            speaker = in.HasMember("speaker") ? in["speaker"].GetString() : "";
            text = in.HasMember("text") ? in["text"].GetString() : "";
            portrait = in.HasMember("portrait") ? in["portrait"].GetString() : "";
        }
    };

    /*!
     * \struct DialogueSequence
     * \brief An ordered collection of DialogueLine entries forming one conversation.
     */
    struct DialogueSequence
    {
        std::string              id = "";
        std::vector<DialogueLine> lines;

        /*!
         * \brief Serializes this dialogue sequence to a JSON value.
         * \param out Output JSON value to populate.
         * \param alloc RapidJSON allocator for memory management.
         */
        void Serialize(rapidjson::Value& out, rapidjson::Document::AllocatorType& alloc) const
        {
            out.SetObject();
            out.AddMember("id", rapidjson::Value(id.c_str(), alloc), alloc);

            rapidjson::Value arr(rapidjson::kArrayType);
            for (const auto& line : lines)
            {
                rapidjson::Value lineVal;
                line.Serialize(lineVal, alloc);
                arr.PushBack(lineVal, alloc);
            }
            out.AddMember("lines", arr, alloc);
        }

        /*!
         * \brief Deserializes this dialogue sequence from a JSON value.
         * \param in Input JSON value to read from.
         */
        void Deserialize(const rapidjson::Value& in)
        {
            id = in.HasMember("id") ? in["id"].GetString() : "";
            lines.clear();
            if (in.HasMember("lines") && in["lines"].IsArray())
            {
                for (const auto& lineVal : in["lines"].GetArray())
                {
                    DialogueLine l;
                    l.Deserialize(lineVal);
                    lines.push_back(l);
                }
            }
        }
    };

    namespace Easing
    {
        /*! \brief Linear interpolation (no easing). \param t Normalized time [0,1]. \return Eased value. */
        inline float Linear(float t)
        {
            return t;
        }

        /*! \brief Quadratic ease-in curve. \param t Normalized time [0,1]. \return Eased value. */
        inline float EaseInQuad(float t)
        {
            return t * t;
        }

        /*! \brief Quadratic ease-out curve. \param t Normalized time [0,1]. \return Eased value. */
        inline float EaseOutQuad(float t)
        {
            return t * (2.0f - t);
        }

        /*! \brief Quadratic ease-in-out curve. \param t Normalized time [0,1]. \return Eased value. */
        inline float EaseInOutQuad(float t)
        {
            return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
        }

        /*! \brief Cubic ease-in curve. \param t Normalized time [0,1]. \return Eased value. */
        inline float EaseInCubic(float t)
        {
            return t * t * t;
        }

        /*! \brief Cubic ease-out curve. \param t Normalized time [0,1]. \return Eased value. */
        inline float EaseOutCubic(float t)
        {
            float f = t - 1.0f;
            return f * f * f + 1.0f;
        }

        /*! \brief Cubic ease-in-out curve. \param t Normalized time [0,1]. \return Eased value. */
        inline float EaseInOutCubic(float t)
        {
            return t < 0.5f ? 4.0f * t * t * t : (t - 1.0f) * (2.0f * t - 2.0f) * (2.0f * t - 2.0f) + 1.0f;
        }

        /*! \brief Quartic ease-in curve. \param t Normalized time [0,1]. \return Eased value. */
        inline float EaseInQuart(float t)
        {
            return t * t * t * t;
        }

        /*! \brief Quartic ease-out curve. \param t Normalized time [0,1]. \return Eased value. */
        inline float EaseOutQuart(float t)
        {
            float f = t - 1.0f;
            return 1.0f - f * f * f * f;
        }

        /*! \brief Quartic ease-in-out curve. \param t Normalized time [0,1]. \return Eased value. */
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

        /*! \brief Elastic ease-out curve with spring-like overshoot. \param t Normalized time [0,1]. \return Eased value. */
        inline float EaseOutElastic(float t)
        {
            if (t == 0.0f || t == 1.0f) return t;

            float p = 0.3f;
            return std::pow(2.0f, -10.0f * t) * std::sin((t - p / 4.0f) * (2.0f * 3.14159f) / p) + 1.0f;
        }

        /*! \brief Bounce ease-out curve simulating a bouncing ball. \param t Normalized time [0,1]. \return Eased value. */
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

        /*!
         * \brief Applies the specified easing function to a normalized time value.
         * \param type The easing curve to apply.
         * \param t Normalized time, clamped to [0,1].
         * \return Eased value.
         */
        inline float Apply(EasingType type, float t)
        {
            t = (std::max)(0.0f, (std::min)(1.0f, t));

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

    /*!
     * \brief Linearly interpolates between two Vec2 values.
     * \param a Start value.
     * \param b End value.
     * \param t Interpolation factor [0,1].
     * \return Interpolated Vec2.
     */
    inline Vec2 LerpVec2(const Vec2& a, const Vec2& b, float t)
    {
        return Vec2(
            a.x + (b.x - a.x) * t,
            a.y + (b.y - a.y) * t
        );
    }

    /*!
     * \brief Linearly interpolates between two Color values.
     * \param a Start color.
     * \param b End color.
     * \param t Interpolation factor [0,1].
     * \return Interpolated Color.
     */
    inline Color LerpColor(const Color& a, const Color& b, float t)
    {
        return Color(
            a.r + (b.r - a.r) * t,
            a.g + (b.g - a.g) * t,
            a.b + (b.b - a.b) * t,
            a.a + (b.a - a.a) * t
        );
    }

    /*!
     * \brief Linearly interpolates between two float values.
     * \param a Start value.
     * \param b End value.
     * \param t Interpolation factor [0,1].
     * \return Interpolated float.
     */
    inline float LerpFloat(float a, float b, float t)
    {
        return a + (b - a) * t;
    }
}