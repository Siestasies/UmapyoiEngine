/*!
\file   EditorMath.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Provides common mathematical utility functions for the editor system.

This header contains inline helper functions for geometric calculations, vector operations,
and coordinate snapping used by editor subsystems for picking, gizmo rendering, and transformation.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#undef min
#undef max

#include "Math/Math.h"
#include <cmath>
#include <algorithm>

namespace Uma_Engine
{
    /*!
     * \brief Calculates the shortest distance from a point to a line segment.
     * \param point Test point.
     * \param lineStart Line segment start point.
     * \param lineEnd Line segment end point.
     * \return Distance from point to line segment.
     */
    inline float DistanceToLineSegment(const Vec2& point, const Vec2& lineStart, const Vec2& lineEnd)
    {
        Vec2 line = lineEnd - lineStart;
        float lineLength = Uma_Math::magnitude(line);

        if (lineLength < 0.001f)
        {
            Vec2 toPoint = point - lineStart;
            return Uma_Math::magnitude(toPoint);
        }

        Vec2 lineNorm = Uma_Math::normalized(line);
        Vec2 toPoint = point - lineStart;

        float projection = Uma_Math::dot(toPoint, lineNorm);
        projection = std::max(0.0f, std::min(lineLength, projection));

        Vec2 closestPoint = lineStart + lineNorm * projection;
        Vec2 diff = point - closestPoint;

        return Uma_Math::magnitude(diff);
    }

    /*!
     * \brief Snaps a position to a grid with specified cell size.
     * \param pos Position to snap.
     * \param gridSize Grid cell size.
     * \return Snapped position.
     */
    inline Vec2 SnapToGrid(const Vec2& pos, float gridSize)
    {
        if (gridSize <= 0.0f)
            return pos;

        return Vec2(
            std::round(pos.x / gridSize) * gridSize,
            std::round(pos.y / gridSize) * gridSize
        );
    }

    /*!
     * \brief Calculates the angle between two vectors.
     * \param v1 First vector.
     * \param v2 Second vector.
     * \return Angle in radians.
     */
    inline float AngleBetween(const Vec2& v1, const Vec2& v2)
    {
        return Uma_Math::angle(v1, v2);
    }

    /*!
     * \brief Clamps a float value between min and max.
     * \param value Value to clamp.
     * \param a Minimum value.
     * \param b Maximum value.
     * \return Clamped value.
     */
    inline float Clamp(float value, float a, float b)
    {
        return std::max(a, std::min(b, value));
    }

    /*!
     * \brief Clamps each component of a Vec2 between corresponding min and max components.
     * \param value Vector to clamp.
     * \param min Minimum vector.
     * \param max Maximum vector.
     * \return Clamped vector.
     */
    inline Vec2 Clamp(const Vec2& value, const Vec2& min, const Vec2& max)
    {
        return Vec2(
            Clamp(value.x, min.x, max.x),
            Clamp(value.y, min.y, max.y)
        );
    }

    /*!
     * \brief Linearly interpolates between two float values.
     * \param a First value.
     * \param b Second value.
     * \param t Interpolation factor.
     * \return Interpolated value.
     */
    inline float Lerp(float a, float b, float t)
    {
        return a + (b - a) * t;
    }

    /*!
     * \brief Linearly interpolates between two Vec2 vectors.
     * \param a First vector.
     * \param b Second vector.
     * \param t Interpolation factor.
     * \return Interpolated vector.
     */
    inline Vec2 Lerp(const Vec2& a, const Vec2& b, float t)
    {
        return Uma_Math::lerp(a, b, t);
    }

    /*!
     * \brief Tests if a point is inside a rectangle defined by min and max bounds.
     * \param point Point to test.
     * \param rectMin Rectangle minimum corner.
     * \param rectMax Rectangle maximum corner.
     * \return True if point is inside rectangle.
     */
    inline bool PointInRect(const Vec2& point, const Vec2& rectMin, const Vec2& rectMax)
    {
        return point.x >= rectMin.x && point.x <= rectMax.x &&
            point.y >= rectMin.y && point.y <= rectMax.y;
    }

    /*!
     * \brief Calculates the length (magnitude) of a vector.
     * \param v Vector.
     * \return Vector length.
     */
    inline float Length(const Vec2& v)
    {
        return Uma_Math::magnitude(v);
    }

    /*!
     * \brief Normalizes a vector to unit length.
     * \param v Vector to normalize.
     * \return Normalized vector.
     */
    inline Vec2 Normalize(const Vec2& v)
    {
        return Uma_Math::normalized(v);
    }

    /*!
     * \brief Calculates the distance between two points.
     * \param a First point.
     * \param b Second point.
     * \return Distance between points.
     */
    inline float Distance(const Vec2& a, const Vec2& b)
    {
        return Uma_Math::distance(a, b);
    }

    /*!
     * \brief Calculates the squared distance between two points (faster, no sqrt).
     * \param a First point.
     * \param b Second point.
     * \return Squared distance between points.
     */
    inline float DistanceSquared(const Vec2& a, const Vec2& b)
    {
        return Uma_Math::distanceSquared(a, b);
    }

    /*!
     * \brief Calculates the dot product of two vectors.
     * \param a First vector.
     * \param b Second vector.
     * \return Dot product.
     */
    inline float Dot(const Vec2& a, const Vec2& b)
    {
        return Uma_Math::dot(a, b);
    }

    /*!
     * \brief Reflects a vector across a normal.
     * \param incident Incident vector.
     * \param normal Normal vector.
     * \return Reflected vector.
     */
    inline Vec2 Reflect(const Vec2& incident, const Vec2& normal)
    {
        return Uma_Math::reflect(incident, normal);
    }

    /*!
     * \brief Calculates the squared length of a vector (faster, no sqrt).
     * \param v Vector.
     * \return Squared length.
     */
    inline float LengthSquared(const Vec2& v)
    {
        return Uma_Math::magnitudeSquared(v);
    }

    /*!
     * \brief Projects vector a onto vector b.
     * \param a Vector to project.
     * \param b Vector to project onto.
     * \return Projected vector.
     */
    inline Vec2 Project(const Vec2& a, const Vec2& b)
    {
        float dotProduct = Uma_Math::dot(a, b);
        float bLengthSquared = Uma_Math::magnitudeSquared(b);

        if (bLengthSquared < 0.0001f)
            return Vec2(0.0f, 0.0f);

        return b * (dotProduct / bLengthSquared);
    }

    /*!
     * \brief Returns a perpendicular vector (rotated 90 degrees counter-clockwise).
     * \param v Input vector.
     * \return Perpendicular vector.
     */
    inline Vec2 Perpendicular(const Vec2& v)
    {
        return Vec2(-v.y, v.x);
    }

    /*!
     * \brief Rotates a vector by an angle in radians.
     * \param v Vector to rotate.
     * \param angleRadians Rotation angle in radians.
     * \return Rotated vector.
     */
    inline Vec2 Rotate(const Vec2& v, float angleRadians)
    {
        float cosAngle = std::cos(angleRadians);
        float sinAngle = std::sin(angleRadians);

        return Vec2(
            v.x * cosAngle - v.y * sinAngle,
            v.x * sinAngle + v.y * cosAngle
        );
    }

    /*!
     * \brief Gets the angle of a vector in radians.
     * \param v Vector.
     * \return Angle in radians.
     */
    inline float GetAngle(const Vec2& v)
    {
        return std::atan2(v.y, v.x);
    }

    /*!
     * \brief Creates a vector from an angle with specified length.
     * \param angleRadians Angle in radians.
     * \param length Vector length.
     * \return Constructed vector.
     */
    inline Vec2 FromAngle(float angleRadians, float length = 1.0f)
    {
        return Vec2(
            std::cos(angleRadians) * length,
            std::sin(angleRadians) * length
        );
    }

    /*!
     * \brief Component-wise minimum of two vectors.
     * \param a First vector.
     * \param b Second vector.
     * \return Component-wise minimum vector.
     */
    inline Vec2 Min(const Vec2& a, const Vec2& b)
    {
        return Vec2(
            std::min(a.x, b.x),
            std::min(a.y, b.y)
        );
    }

    /*!
     * \brief Component-wise maximum of two vectors.
     * \param a First vector.
     * \param b Second vector.
     * \return Component-wise maximum vector.
     */
    inline Vec2 Max(const Vec2& a, const Vec2& b)
    {
        return Vec2(
            std::max(a.x, b.x),
            std::max(a.y, b.y)
        );
    }

    /*!
     * \brief Component-wise absolute value of a vector.
     * \param v Input vector.
     * \return Component-wise absolute vector.
     */
    inline Vec2 Abs(const Vec2& v)
    {
        return Vec2(
            std::abs(v.x),
            std::abs(v.y)
        );
    }

    /*!
     * \brief Component-wise sign of a vector (-1, 0, or 1).
     * \param v Input vector.
     * \return Component-wise sign vector.
     */
    inline Vec2 Sign(const Vec2& v)
    {
        return Vec2(
            v.x > 0.0f ? 1.0f : (v.x < 0.0f ? -1.0f : 0.0f),
            v.y > 0.0f ? 1.0f : (v.y < 0.0f ? -1.0f : 0.0f)
        );
    }

    /*!
     * \brief Component-wise floor of a vector.
     * \param v Input vector.
     * \return Component-wise floored vector.
     */
    inline Vec2 Floor(const Vec2& v)
    {
        return Vec2(
            std::floor(v.x),
            std::floor(v.y)
        );
    }

    /*!
     * \brief Component-wise ceiling of a vector.
     * \param v Input vector.
     * \return Component-wise ceiled vector.
     */
    inline Vec2 Ceil(const Vec2& v)
    {
        return Vec2(
            std::ceil(v.x),
            std::ceil(v.y)
        );
    }

    /*!
     * \brief Component-wise round of a vector.
     * \param v Input vector.
     * \return Component-wise rounded vector.
     */
    inline Vec2 Round(const Vec2& v)
    {
        return Vec2(
            std::round(v.x),
            std::round(v.y)
        );
    }
}