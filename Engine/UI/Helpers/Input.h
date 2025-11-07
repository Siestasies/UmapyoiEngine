/*!
\file   Input.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Provides input helper functions for UI interaction and hit testing.

This header contains utility functions for converting screen coordinates,
testing point containment, and raycasting against UI elements.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "../Core/UITypes.h"
#include "../../Math/Math.h"

namespace Uma_UI
{
    /*!
     * \brief Converts screen pixel coordinates to NDC space [-1, 1].
     * \param screenX Mouse X in pixels (0 = left).
     * \param screenY Mouse Y in pixels (0 = top).
     * \param screenWidth Window width in pixels.
     * \param screenHeight Window height in pixels.
     * \return Position in NDC space.
     */
    inline Vec2 ScreenToNDC(float screenX, float screenY, float screenWidth, float screenHeight)
    {
        float flippedY = screenHeight - screenY;
        float normX = screenX / screenWidth;
        float normY = flippedY / screenHeight;
        float ndcX = normX * 2.0f - 1.0f;
        float ndcY = normY * 2.0f - 1.0f;
        return Vec2(ndcX, ndcY);
    }

    /*!
     * \brief Tests if an NDC point is inside a rectangle.
     * \param point Point in NDC space.
     * \param rect Rectangle to test.
     * \return True if point is inside rect.
     */
    inline bool IsPointInRect(const Vec2& point, const Rect& rect)
    {
        return rect.Contains(point);
    }

    /*!
     * \brief Finds the topmost UI element at a pointer position.
     * \param point Pointer position in NDC.
     * \param rects List of UI rects with entity IDs, sorted back to front.
     * \return Entity ID of topmost hit, or -1 if none.
     */
    inline Uma_ECS::Entity RaycastUI(const Vec2& point, const std::vector<std::pair<Uma_ECS::Entity, Rect>>& rects)
    {
        for (auto it = rects.rbegin(); it != rects.rend(); ++it)
        {
            if (IsPointInRect(point, it->second))
            {
                return it->first;
            }
        }
        return static_cast<Uma_ECS::Entity>(-1);
    }
}