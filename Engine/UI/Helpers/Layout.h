#pragma once

#include "../Core/UITypes.h"
#include "../Components/Canvas.h"
#include "../Components/RectTransform.h"
#include "../../Math/Math.h"

namespace Uma_UI
{
    /**
     * \brief Compute canvas scale factor based on screen resolution
     * \param canvas Canvas component
     * \param screenWidth Current screen width in pixels
     * \param screenHeight Current screen height in pixels
     * \return Scale factor to apply to UI elements
     */
    inline float ComputeCanvasScale(const Canvas& canvas, float screenWidth, float screenHeight)
    {
        if (canvas.scaleMode == CanvasScaleMode::ConstantPixelSize)
        {
            return 1.0f;
        }

        // ScaleWithScreenSize
        float widthScale = screenWidth / canvas.referenceResolution.x;
        float heightScale = screenHeight / canvas.referenceResolution.y;

        // Lerp between width and height based on matchWidthOrHeight
        float t = canvas.matchWidthOrHeight;
        return widthScale * (1.0f - t) + heightScale * t;
    }

    /**
     * \brief Convert pixel coordinates to NDC [-1, 1]
     * \param pixelPos Position in pixels (origin bottom-left)
     * \param screenWidth Screen width in pixels
     * \param screenHeight Screen height in pixels
     * \return Position in NDC space
     */
    inline Vec2 PixelToNDC(const Vec2& pixelPos, float screenWidth, float screenHeight)
    {
        float ndcX = (pixelPos.x / screenWidth) * 2.0f - 1.0f;
        float ndcY = (pixelPos.y / screenHeight) * 2.0f - 1.0f;
        return Vec2(ndcX, ndcY);
    }

    /**
     * \brief Convert NDC coordinates to pixels
     * \param ndcPos Position in NDC [-1, 1]
     * \param screenWidth Screen width in pixels
     * \param screenHeight Screen height in pixels
     * \return Position in pixel space (origin bottom-left)
     */
    inline Vec2 NDCToPixel(const Vec2& ndcPos, float screenWidth, float screenHeight)
    {
        float pixelX = (ndcPos.x + 1.0f) * 0.5f * screenWidth;
        float pixelY = (ndcPos.y + 1.0f) * 0.5f * screenHeight;
        return Vec2(pixelX, pixelY);
    }

    /**
     * \brief Convert pixel size to NDC size
     * \param pixelSize Size in pixels
     * \param screenWidth Screen width in pixels
     * \param screenHeight Screen height in pixels
     * \return Size in NDC space
     */
    inline Vec2 PixelSizeToNDC(const Vec2& pixelSize, float screenWidth, float screenHeight)
    {
        float ndcW = (pixelSize.x / screenWidth) * 2.0f;
        float ndcH = (pixelSize.y / screenHeight) * 2.0f;
        return Vec2(ndcW, ndcH);
    }

    /**
     * \brief Compute final NDC rect from RectTransform + parent rect + canvas scale
     * \param rectTransform RectTransform to compute
     * \param parentRect Parent's computed NDC rect (or screen rect if root)
     * \param canvasScale Scale factor from canvas
     * \param screenWidth Screen width in pixels
     * \param screenHeight Screen height in pixels
     * \return Computed NDC rectangle
     */
    inline Rect ComputeRectInNDC(
        const RectTransform& rectTransform,
        const Rect& parentRect,
        float canvasScale,
        float screenWidth,
        float screenHeight)
    {
        // 1. Compute anchor positions in parent space (NDC)
        float parentLeft = parentRect.Left();
        float parentRight = parentRect.Right();
        float parentBottom = parentRect.Bottom();
        float parentTop = parentRect.Top();

        float parentWidth = parentRect.width;
        float parentHeight = parentRect.height;

        // Anchor corners in parent's NDC space
        Vec2 anchorMinNDC(
            parentLeft + rectTransform.anchorMin.x * parentWidth,
            parentBottom + rectTransform.anchorMin.y * parentHeight
        );

        Vec2 anchorMaxNDC(
            parentLeft + rectTransform.anchorMax.x * parentWidth,
            parentBottom + rectTransform.anchorMax.y * parentHeight
        );

        // 2. Compute size in pixels (scaled by canvas)
        Vec2 sizePixels = rectTransform.sizeDelta * canvasScale;

        // If stretching, size is determined by anchor distance + offset
        float finalWidthNDC;
        float finalHeightNDC;

        if (rectTransform.IsStretchingHorizontal())
        {
            // Width = distance between anchors + left/right offsets
            finalWidthNDC = (anchorMaxNDC.x - anchorMinNDC.x) +
                PixelSizeToNDC(Vec2(sizePixels.x, 0.0f), screenWidth, screenHeight).x;
        }
        else
        {
            // Fixed width
            finalWidthNDC = PixelSizeToNDC(Vec2(sizePixels.x, 0.0f), screenWidth, screenHeight).x;
        }

        if (rectTransform.IsStretchingVertical())
        {
            // Height = distance between anchors + bottom/top offsets
            finalHeightNDC = (anchorMaxNDC.y - anchorMinNDC.y) +
                PixelSizeToNDC(Vec2(0.0f, sizePixels.y), screenWidth, screenHeight).y;
        }
        else
        {
            // Fixed height
            finalHeightNDC = PixelSizeToNDC(Vec2(0.0f, sizePixels.y), screenWidth, screenHeight).y;
        }

        // 3. Compute center based on anchored position and pivot
        Vec2 anchorCenterNDC(
            (anchorMinNDC.x + anchorMaxNDC.x) * 0.5f,
            (anchorMinNDC.y + anchorMaxNDC.y) * 0.5f
        );

        Vec2 offsetNDC = PixelToNDC(
            rectTransform.anchoredPosition * canvasScale,
            screenWidth,
            screenHeight
        ) - Vec2(0.0f, 0.0f); // Offset from origin

        // Adjust for pivot
        Vec2 pivotOffsetNDC(
            (0.5f - rectTransform.pivot.x) * finalWidthNDC,
            (0.5f - rectTransform.pivot.y) * finalHeightNDC
        );

        Vec2 centerNDC = anchorCenterNDC + offsetNDC + pivotOffsetNDC;

        // 4. Return final rect
        Rect result;
        result.x = centerNDC.x;
        result.y = centerNDC.y;
        result.width = finalWidthNDC;
        result.height = finalHeightNDC;

        return result;
    }

    /**
     * \brief Get screen-space rect (full screen in NDC)
     * \return NDC rect covering entire screen
     */
    inline Rect GetScreenRect()
    {
        return Rect(0.0f, 0.0f, 2.0f, 2.0f);  // [-1,1] in both axes
    }
}