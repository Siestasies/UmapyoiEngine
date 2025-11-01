#pragma once

#include "../Core/UITypes.h"
#include "../../Math/Math.h"

namespace Uma_UI
{
    /**
     * \brief Convert screen pixel coordinates to NDC space [-1, 1]
     * \param screenX Mouse X in pixels (0 = left)
     * \param screenY Mouse Y in pixels (0 = top, inverted from OpenGL)
     * \param screenWidth Window width in pixels
     * \param screenHeight Window height in pixels
     * \return Position in NDC space
     */
    inline Vec2 ScreenToNDC(float screenX, float screenY, float screenWidth, float screenHeight)
    {
        // Convert from top-left origin to bottom-left
        float flippedY = screenHeight - screenY;

        // Normalize to [0,1]
        float normX = screenX / screenWidth;
        float normY = flippedY / screenHeight;

        // Convert to NDC [-1,1]
        float ndcX = normX * 2.0f - 1.0f;
        float ndcY = normY * 2.0f - 1.0f;

        return Vec2(ndcX, ndcY);
    }

    /**
     * \brief Test if NDC point is inside a rect
     * \param point Point in NDC space
     * \param rect Rectangle to test
     * \return true if point is inside rect
     */
    inline bool IsPointInRect(const Vec2& point, const Rect& rect)
    {
        return rect.Contains(point);
    }

    /**
     * \brief Find topmost UI element at pointer position
     * \param point Pointer position in NDC
     * \param rects List of UI rects with entity IDs
     * \return Entity ID of topmost hit, or -1 if none
     *
     * Note: rects should be pre-sorted by render order (back to front)
     */
    inline Uma_ECS::Entity RaycastUI(const Vec2& point, const std::vector<std::pair<Uma_ECS::Entity, Rect>>& rects)
    {
        // Iterate backwards (front to back in render order)
        for (auto it = rects.rbegin(); it != rects.rend(); ++it)
        {
            if (IsPointInRect(point, it->second))
            {
                return it->first;
            }
        }

        return static_cast<Uma_ECS::Entity>(-1);  // No hit
    }
}