#pragma once

#undef min
#undef max

#include "../../Math/Math.h"
#include <cmath>
#include <algorithm>

namespace Uma_Engine
{
    /**
     * \brief Calculate distance from point to line segment
     * \param point Test point
     * \param lineStart Line start point
     * \param lineEnd Line end point
     * \return Shortest distance from point to line segment
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

    /**
     * \brief Snap position to grid
     * \param pos Position to snap
     * \param gridSize Grid cell size
     * \return Snapped position
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

    /**
     * \brief Calculate angle between two vectors
     * \param v1 First vector
     * \param v2 Second vector
     * \return Angle in radians
     */
    inline float AngleBetween(const Vec2& v1, const Vec2& v2)
    {
        return Uma_Math::angle(v1, v2);
    }

    /**
     * \brief Clamp value between min and max
     */
    inline float Clamp(float value, float a, float b)
    {
        return std::max(a, std::min(b, value));
    }

    /**
     * \brief Clamp Vec2 components
     */
    inline Vec2 Clamp(const Vec2& value, const Vec2& min, const Vec2& max)
    {
        return Vec2(
            Clamp(value.x, min.x, max.x),
            Clamp(value.y, min.y, max.y)
        );
    }

    /**
     * \brief Linear interpolation
     */
    inline float Lerp(float a, float b, float t)
    {
        return a + (b - a) * t;
    }

    /**
     * \brief Vector linear interpolation
     */
    inline Vec2 Lerp(const Vec2& a, const Vec2& b, float t)
    {
        return Uma_Math::lerp(a, b, t);
    }

    /**
     * \brief Check if point is inside rectangle
     */
    inline bool PointInRect(const Vec2& point, const Vec2& rectMin, const Vec2& rectMax)
    {
        return point.x >= rectMin.x && point.x <= rectMax.x &&
            point.y >= rectMin.y && point.y <= rectMax.y;
    }

    /**
     * \brief Calculate vector length
     */
    inline float Length(const Vec2& v)
    {
        return Uma_Math::magnitude(v);
    }

    /**
     * \brief Normalize vector
     */
    inline Vec2 Normalize(const Vec2& v)
    {
        return Uma_Math::normalized(v);
    }

    /**
     * \brief Calculate distance between two points
     */
    inline float Distance(const Vec2& a, const Vec2& b)
    {
        return Uma_Math::distance(a, b);
    }

    /**
     * \brief Calculate squared distance (faster, no sqrt)
     */
    inline float DistanceSquared(const Vec2& a, const Vec2& b)
    {
        return Uma_Math::distanceSquared(a, b);
    }

    /**
     * \brief Dot product of two vectors
     */
    inline float Dot(const Vec2& a, const Vec2& b)
    {
        return Uma_Math::dot(a, b);
    }

    /**
     * \brief Reflect vector across a normal
     */
    inline Vec2 Reflect(const Vec2& incident, const Vec2& normal)
    {
        return Uma_Math::reflect(incident, normal);
    }

    /**
     * \brief Calculate vector length squared (faster, no sqrt)
     */
    inline float LengthSquared(const Vec2& v)
    {
        return Uma_Math::magnitudeSquared(v);
    }

    /**
     * \brief Project vector a onto vector b
     */
    inline Vec2 Project(const Vec2& a, const Vec2& b)
    {
        float dotProduct = Uma_Math::dot(a, b);
        float bLengthSquared = Uma_Math::magnitudeSquared(b);

        if (bLengthSquared < 0.0001f)
            return Vec2(0.0f, 0.0f);

        return b * (dotProduct / bLengthSquared);
    }

    /**
     * \brief Perpendicular vector (rotated 90 degrees counter-clockwise)
     */
    inline Vec2 Perpendicular(const Vec2& v)
    {
        return Vec2(-v.y, v.x);
    }

    /**
     * \brief Rotate vector by angle (radians)
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

    /**
     * \brief Get angle of vector in radians
     */
    inline float GetAngle(const Vec2& v)
    {
        return std::atan2(v.y, v.x);
    }

    /**
     * \brief Create vector from angle (radians) with given length
     */
    inline Vec2 FromAngle(float angleRadians, float length = 1.0f)
    {
        return Vec2(
            std::cos(angleRadians) * length,
            std::sin(angleRadians) * length
        );
    }

    /**
     * \brief Component-wise min
     */
    inline Vec2 Min(const Vec2& a, const Vec2& b)
    {
        return Vec2(
            std::min(a.x, b.x),
            std::min(a.y, b.y)
        );
    }

    /**
     * \brief Component-wise max
     */
    inline Vec2 Max(const Vec2& a, const Vec2& b)
    {
        return Vec2(
            std::max(a.x, b.x),
            std::max(a.y, b.y)
        );
    }

    /**
     * \brief Component-wise absolute value
     */
    inline Vec2 Abs(const Vec2& v)
    {
        return Vec2(
            std::abs(v.x),
            std::abs(v.y)
        );
    }

    /**
     * \brief Sign of components (-1, 0, or 1)
     */
    inline Vec2 Sign(const Vec2& v)
    {
        return Vec2(
            v.x > 0.0f ? 1.0f : (v.x < 0.0f ? -1.0f : 0.0f),
            v.y > 0.0f ? 1.0f : (v.y < 0.0f ? -1.0f : 0.0f)
        );
    }

    /**
     * \brief Floor components
     */
    inline Vec2 Floor(const Vec2& v)
    {
        return Vec2(
            std::floor(v.x),
            std::floor(v.y)
        );
    }

    /**
     * \brief Ceil components
     */
    inline Vec2 Ceil(const Vec2& v)
    {
        return Vec2(
            std::ceil(v.x),
            std::ceil(v.y)
        );
    }

    /**
     * \brief Round components
     */
    inline Vec2 Round(const Vec2& v)
    {
        return Vec2(
            std::round(v.x),
            std::round(v.y)
        );
    }

}
