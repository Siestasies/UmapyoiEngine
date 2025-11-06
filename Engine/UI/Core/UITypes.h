/*!
\file   UITypes.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author CSD2401 Jedrek Lee Jing Wei (100%)
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
#include <functional>

namespace Uma_UI
{
    using UICallback = std::function<void(Uma_ECS::Entity)>;

    /*!
     * \struct Colour
     * \brief RGBA color with float components in range [0,1].
     */
    struct Colour
    {
        float r = 1.0f;
        float g = 1.0f;
        float b = 1.0f;
        float a = 1.0f;

        constexpr Colour() = default;
        constexpr Colour(float r_, float g_, float b_, float a_ = 1.0f) : r(r_), g(g_), b(b_), a(a_) {}

        /*!
         * \brief Converts color to Vec3 (RGB only).
         * \return Vec3 containing RGB components.
         */
        Vec3 ToVec3() const { return Vec3(r, g, b); }

        static constexpr Colour White() { return Colour(1.0f, 1.0f, 1.0f, 1.0f); }
        static constexpr Colour Black() { return Colour(0.0f, 0.0f, 0.0f, 1.0f); }
        static constexpr Colour Clear() { return Colour(0.0f, 0.0f, 0.0f, 0.0f); }
        static constexpr Colour Red() { return Colour(1.0f, 0.0f, 0.0f, 1.0f); }
        static constexpr Colour Green() { return Colour(0.0f, 1.0f, 0.0f, 1.0f); }
        static constexpr Colour Blue() { return Colour(0.0f, 0.0f, 1.0f, 1.0f); }
        static constexpr Colour Gray() { return Colour(0.5f, 0.5f, 0.5f, 1.0f); }
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

        constexpr Rect() = default;
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

        static AnchorPreset TopLeft() { return { Vec2(0.0f, 1.0f), Vec2(0.0f, 1.0f), Vec2(0.0f, 1.0f) }; }
        static AnchorPreset TopCenter() { return { Vec2(0.5f, 1.0f), Vec2(0.5f, 1.0f), Vec2(0.5f, 1.0f) }; }
        static AnchorPreset TopRight() { return { Vec2(1.0f, 1.0f), Vec2(1.0f, 1.0f), Vec2(1.0f, 1.0f) }; }
        static AnchorPreset MiddleLeft() { return { Vec2(0.0f, 0.5f), Vec2(0.0f, 0.5f), Vec2(0.0f, 0.5f) }; }
        static AnchorPreset MiddleCenter() { return { Vec2(0.5f, 0.5f), Vec2(0.5f, 0.5f), Vec2(0.5f, 0.5f) }; }
        static AnchorPreset MiddleRight() { return { Vec2(1.0f, 0.5f), Vec2(1.0f, 0.5f), Vec2(1.0f, 0.5f) }; }
        static AnchorPreset BottomLeft() { return { Vec2(0.0f, 0.0f), Vec2(0.0f, 0.0f), Vec2(0.0f, 0.0f) }; }
        static AnchorPreset BottomCenter() { return { Vec2(0.5f, 0.0f), Vec2(0.5f, 0.0f), Vec2(0.5f, 0.0f) }; }
        static AnchorPreset BottomRight() { return { Vec2(1.0f, 0.0f), Vec2(1.0f, 0.0f), Vec2(1.0f, 0.0f) }; }
        static AnchorPreset StretchHorizontal() { return { Vec2(0.0f, 0.5f), Vec2(1.0f, 0.5f), Vec2(0.5f, 0.5f) }; }
        static AnchorPreset StretchVertical() { return { Vec2(0.5f, 0.0f), Vec2(0.5f, 1.0f), Vec2(0.5f, 0.5f) }; }
        static AnchorPreset StretchAll() { return { Vec2(0.0f, 0.0f), Vec2(1.0f, 1.0f), Vec2(0.5f, 0.5f) }; }
    };
}