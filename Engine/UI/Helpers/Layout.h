/*!
\file   Layout.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Provides layout computation helper functions for UI positioning.

This header contains utility functions for computing canvas scales, converting
between coordinate spaces, and calculating final UI element positions based on
anchor and pivot settings.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "../Core/UITypes.h"
#include "../Components/Canvas.h"
#include "../Components/RectTransform.h"
#include "../../Math/Math.h"

namespace Uma_UI
{
    /*!
     * \brief Computes canvas scale factor based on screen resolution.
     * \param canvas Canvas component with scaling settings.
     * \param screenWidth Current screen width in pixels.
     * \param screenHeight Current screen height in pixels.
     * \return Scale factor to apply to UI elements.
     */
    inline float ComputeCanvasScale(const Canvas& canvas, float screenWidth, float screenHeight)
    {
        if (canvas.scaleMode == CanvasScaleMode::ConstantPixelSize)
        {
            return 1.0f;
        }

        float widthScale = screenWidth / canvas.referenceResolution.x;
        float heightScale = screenHeight / canvas.referenceResolution.y;
        float t = canvas.matchWidthOrHeight;
        return widthScale * (1.0f - t) + heightScale * t;
    }

    /*!
     * \brief Converts pixel coordinates to NDC space [-1, 1].
     * \param pixelPos Position in pixels (origin bottom-left).
     * \param screenWidth Screen width in pixels.
     * \param screenHeight Screen height in pixels.
     * \return Position in NDC space.
     */
    inline Vec2 PixelToNDC(const Vec2& pixelPos, float screenWidth, float screenHeight)
    {
        float ndcX = (pixelPos.x / screenWidth) * 2.0f - 1.0f;
        float ndcY = (pixelPos.y / screenHeight) * 2.0f - 1.0f;
        return Vec2(ndcX, ndcY);
    }

    /*!
     * \brief Converts NDC coordinates to pixel space.
     * \param ndcPos Position in NDC [-1, 1].
     * \param screenWidth Screen width in pixels.
     * \param screenHeight Screen height in pixels.
     * \return Position in pixel space (origin bottom-left).
     */
    inline Vec2 NDCToPixel(const Vec2& ndcPos, float screenWidth, float screenHeight)
    {
        float pixelX = (ndcPos.x + 1.0f) * 0.5f * screenWidth;
        float pixelY = (ndcPos.y + 1.0f) * 0.5f * screenHeight;
        return Vec2(pixelX, pixelY);
    }

    /*!
     * \brief Converts pixel size to NDC size.
     * \param pixelSize Size in pixels.
     * \param screenWidth Screen width in pixels.
     * \param screenHeight Screen height in pixels.
     * \return Size in NDC space.
     */
    inline Vec2 PixelSizeToNDC(const Vec2& pixelSize, float screenWidth, float screenHeight)
    {
        float ndcW = (pixelSize.x / screenWidth) * 2.0f;
        float ndcH = (pixelSize.y / screenHeight) * 2.0f;
        return Vec2(ndcW, ndcH);
    }

    /*!
     * \brief Gets the screen-space rectangle in NDC covering the entire screen.
     * \return NDC rect centered at origin with size (2.0, 2.0).
     */
    inline Rect GetScreenRect()
    {
        return Rect(0.0f, 0.0f, 2.0f, 2.0f);
    }

    /*!
     * \brief Computes final NDC rectangle from RectTransform and parent data.
     * \param rectTransform RectTransform to compute.
     * \param parentRect Parent's computed NDC rect.
     * \param canvasScale Scale factor from canvas.
     * \param screenWidth Screen width in pixels.
     * \param screenHeight Screen height in pixels.
     * \return Computed NDC rectangle.
     */
    inline Rect ComputeRectInNDC(
        const RectTransform& rectTransform,
        const Rect& parentRect,
        float canvasScale,
        float screenWidth,
        float screenHeight)
    {
        float parentLeft = parentRect.Left();
        //float parentRight = parentRect.Right();
        float parentBottom = parentRect.Bottom();
        //float parentTop = parentRect.Top();
        float parentWidth = parentRect.width;
        float parentHeight = parentRect.height;

        Vec2 anchorMinNDC(
            parentLeft + rectTransform.anchorMin.x * parentWidth,
            parentBottom + rectTransform.anchorMin.y * parentHeight
        );

        Vec2 anchorMaxNDC(
            parentLeft + rectTransform.anchorMax.x * parentWidth,
            parentBottom + rectTransform.anchorMax.y * parentHeight
        );

        Vec2 sizePixels = rectTransform.sizeDelta * canvasScale;
        Vec2 sizeNDC = PixelSizeToNDC(sizePixels, screenWidth, screenHeight);
        Vec2 anchoredPosPixels = rectTransform.anchoredPosition * canvasScale;
        Vec2 anchoredPosNDC = PixelSizeToNDC(anchoredPosPixels, screenWidth, screenHeight);

        float finalWidthNDC;
        float finalHeightNDC;
        float finalCenterX;
        float finalCenterY;

        if (rectTransform.IsStretchingHorizontal())
        {
            float anchorSpanNDC = anchorMaxNDC.x - anchorMinNDC.x;
            finalWidthNDC = anchorSpanNDC + sizeNDC.x;
            finalCenterX = (anchorMinNDC.x + anchorMaxNDC.x) * 0.5f + anchoredPosNDC.x;
        }
        else
        {
            finalWidthNDC = sizeNDC.x;
            float anchorX = anchorMinNDC.x;
            finalCenterX = anchorX + anchoredPosNDC.x + finalWidthNDC * (0.5f - rectTransform.pivot.x);
        }

        if (rectTransform.IsStretchingVertical())
        {
            float anchorSpanNDC = anchorMaxNDC.y - anchorMinNDC.y;
            finalHeightNDC = anchorSpanNDC + sizeNDC.y;
            finalCenterY = (anchorMinNDC.y + anchorMaxNDC.y) * 0.5f + anchoredPosNDC.y;
        }
        else
        {
            finalHeightNDC = sizeNDC.y;
            float anchorY = anchorMinNDC.y;
            finalCenterY = anchorY + anchoredPosNDC.y + finalHeightNDC * (0.5f - rectTransform.pivot.y);
        }

        return Rect(finalCenterX, finalCenterY, finalWidthNDC, finalHeightNDC);
    }

    /*!
     * \brief Converts NDC position to screen pixels with Y-axis flip.
     * \param ndcX X position in NDC.
     * \param ndcY Y position in NDC.
     * \param screenWidth Screen width in pixels.
     * \param screenHeight Screen height in pixels.
     * \return Position in pixel space.
     */
    inline Vec2 NDCToScreen(float ndcX, float ndcY, float screenWidth, float screenHeight)
    {
        return NDCToPixel(Vec2(ndcX, -ndcY), screenWidth, screenHeight);
    }
}